#include <Nodos/PluginHelpers.hpp>

// External
#include <nosVulkanSubsystem/nosVulkanSubsystem.h>
#include <nosVulkanSubsystem/Helpers.hpp>
#include <nosUtil/Stopwatch.hpp>

#include "AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"

namespace nos::aja
{

struct DMAInfo
{
	u32* Buffer;
	u32 Pitch;
	u32 Segments;
	u32 FrameIndex;
};

struct DMAReadNodeContext : NodeContext
{
	DMAReadNodeContext(const nosFbNode* node) : NodeContext(node)
	{
	}

	uint8_t DoubleBufferIdx = 0;
	NTV2Channel Channel = NTV2_CHANNEL_INVALID;
	std::shared_ptr<AJADevice> Device = nullptr;
	NTV2VideoFormat Format = NTV2_FORMAT_UNKNOWN;

	bool Interlaced() const
	{
		return !IsProgressivePicture(Format); // TODO: Implement
	}

	uint32_t StartDoubleBuffer()
	{
		SetFrame(uint8_t(!Interlaced()));
		return 0;
	}

	bool NeedsFrameSet = false;

	virtual void OnPathStart()
	{
		NeedsFrameSet = true;
	}

	uint32_t GetFrameBufferIndex(uint8_t doubleBufferIndex) const
	{
		return 2 * Channel + doubleBufferIndex;
	}

	void SetFrame(uint8_t curDoubleBufferIndex)
	{
		u32 idx = GetFrameBufferIndex(curDoubleBufferIndex);
		Device->SetInputFrame(Channel, idx);
		if (NTV2_IS_QUAD_FRAME_FORMAT(Format)) // TODO: Get from channel info
			for (u32 i = Channel + 1; i < Channel + 4; ++i)
				Device->SetInputFrame(NTV2Channel(i), idx);
	}

	uint8_t NextDoubleBuffer(uint8_t curDoubleBuffer)
	{
		if (Interlaced())
			return curDoubleBuffer;
		SetFrame(curDoubleBuffer);
		return curDoubleBuffer ^ 1;
	}
 
	void GetScheduleInfo(nosScheduleInfo* out) override
	{
		*out = nosScheduleInfo {
			.Importance = 1,
			.DeltaSeconds = GetDeltaSeconds(Format, Interlaced()),
			.Type = NOS_SCHEDULE_TYPE_ON_DEMAND,
		};
	}

	size_t GetMaxFrameBufferSize()
	{
		size_t max = 0;

		for (int i = 0; i < NTV2_MAX_NUM_CHANNELS; ++i)
			max = std::max(max, (size_t)Device->GetFBSize(NTV2Channel(i)));

		return max;
	}

	size_t GetFrameBufferOffset(NTV2Channel channel, u32 frame)
	{
		return GetMaxFrameBufferSize() * 2 * channel + (frame & 1) * Device->GetFBSize(channel);
	}

