// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#pragma once
#include <Nodos/PluginHelpers.hpp>

#include <ancillarylist.h>
#include <ancillarydata.h>

#include "AJA_generated.h"

namespace nos::aja
{

// ANC flatbuffer types live in nos.mediaio. Alias them locally so call sites
// that pre-date the move don't have to be fully qualified.
using ANCFrame = nos::mediaio::ANCFrame;
using ANCFrameBuilder = nos::mediaio::ANCFrameBuilder;
using ANCPacket = nos::mediaio::ANCPacket;
using ANCPacketBuilder = nos::mediaio::ANCPacketBuilder;
using ANCDataSpace = nos::mediaio::ANCDataSpace;
using ANCDataChannel = nos::mediaio::ANCDataChannel;
using ANCDataLink = nos::mediaio::ANCDataLink;
using ANCDataStream = nos::mediaio::ANCDataStream;

// Per-channel ANC region size. 8 KB per field is the AJA SDK default and is
// enough for any realistic SMPTE 291 packet load on 12G-SDI.
static constexpr ULWord ANC_FIELD_BYTE_COUNT = 8 * 1024;

// Stand-in for CNTV2Card::NULL_POINTER (which is protected). Default-constructed
// NTV2Buffer has zero size and signals "no field 2" to the SDK.
inline NTV2Buffer& AncEmptyBuffer()
{
	static NTV2Buffer empty;
	return empty;
}

struct DMANodeBase : NodeContext
{
	DMANodeBase(nosFbNodePtr node, DMADirection dir) : NodeContext(node), Direction(dir)
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

	bool AncConfigured = false;
	bool AncInserterEnabled = false;
	uint8_t LastDmaSlot = 0;
	NTV2Buffer AncF1Buffer;
	NTV2Buffer AncF2Buffer;

	virtual void OnPathStart()
	{
		NeedsFrameSet = true;
		DoubleBufferIdx = 0;
		NextVBL = 0;
		AncConfigured = false;
		AncInserterEnabled = false;
		LastDmaSlot = 0;
	}

	void OnPathStop() override
	{
		FrameBufferOffsets.clear();
		if (AncConfigured && Device && Channel != NTV2_CHANNEL_INVALID)
		{
			const UWord sdiIndex = UWord(Channel);
			if (IsInput())
				Device->AncExtractSetEnable(sdiIndex, false);
			else
				Device->AncInsertSetEnable(sdiIndex, false);
		}
		AncConfigured = false;
		AncInserterEnabled = false;
	}

	// Lazy-allocate the host-side staging buffers for ANC F1/F2 transfer.
	void EnsureAncBuffers()
	{
		if (AncF1Buffer.GetByteCount() != ANC_FIELD_BYTE_COUNT)
			AncF1Buffer = NTV2Buffer(size_t(ANC_FIELD_BYTE_COUNT));
		if (AncF2Buffer.GetByteCount() != ANC_FIELD_BYTE_COUNT)
			AncF2Buffer = NTV2Buffer(size_t(ANC_FIELD_BYTE_COUNT));
	}

