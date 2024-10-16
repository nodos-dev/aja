// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#pragma once
#include <Nodos/PluginHelpers.hpp>

namespace nos::aja
{
    
struct DMANodeBase : NodeContext
{
	DMANodeBase(const nosFbNode* node, DMADirection dir) : NodeContext(node), Direction(dir)
	{
	}

	uint8_t DoubleBufferIdx = 0;
	NTV2Channel Channel = NTV2_CHANNEL_INVALID;
	std::shared_ptr<AJADevice> Device = nullptr;
	NTV2VideoFormat Format = NTV2_FORMAT_UNKNOWN;
	std::string ChannelName;
	AJADevice::Mode Mode = AJADevice::SL;
	DMADirection Direction;
	nos::mediaio::YCbCrPixelFormat PixelFormat = nos::mediaio::YCbCrPixelFormat::YUV8;

	bool IsInterlaced() const
	{
		return !IsProgressivePicture(Format);
	}

	bool IsQuad() const
	{
		return AJADevice::IsQuad(Mode);
	}

	bool IsInput() const {
		return Direction == DMA_READ;
	}

	bool NeedsFrameSet = false;
	ULWord NextVBL = 0;

	virtual void OnPathStart() { NeedsFrameSet = true; DoubleBufferIdx = 0; NextVBL = 0; }

	void SetFrame(u32 doubleBufferIndex)
	{
		u32 frameIndex = GetFrameBufferOffset(Channel, doubleBufferIndex) / Device->GetFBSize(Channel);
		IsInput() ? Device->SetInputFrame(Channel, frameIndex)
			: Device->SetOutputFrame(Channel, frameIndex);
		if (IsQuad())
			for (u32 i = Channel + 1; i < Channel + 4u; ++i)
				IsInput() ? Device->SetInputFrame(NTV2Channel(i), frameIndex)
				: Device->SetOutputFrame(NTV2Channel(i), frameIndex);
	}

	uint32_t StartDoubleBuffer()
	{
		SetFrame(uint32_t(!IsInterlaced()));
		return 0;
	}

	uint32_t NextDoubleBuffer(uint32_t curDoubleBuffer)
	{
		if (IsInterlaced())
			return curDoubleBuffer;
		SetFrame(curDoubleBuffer);
		return curDoubleBuffer ^ 1;
	}

	size_t GetMaxFrameBufferSize()
	{
		size_t max = 0;

		for (int i = 0; i < NTV2_MAX_NUM_CHANNELS; ++i)
			max = std::max(max, (size_t)Device->GetFBSize(NTV2Channel(i)));

		return max;
	}

	std::unordered_map<NTV2Channel, std::unordered_map<uint8_t, size_t>> FrameBufferOffsets;

	void OnPathStop() override
	{
		FrameBufferOffsets.clear();
	}

	u32 GetFrameBufferOffset(NTV2Channel channel, uint8_t frame)
	{
		auto it = FrameBufferOffsets.find(channel);
		if (it == FrameBufferOffsets.end())
			it = FrameBufferOffsets.insert({ channel, {} }).first;
		auto offsetIt = it->second.find(frame);
		if (offsetIt == it->second.end())
			offsetIt = it->second.insert({ frame, GetMaxFrameBufferSize() * 2 * channel + (uint32_t(frame) & 1) * Device->GetFBSize(channel) }).first;
		assert(offsetIt->second <= UINT32_MAX);
		return u32(offsetIt->second);
	}

	struct DMAInfo {
		nosVec2u CompressedExtent;
		size_t BufferSize;
	};

	DMAInfo GetDMAInfo()
	{
		u32 width, height;
		Device->GetExtent(Format, Mode, width, height);
		int BitWidth = PixelFormat == mediaio::YCbCrPixelFormat::YUV8 ? 8 : 10;
		nosVec2u compressedExt((10 == BitWidth) ? ((width + (48 - width % 48) % 48) / 3) << 1 : width >> 1, height >> u32(IsInterlaced()));
		uint32_t bufferSize = compressedExt.x * compressedExt.y * 4;
		return {compressedExt, bufferSize};
	}

	void DMATransfer(nos::sys::vulkan::FieldType fieldType, uint32_t curVBLCount, uint8_t* buffer, uint64_t inputBufferSize)
	{
		auto [compressedExt, bufferSize] = GetDMAInfo();
		assert(bufferSize <= UINT32_MAX);

		if (bufferSize != inputBufferSize)
			return nosEngine.LogE("DMATransfer buffer size mismatch");

		if (NeedsFrameSet)
		{
			DoubleBufferIdx = StartDoubleBuffer();
			NeedsFrameSet = false;
		}

		if (curVBLCount < NextVBL)
			return;
		
		auto offset =  GetFrameBufferOffset(Channel, DoubleBufferIdx);
		{
			ScopedProfilerEvent _("AJA " + ChannelName + (IsInput() ? " DMA Read" : " DMA Write"));
			if (IsInterlaced())
			{
				auto pitch = compressedExt.x * 4;
				auto segments = compressedExt.y;
				auto fieldId = fieldType == nos::sys::vulkan::FieldType::EVEN ? NTV2_FIELD0 : NTV2_FIELD1;
				util::Stopwatch sw;
				Device->DmaTransfer(NTV2_DMA_FIRST_AVAILABLE, IsInput(), 0,
					const_cast<ULWord*>((u32*)buffer), // target CPU buffer address
					offset + fieldId * pitch, // source AJA buffer address
					pitch, // length of one line
					segments, // number of lines
					pitch, // increment target buffer one line on CPU memory
					pitch * 2, // increment AJA card source buffer double the size of one line
					true);
				auto elapsed = sw.Elapsed();
				nosEngine.WatchLog(("AJA " + ChannelName + (IsInput() ? " DMA Read" : " DMA Write")).c_str(),
					nos::util::Stopwatch::ElapsedString(elapsed).c_str());
			}
			else
			{
				util::Stopwatch sw;
				Device->DmaTransfer(NTV2_DMA_FIRST_AVAILABLE, IsInput(), 0, const_cast<ULWord*>((u32*)buffer),
					offset, u32(bufferSize), true);
				auto elapsed = sw.Elapsed();
				nosEngine.WatchLog(("AJA " + ChannelName + (IsInput() ? " DMA Read" : " DMA Write")).c_str(),
					nos::util::Stopwatch::ElapsedString(elapsed).c_str());
			}
		}

		DoubleBufferIdx = NextDoubleBuffer(DoubleBufferIdx);

		ULWord newVBLCount = 0;
		if (Direction == DMA_READ)
			Device->GetInputVerticalInterruptCount(newVBLCount, Channel);
		else
			Device->GetOutputVerticalInterruptCount(newVBLCount, Channel);

		// DMA likely skipped a frame
		if (curVBLCount != newVBLCount)
			nosEngine.CallNodeFunction(NodeId, NOS_NAME("Drop"));

		NextVBL = newVBLCount + 1;
	}
};

}