	nosResult ExecuteNode(const nosNodeExecuteArgs* args) override
	{
		ChannelInfo* channelInfo = nullptr;
		nosResourceShareInfo outputBuffer{};
		const nosBuffer* outputPinData = nullptr;
		auto fieldType = nos::sys::vulkan::FieldType::UNKNOWN;
		for (size_t i = 0; i < args->PinCount; ++i)
		{
			auto& pin = args->Pins[i];
			if (pin.Name == NOS_NAME_STATIC("Channel"))
				channelInfo = InterpretPinValue<ChannelInfo>(*pin.Data);
			if (pin.Name == NOS_NAME_STATIC("Output"))
			{
				outputPinData = pin.Data;
				outputBuffer = vkss::ConvertToResourceInfo(*InterpretPinValue<sys::vulkan::Buffer>(*outputPinData));
			}
			if (pin.Name == NOS_NAME("FieldType"))
				fieldType = *InterpretPinValue<sys::vulkan::FieldType>(*pin.Data);
		}
		
		if (!channelInfo->device())
			return NOS_RESULT_FAILED;

		Device = AJADevice::GetDeviceBySerialNumber(channelInfo->device()->serial_number());
		if (!Device)
			return NOS_RESULT_FAILED;
		auto channelStr = channelInfo->channel_name();
		if (!channelStr)
			return NOS_RESULT_FAILED;
		Channel = ParseChannel(channelStr->string_view());
		Format = NTV2VideoFormat(channelInfo->video_format_idx());

		if (NeedsFrameSet)
		{
			StartDoubleBuffer();
			NeedsFrameSet = false;
		}

		u32 width, height;
		Device->GetExtent(Format, AJADevice::SL, width, height);
		
		auto PixelFormat = nos::MediaIO::YCbCrPixelFormat::YUV8;
		int BitWidth = PixelFormat == nos::MediaIO::YCbCrPixelFormat::YUV8 ? 8 : 10;
		nosVec2u compressedExt((10 == BitWidth) ? ((width + (48 - width % 48) % 48) / 3) << 1 : width >> 1, height >> u32(Interlaced()));
		uint32_t bufferSize = compressedExt.x * compressedExt.y * 4;

		constexpr nosMemoryFlags memoryFlags = nosMemoryFlags(NOS_MEMORY_FLAGS_HOST_VISIBLE);
		if (outputBuffer.Memory.Size != bufferSize || outputBuffer.Info.Buffer.MemoryFlags != memoryFlags)
		{
			nosResourceShareInfo bufInfo = {
				.Info = {
					.Type = NOS_RESOURCE_TYPE_BUFFER,
					.Buffer = nosBufferInfo{
						.Size = (uint32_t)bufferSize,
						.Usage = nosBufferUsage(NOS_BUFFER_USAGE_STORAGE_BUFFER | NOS_BUFFER_USAGE_TRANSFER_SRC),
						.MemoryFlags = memoryFlags,
					}}};
			auto bufferDesc = vkss::ConvertBufferInfo(bufInfo);
			nosEngine.SetPinValueByName(NodeId, NOS_NAME_STATIC("Output"), Buffer::From(bufferDesc));
			for (size_t i = 0; i < args->PinCount; ++i)
			{
				auto& pin = args->Pins[i];
				if (pin.Name == NOS_NAME_STATIC("Channel"))
					channelInfo = InterpretPinValue<ChannelInfo>(*pin.Data);
				if (pin.Name == NOS_NAME_STATIC("Output"))
					outputBuffer = vkss::ConvertToResourceInfo(*InterpretPinValue<sys::vulkan::Buffer>(*pin.Data));
			}
		}

		if(!outputBuffer.Memory.Handle)
			return NOS_RESULT_SUCCESS;

		u8* buffer = nosVulkan->Map(&outputBuffer);

		//auto frameBufferIndex = GetFrameBufferIndex(DoubleBufferIdx);
		auto offset = GetFrameBufferOffset(Channel, DoubleBufferIdx);
		if (Interlaced())
		{
			util::Stopwatch sw;
			auto pitch = compressedExt.x * 4;
			auto segments = compressedExt.y;
			auto fieldId = fieldType == nos::sys::vulkan::FieldType::EVEN ? NTV2_FIELD0 : NTV2_FIELD1;
			Device->DmaTransfer(NTV2_DMA_FIRST_AVAILABLE, true, 0,
				const_cast<ULWord*>((u32*)buffer), // target CPU buffer address
				fieldId * pitch, // source AJA buffer address
				pitch, // length of one line
				segments, // number of lines
				pitch, // increment target buffer one line on CPU memory
				pitch * 2, // increment AJA card source buffer double the size of one line
				true);
			auto elapsed = sw.Elapsed();
			nosEngine.WatchLog(("AJA " + channelStr->str() + " DMA Read").c_str(), nos::util::Stopwatch::ElapsedString(elapsed).c_str());
		}
		else
		{
			util::Stopwatch sw;
			Device->DmaTransfer(NTV2_DMA_FIRST_AVAILABLE, true, 0, const_cast<ULWord*>((u32*)buffer), offset, bufferSize, true);
			auto elapsed = sw.Elapsed();
			nosEngine.WatchLog(("AJA " + channelStr->str() + " DMA Read").c_str(), nos::util::Stopwatch::ElapsedString(elapsed).c_str());
		}

		static_cast<nos::sys::vulkan::Buffer*>(outputPinData->Data)->mutate_field_type(fieldType);

		DoubleBufferIdx = NextDoubleBuffer(DoubleBufferIdx);

		return NOS_RESULT_SUCCESS;
	}
};

nosResult RegisterDMAReadNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMARead"), DMAReadNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}