	// Initialize the AJA ANC extractor (input) or inserter (output) for the
	// current channel. Idempotent within a path run.
	bool ConfigureAnc()
	{
		if (AncConfigured || !Device || Channel == NTV2_CHANNEL_INVALID)
			return AncConfigured;
		Device->AncSetFrameBufferSize(ANC_FIELD_BYTE_COUNT, ANC_FIELD_BYTE_COUNT);

		// One-shot wipe of the ANC regions in this channel's frame buffer slots.
		// Without this, slots can carry leftover ANC bytes from a prior run /
		// different config — the inserter would emit those before our first
		// DMAWriteAnc lands, and SetFromDeviceAncBuffers would surface them on
		// the read side until the extractor overwrites the slot.
		{
			const uint32_t fbSize = Device->GetFBSize(Channel);
			const uint32_t channelCount = IsQuad() ? 4u : 1u;
			for (uint32_t c = 0; c < channelCount; ++c)
			{
				const NTV2Channel ch = NTV2Channel(Channel + c);
				const UWord slot0 = UWord(GetFrameBufferOffset(ch, 0) / fbSize);
				const UWord slot1 = UWord(GetFrameBufferOffset(ch, 1) / fbSize);
				Device->DMAClearAncRegion(slot0, slot1, NTV2_AncRgn_All, ch);
			}
		}

		const UWord sdiIndex = UWord(Channel);
		if (IsInput())
		{
			Device->AncExtractInit(sdiIndex, Channel);
			// Enable extraction across all four raster regions (VANC Y/C, HANC Y/C);
			// without this, the extractor is on but pulls no packets.
			Device->AncExtractSetComponents(sdiIndex, true, true, true, true);
			Device->AncExtractSetEnable(sdiIndex, true);
		}
		else
		{
			// Put the legacy RP188 register-based inserter in passive bypass so it
			// doesn't overwrite the ATC line that the ANC inserter writes. On cold
			// boot the register defaults to zero with bypass disabled, which would
			// emit 00:00:00:00 on the wire even when ATC packets are present in VANC.
			Device->SetRP188Mode(Channel, NTV2_RP188_OUTPUT);
			Device->EnableRP188Bypass(Channel);
			if (IsQuad())
				for (uint32_t i = Channel + 1; i < Channel + 4u; ++i)
				{
					auto ch = NTV2Channel(i);
					Device->SetRP188Mode(ch, NTV2_RP188_OUTPUT);
					Device->EnableRP188Bypass(ch);
				}
			Device->AncInsertInit(sdiIndex, Channel);
			// Enable insertion across all four raster regions (VANC Y/C, HANC Y/C);
			// without this, the inserter is armed but emits nothing.
			Device->AncInsertSetComponents(sdiIndex, true, true, true, true);
			// Prime read params with size 0 so the inserter has a defined state.
			// Do NOT enable the inserter yet — on cold boot the SDI output stack
			// may not be transmitting yet, and enabling before the first real
			// DMAWriteAnc + SetReadParams can leave it latched in a state that
			// emits nothing for the rest of the run. We enable in WriteAnc once
			// we have real ANC bytes ready in the slot the SDI is about to read.
			Device->AncInsertSetReadParams(sdiIndex, 0, 0, Channel);
			Device->AncInsertSetField2ReadParams(sdiIndex, 0, 0, Channel);
		}
		AncConfigured = true;
		return true;
	}

	// Capture: extract ANC packets from the device's ANC region for the most
	// recently transferred frame. Returns a Nodos buffer ready to push out to a pin.
	nos::Buffer ReadAnc()
	{
		EnsureAncBuffers();
		if (!ConfigureAnc())
			return {};
		const UWord sdiIndex = UWord(Channel);
		// Use the slot the most recent video DMATransfer used, not DoubleBufferIdx
		// (which has already been flipped by NextDoubleBuffer). Otherwise ANC and
		// video are read from different slots and end up out of sync.
		uint32_t frameIndex = GetFrameBufferOffset(Channel, LastDmaSlot) / Device->GetFBSize(Channel);
		Device->AncExtractSetWriteParams(sdiIndex, frameIndex, Channel);
		if (IsInterlaced())
			Device->AncExtractSetField2WriteParams(sdiIndex, frameIndex, Channel);
		// Zero before DMA so any region the extractor leaves untouched (shorter
		// frame, missing terminator) doesn't surface stale packets from the
		// previous read.
		AncF1Buffer.Fill(uint8_t(0));
		AncF2Buffer.Fill(uint8_t(0));
		Device->DMAReadAnc(frameIndex, AncF1Buffer, IsInterlaced() ? AncF2Buffer : AncEmptyBuffer(), Channel);

		AJAAncillaryList list;
		AJAAncillaryList::SetFromDeviceAncBuffers(AncF1Buffer,
			IsInterlaced() ? AncF2Buffer : AncEmptyBuffer(),
			list);

		flatbuffers::FlatBufferBuilder fbb;
		std::vector<flatbuffers::Offset<ANCPacket>> packets;
		packets.reserve(list.CountAncillaryData());
		for (uint32_t i = 0; i < list.CountAncillaryData(); ++i)
		{
			AJAAncillaryData* p = list.GetAncillaryDataAtIndex(i);
			if (!p || p->IsEmpty())
				continue;
			// Note: we don't gate on p->ChecksumOK(). The AJA hardware extractor's
			// stored checksum byte doesn't reliably match Calculate8BitChecksum of
			// the recovered payload (loopback round-trip is the worst case), so the
			// check produces false positives. Consumers do their own domain
			// validation (e.g. ExtractTimecode rejects HH>=24/MM>=60/...).
			auto payloadBytes = p->GetPayloadByteCount();
			std::vector<uint8_t> payload(payloadBytes);
			if (payloadBytes)
				p->GetPayloadData(payload.data(), uint32_t(payloadBytes));
			auto payloadOffset = fbb.CreateVector(payload);
			ANCPacketBuilder pkt(fbb);
			pkt.add_did(p->GetDID());
			pkt.add_sdid(p->GetSID());
			pkt.add_line_number(p->GetLocationLineNumber());
			pkt.add_horiz_offset(p->GetLocationHorizOffset());
			pkt.add_space(p->IsHanc() ? ANCDataSpace::HANC : ANCDataSpace::VANC);
			pkt.add_channel(p->IsLumaChannel() ? ANCDataChannel::Y :
				p->IsChromaChannel() ? ANCDataChannel::C : ANCDataChannel::Both);
			pkt.add_link(p->GetLocationVideoLink() == AJAAncDataLink_B ? ANCDataLink::B : ANCDataLink::A);
			ANCDataStream stream = ANCDataStream::Unknown;
			switch (p->GetLocationDataStream())
			{
			case AJAAncDataStream_1: stream = ANCDataStream::DS1; break;
			case AJAAncDataStream_2: stream = ANCDataStream::DS2; break;
			case AJAAncDataStream_3: stream = ANCDataStream::DS3; break;
			case AJAAncDataStream_4: stream = ANCDataStream::DS4; break;
			default:                 stream = ANCDataStream::Unknown; break;
			}
			pkt.add_stream(stream);
			pkt.add_is_field2(p->GetDataLocation().GetLineNumber() != 0 && !IsProgressivePicture(Format)
				&& p->GetLocationLineNumber() > GetDisplayHeight(Format) / 2);
			pkt.add_payload(payloadOffset);
			packets.push_back(pkt.Finish());
		}
		auto packetsVec = fbb.CreateVector(packets);
		ANCFrameBuilder frame(fbb);
		frame.add_packets(packetsVec);
		fbb.Finish(frame.Finish());
		return nos::Buffer(fbb.Release());
	}

	// Playback: serialize the incoming ANCFrame pin payload into the device's
	// ANC region for the next outgoing frame.
	void WriteAnc(const ANCFrame* incoming)
	{
		if (!incoming || !incoming->packets() || incoming->packets()->size() == 0)
			return;
		EnsureAncBuffers();
		if (!ConfigureAnc())
			return;
		AJAAncillaryList list;
		for (auto* pkt : *incoming->packets())
		{
			if (!pkt)
				continue;
			AJAAncillaryData anc;
			anc.SetDID(pkt->did());
			anc.SetSID(pkt->sdid());
			AJAAncDataLoc loc;
			loc.SetDataSpace(pkt->space() == ANCDataSpace::VANC ? AJAAncDataSpace_VANC : AJAAncDataSpace_HANC);
			loc.SetDataChannel(pkt->channel() == ANCDataChannel::C ? AJAAncDataChannel_C : AJAAncDataChannel_Y);
			loc.SetDataLink(pkt->link() == ANCDataLink::B ? AJAAncDataLink_B : AJAAncDataLink_A);
			switch (pkt->stream())
			{
			case ANCDataStream::DS1: loc.SetDataStream(AJAAncDataStream_1); break;
			case ANCDataStream::DS2: loc.SetDataStream(AJAAncDataStream_2); break;
			case ANCDataStream::DS3: loc.SetDataStream(AJAAncDataStream_3); break;
			case ANCDataStream::DS4: loc.SetDataStream(AJAAncDataStream_4); break;
			default:                 loc.SetDataStream(AJAAncDataStream_1); break;
			}
			loc.SetLineNumber(pkt->line_number());
			loc.SetHorizontalOffset(pkt->horiz_offset());
			anc.SetDataLocation(loc);
			if (auto* payload = pkt->payload(); payload && payload->size())
				anc.SetPayloadData(payload->data(), uint32_t(payload->size()));
			list.AddAncillaryData(anc);
		}
		AncF1Buffer.Fill(uint8_t(0));
		AncF2Buffer.Fill(uint8_t(0));
		list.GetTransmitData(AncF1Buffer, IsInterlaced() ? AncF2Buffer : AncEmptyBuffer(),
			!IsInterlaced(), 0);
		const UWord sdiIndex = UWord(Channel);
		// Match the slot used by the most recent video DMATransfer.
		uint32_t frameIndex = GetFrameBufferOffset(Channel, LastDmaSlot) / Device->GetFBSize(Channel);
		Device->DMAWriteAnc(frameIndex, AncF1Buffer, IsInterlaced() ? AncF2Buffer : AncEmptyBuffer(), Channel);
		Device->AncInsertSetReadParams(sdiIndex, frameIndex, AncF1Buffer.GetByteCount(), Channel);
		if (IsInterlaced())
			Device->AncInsertSetField2ReadParams(sdiIndex, frameIndex, AncF2Buffer.GetByteCount(), Channel);
		// Defer inserter enable until the first real DMAWriteAnc + SetReadParams
		// has landed; mirrors the AJA SDK sample pattern (ntv2llburn) and avoids
		// a cold-boot race where enabling pre-stream causes the inserter to stay
		// silent until the path is restarted.
		if (!AncInserterEnabled)
		{
			Device->AncInsertSetEnable(sdiIndex, true);
			AncInserterEnabled = true;
		}
	}

	void SetFrame(uint32_t doubleBufferIndex)
	{
		uint32_t frameIndex = GetFrameBufferOffset(Channel, doubleBufferIndex) / Device->GetFBSize(Channel);
		IsInput() ? Device->SetInputFrame(Channel, frameIndex)
			: Device->SetOutputFrame(Channel, frameIndex);
		if (IsQuad())
			for (uint32_t i = Channel + 1; i < Channel + 4u; ++i)
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

	uint32_t GetFrameBufferOffset(NTV2Channel channel, uint8_t frame)
	{
		auto it = FrameBufferOffsets.find(channel);
		if (it == FrameBufferOffsets.end())
			it = FrameBufferOffsets.insert({ channel, {} }).first;
		auto offsetIt = it->second.find(frame);
		if (offsetIt == it->second.end())
			offsetIt = it->second.insert({ frame, GetMaxFrameBufferSize() * 2 * channel + (uint32_t(frame) & 1) * Device->GetFBSize(channel) }).first;
		assert(offsetIt->second <= UINT32_MAX);
		return uint32_t(offsetIt->second);
	}

	struct DMAInfo {
		nosVec2u CompressedExtent;
		size_t BufferSize;
	};

	DMAInfo GetDMAInfo()
	{
		uint32_t width, height;
		Device->GetExtent(Format, Mode, width, height);
		int BitWidth = PixelFormat == mediaio::YCbCrPixelFormat::YUV8 ? 8 : 10;
		nosVec2u compressedExt((10 == BitWidth) ? ((width + (48 - width % 48) % 48) / 3) << 1 : width >> 1, height >> uint32_t(IsInterlaced()));
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
					const_cast<ULWord*>((uint32_t*)buffer), // target CPU buffer address
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
				Device->DmaTransfer(NTV2_DMA_FIRST_AVAILABLE, IsInput(), 0, const_cast<ULWord*>((uint32_t*)buffer),
					offset, uint32_t(bufferSize), true);
				auto elapsed = sw.Elapsed();
				nosEngine.WatchLog(("AJA " + ChannelName + (IsInput() ? " DMA Read" : " DMA Write")).c_str(),
					nos::util::Stopwatch::ElapsedString(elapsed).c_str());
			}
		}

		// Remember which slot this DMA used, before NextDoubleBuffer flips state.
		// ReadAnc/WriteAnc need this to keep ANC paired with the same slot the
		// video frame just hit.
		LastDmaSlot = DoubleBufferIdx;
		DoubleBufferIdx = NextDoubleBuffer(DoubleBufferIdx);

		ULWord newVBLCount = 0;
		if (Direction == DMA_READ)
			Device->GetInputVerticalInterruptCount(newVBLCount, Channel);
		else
			Device->GetOutputVerticalInterruptCount(newVBLCount, Channel);
		// DMA likely skipped a frame
		if (curVBLCount != newVBLCount)
		{
#if NOS_AJA_DIAGNOSTICS
			nosEngine.LogI("AJA %s DMA Dropped", ChannelName.c_str());
#endif
			nosEngine.TriggerNodeEvent(NodeId, NOS_NAME("Drop"));
		}

		NextVBL = newVBLCount + 1;
	}
};

}