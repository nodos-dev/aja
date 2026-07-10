// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#pragma once
#include <array>

#include <Nodos/PluginHelpers.hpp>
#include <ajantv2/includes/ntv2rp188.h>

#include <ancillarylist.h>
#include <ancillarydata.h>
#include <ntv2utils.h>

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
using ANCDataCoding = nos::mediaio::ANCDataCoding;
using Timecode = nos::mediaio::Timecode;
using ATCSource = nos::mediaio::ATCSource;

// Map an NTV2 video format to the matching TimecodeFormat for CRP188. Falls
// back to kTCFormatUnknown when the format isn't one of the SMPTE-defined
// frame rates — CRP188 then refuses to build a valid RP188 register pair.
inline TimecodeFormat NTV2FormatToTimecodeFormat(NTV2VideoFormat format)
{
	switch (GetNTV2FrameRateFromVideoFormat(format))
	{
	case NTV2_FRAMERATE_6000: return kTCFormat60fps;
	case NTV2_FRAMERATE_5994: return kTCFormat60fpsDF;
	case NTV2_FRAMERATE_5000: return kTCFormat50fps;
	case NTV2_FRAMERATE_4800: return kTCFormat48fps;
	case NTV2_FRAMERATE_4795: return kTCFormat48fps;
	case NTV2_FRAMERATE_3000: return kTCFormat30fps;
	case NTV2_FRAMERATE_2997: return kTCFormat30fpsDF;
	case NTV2_FRAMERATE_2500: return kTCFormat25fps;
	case NTV2_FRAMERATE_2400: return kTCFormat24fps;
	case NTV2_FRAMERATE_2398: return kTCFormat24fps;
	default:                  return kTCFormatUnknown;
	}
}

// CRP188 source/output-filter codes — bit 2-0 of DBB1 in SMPTE ST 12-2.
inline UByte ATCSourceToRP188Filter(ATCSource src)
{
	switch (src)
	{
	case ATCSource::ATC_VITC1: return 0x01;
	case ATCSource::ATC_VITC2: return 0x02;
	case ATCSource::Auto:      return 0xFF; // "any" filter
	case ATCSource::ATC_LTC:
	default:                   return 0x00;
	}
}

inline ATCSource RP188FilterToATCSource(UByte filter)
{
	switch (filter & 0x07)
	{
	case 0x01: return ATCSource::ATC_VITC1;
	case 0x02: return ATCSource::ATC_VITC2;
	default:   return ATCSource::ATC_LTC;
	}
}

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
	bool RP188Configured = false;
	UByte RP188Filter = 0xFF; // Tracks the last filter passed to SetRP188SourceFilter
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
		RP188Configured = false;
		RP188Filter = 0xFF;
		LastDmaSlot = 0;
	}

	void OnPathStop() override
	{
		FrameBufferOffsets.clear();
		if (AncConfigured && Device && Channel != NTV2_CHANNEL_INVALID)
		{
			const uint32_t channelCount = AncChannelCount();
			for (uint32_t c = 0; c < channelCount; ++c)
			{
				const UWord sdiIndex = UWord(Channel + c);
				if (IsInput())
					Device->AncExtractSetEnable(sdiIndex, false);
				else
					Device->AncInsertSetEnable(sdiIndex, false);
			}
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

	uint32_t AncChannelCount() const { return IsQuad() ? 4u : 1u; }

	// Initialize the AJA ANC extractor (input) or inserter (output) for the
	// current channel. Idempotent within a path run.
	bool ConfigureAnc()
	{
		if (AncConfigured || !Device || Channel == NTV2_CHANNEL_INVALID)
			return AncConfigured;
		Device->AncSetFrameBufferSize(ANC_FIELD_BYTE_COUNT, ANC_FIELD_BYTE_COUNT);

		const uint32_t channelCount = AncChannelCount();

		// One-shot wipe of the ANC regions in this channel's frame buffer slots.
		// Without this, slots can carry leftover ANC bytes from a prior run /
		// different config — the inserter would emit those before our first
		// DMAWriteAnc lands, and SetFromDeviceAncBuffers would surface them on
		// the read side until the extractor overwrites the slot.
		{
			const uint32_t fbSize = Device->GetFBSize(Channel);
			for (uint32_t c = 0; c < channelCount; ++c)
			{
				const NTV2Channel ch = NTV2Channel(Channel + c);
				const UWord slot0 = UWord(GetFrameBufferOffset(ch, 0) / fbSize);
				const UWord slot1 = UWord(GetFrameBufferOffset(ch, 1) / fbSize);
				Device->DMAClearAncRegion(slot0, slot1, NTV2_AncRgn_All, ch);
			}
		}

		if (IsInput())
		{
			// Initialize an extractor per SDI spigot. In quad-link, the four
			// sub-streams can carry independent packets — extracting only on the
			// base SDI silently drops anything on DS2..DS4.
			// Keep the extractor's default filter, which excludes the SMPTE 299M
			// embedded-audio DIDs (0xE0-0xE7, 0xA0-0xA7). Audio is already read separately via DMAReadAudio.
			const NTV2DIDSet audioFilter = CNTV2Card::AncExtractGetDefaultDIDs();
			for (uint32_t c = 0; c < channelCount; ++c)
			{
				const NTV2Channel ch = NTV2Channel(Channel + c);
				const UWord sdiIndex = UWord(ch);
				Device->AncExtractInit(sdiIndex, ch);
				Device->AncExtractSetFilterDIDs(sdiIndex, audioFilter);
				// Enable extraction across all four raster regions (VANC Y/C, HANC Y/C);
				// without this, the extractor is on but pulls no packets.
				Device->AncExtractSetComponents(sdiIndex, true, true, true, true);
				Device->AncExtractSetEnable(sdiIndex, true);
			}
		}
		else
		{
			// The AJA card emits ATC via two independent paths:
			//   1) The dedicated RP188 hardware emitter, sourced from the per-channel
			//      RP188 DBB/Low/High registers (SetRP188Data) when bypass is DISABLED,
			//      or from a routed SDI input's RP188 (SetRP188BypassSource) when
			//      bypass is ENABLED.
			//   2) The ANC inserter, which serializes whatever bytes DMAWriteAnc
			//      placed in the channel's ANC region.
			// Per AJA SDK docs ("ancillarydata.html"), packets the hardware embeds
			// automatically — audio control, ATC, VPID, EDH — should NOT be sent
			// through the inserter as well, or the streams collide on the wire.
			// We therefore drive ATC exclusively through the dedicated emitter
			// (DisableRP188Bypass + SetRP188Data, the ntv2llburn pattern) and
			// strip 0x60/0x60 packets out of the inserter list in WriteAnc.
			for (uint32_t c = 0; c < channelCount; ++c)
			{
				const NTV2Channel ch = NTV2Channel(Channel + c);
				Device->SetRP188Mode(ch, NTV2_RP188_OUTPUT);
				Device->DisableRP188Bypass(ch);
				const UWord sdiIndex = UWord(ch);
				Device->AncInsertInit(sdiIndex, ch);
				// Enable insertion across all four raster regions (VANC Y/C, HANC Y/C);
				// without this, the inserter is armed but emits nothing.
				Device->AncInsertSetComponents(sdiIndex, true, true, true, true);
				// Prime read params with size 0 so the inserter has a defined state.
				// Do NOT enable the inserter yet — on cold boot the SDI output stack
				// may not be transmitting yet, and enabling before the first real
				// DMAWriteAnc + SetReadParams can leave it latched in a state that
				// emits nothing for the rest of the run. We enable in WriteAnc once
				// we have real ANC bytes ready in the slot the SDI is about to read.
				Device->AncInsertSetReadParams(sdiIndex, 0, 0, ch);
				Device->AncInsertSetField2ReadParams(sdiIndex, 0, 0, ch);
			}
		}
		AncConfigured = true;
		return true;
	}

	// Which quad-link sub-channel a packet should be routed to on write.
	// Unknown / unspecified falls back to the base channel.
	uint32_t OffsetForStream(ANCDataStream stream) const
	{
		const uint32_t cnt = AncChannelCount();
		uint32_t c = 0;
		switch (stream)
		{
		case ANCDataStream::DS2: c = 1; break;
		case ANCDataStream::DS3: c = 2; break;
		case ANCDataStream::DS4: c = 3; break;
		default: c = 0; break;
		}
		return c < cnt ? c : 0;
	}

	// Translate one parsed AJAAncillaryData packet into a flatbuffer ANCPacket.
	// `isField2` is supplied by the caller, since AJAAncDataLoc itself does not
	// carry F1/F2 info — we tag from which buffer the SDK parsed the packet.
	static flatbuffers::Offset<ANCPacket> SerializePacket(
		flatbuffers::FlatBufferBuilder& fbb, AJAAncillaryData& p, bool isField2)
	{
		// Note: we don't gate on p.ChecksumOK(). The AJA hardware extractor's
		// stored checksum byte doesn't reliably match Calculate8BitChecksum of
		// the recovered payload (loopback round-trip is the worst case), so the
		// check produces false positives. Consumers do their own domain
		// validation (e.g. ExtractTimecode rejects HH>=24/MM>=60/...).
		auto payloadBytes = p.GetPayloadByteCount();
		std::vector<uint8_t> payload(payloadBytes);
		if (payloadBytes)
			p.GetPayloadData(payload.data(), uint32_t(payloadBytes));
		auto payloadOffset = fbb.CreateVector(payload);

		// Note: the SDK aliases AJAAncDataChannel_Both == AJAAncDataChannel_C,
		// so anything not luma is indistinguishable from chroma on read.
		ANCDataChannel channel = p.GetDataLocation().IsLumaChannel()
			? ANCDataChannel::Y
			: ANCDataChannel::C;

		ANCDataStream stream = ANCDataStream::Unknown;
		switch (p.GetLocationDataStream())
		{
		case AJAAncDataStream_1: stream = ANCDataStream::DS1; break;
		case AJAAncDataStream_2: stream = ANCDataStream::DS2; break;
		case AJAAncDataStream_3: stream = ANCDataStream::DS3; break;
		case AJAAncDataStream_4: stream = ANCDataStream::DS4; break;
		default:                 stream = ANCDataStream::Unknown; break;
		}

		ANCDataCoding coding = ANCDataCoding::Digital;
		switch (p.GetDataCoding())
		{
		case AJAAncDataCoding_Digital: coding = ANCDataCoding::Digital; break;
		case AJAAncDataCoding_Raw:     coding = ANCDataCoding::Raw;     break;
		default:                       coding = ANCDataCoding::Unknown; break;
		}

		ANCPacketBuilder pkt(fbb);
		pkt.add_did(p.GetDID());
		pkt.add_sdid(p.GetSID());
		pkt.add_line_number(p.GetLocationLineNumber());
		pkt.add_horiz_offset(p.GetLocationHorizOffset());
		pkt.add_space(p.IsHanc() ? ANCDataSpace::HANC : ANCDataSpace::VANC);
		pkt.add_channel(channel);
		pkt.add_link(p.GetLocationVideoLink() == AJAAncDataLink_B ? ANCDataLink::B : ANCDataLink::A);
		pkt.add_stream(stream);
		pkt.add_is_field2(isField2);
		pkt.add_coding(coding);
		pkt.add_payload(payloadOffset);
		return pkt.Finish();
	}

	// Inverse of SerializePacket — populate an AJAAncillaryData from a flatbuffer
	// packet ready for AJAAncillaryList::AddAncillaryData.
	static AJAAncillaryData DeserializePacket(const ANCPacket& pkt)
	{
		AJAAncillaryData anc;
		anc.SetDID(pkt.did());
		anc.SetSID(pkt.sdid());
		AJAAncDataLoc loc;
		loc.SetDataSpace(pkt.space() == ANCDataSpace::VANC ? AJAAncDataSpace_VANC : AJAAncDataSpace_HANC);
		AJAAncDataChannel ch = AJAAncDataChannel_Y;
		switch (pkt.channel())
		{
		case ANCDataChannel::Y:    ch = AJAAncDataChannel_Y; break;
		case ANCDataChannel::C:    ch = AJAAncDataChannel_C; break;
		case ANCDataChannel::Both: ch = AJAAncDataChannel_Both; break;
		default:                   ch = AJAAncDataChannel_Y; break;
		}
		loc.SetDataChannel(ch);
		loc.SetDataLink(pkt.link() == ANCDataLink::B ? AJAAncDataLink_B : AJAAncDataLink_A);
		switch (pkt.stream())
		{
		case ANCDataStream::DS1: loc.SetDataStream(AJAAncDataStream_1); break;
		case ANCDataStream::DS2: loc.SetDataStream(AJAAncDataStream_2); break;
		case ANCDataStream::DS3: loc.SetDataStream(AJAAncDataStream_3); break;
		case ANCDataStream::DS4: loc.SetDataStream(AJAAncDataStream_4); break;
		default:                 loc.SetDataStream(AJAAncDataStream_1); break;
		}
		loc.SetLineNumber(pkt.line_number());
		loc.SetHorizontalOffset(pkt.horiz_offset());
		anc.SetDataLocation(loc);
		switch (pkt.coding())
		{
		case ANCDataCoding::Digital: anc.SetDataCoding(AJAAncDataCoding_Digital); break;
		case ANCDataCoding::Raw:     anc.SetDataCoding(AJAAncDataCoding_Raw);     break;
		default:                     anc.SetDataCoding(AJAAncDataCoding_Unknown); break;
		}
		if (auto* payload = pkt.payload(); payload && payload->size())
			anc.SetPayloadData(payload->data(), uint32_t(payload->size()));
		return anc;
	}

	// Capture: extract ANC packets from the device's ANC region for the most
	// recently transferred frame. Returns a Nodos buffer ready to push out to a pin.
	nos::Buffer ReadAnc()
	{
		EnsureAncBuffers();
		if (!ConfigureAnc())
			return {};

		flatbuffers::FlatBufferBuilder fbb;
		std::vector<flatbuffers::Offset<ANCPacket>> packets;

		const uint32_t channelCount = AncChannelCount();
		const bool interlaced = IsInterlaced();
		for (uint32_t c = 0; c < channelCount; ++c)
		{
			const NTV2Channel ch = NTV2Channel(Channel + c);
			const UWord sdiIndex = UWord(ch);
			// Use the slot the most recent video DMATransfer used, not DoubleBufferIdx
			// (which has already been flipped by NextDoubleBuffer). Otherwise ANC and
			// video are read from different slots and end up out of sync.
			const uint32_t frameIndex = GetFrameBufferOffset(ch, LastDmaSlot) / Device->GetFBSize(ch);
			Device->AncExtractSetWriteParams(sdiIndex, frameIndex, ch);
			if (interlaced)
				Device->AncExtractSetField2WriteParams(sdiIndex, frameIndex, ch);

			// Query how many bytes the extractor actually wrote so we can detect
			// overflow. We can't shrink the DMA below the buffer size, but we do
			// log when the extractor reports more than fits. ntv2llburn does the
			// same.
			ULWord f1Bytes = 0, f2Bytes = 0;
			Device->AncExtractGetField1Size(sdiIndex, f1Bytes);
			if (interlaced)
				Device->AncExtractGetField2Size(sdiIndex, f2Bytes);
			if (f1Bytes > ANC_FIELD_BYTE_COUNT || f2Bytes > ANC_FIELD_BYTE_COUNT)
			{
				nosEngine.LogW("AJA %s SDI%u ANC extractor overflow: F1=%u F2=%u (cap=%u)",
					ChannelName.c_str(), unsigned(sdiIndex), unsigned(f1Bytes), unsigned(f2Bytes),
					unsigned(ANC_FIELD_BYTE_COUNT));
			}

			// Zero before DMA so any region the extractor leaves untouched (shorter
			// frame, missing terminator) doesn't surface stale packets from the
			// previous read.
			AncF1Buffer.Fill(uint8_t(0));
			AncF2Buffer.Fill(uint8_t(0));
			Device->DMAReadAnc(frameIndex, AncF1Buffer, interlaced ? AncF2Buffer : AncEmptyBuffer(), ch);

			// Parse F1 and F2 separately so we know which field each packet came
			// from — AJAAncDataLoc does not carry that info on its own.
			AJAAncillaryList listF1, listF2;
			AJAAncillaryList::SetFromDeviceAncBuffers(AncF1Buffer, AncEmptyBuffer(), listF1);
			if (interlaced)
				AJAAncillaryList::SetFromDeviceAncBuffers(AncF2Buffer, AncEmptyBuffer(), listF2);

			auto appendList = [&](AJAAncillaryList& list, bool isF2) {
				for (uint32_t i = 0; i < list.CountAncillaryData(); ++i)
				{
					AJAAncillaryData* p = list.GetAncillaryDataAtIndex(i);
					if (!p || p->IsEmpty())
						continue;
					packets.push_back(SerializePacket(fbb, *p, isF2));
				}
			};
			appendList(listF1, false);
			if (interlaced)
				appendList(listF2, true);
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

		const uint32_t channelCount = AncChannelCount();
		const bool interlaced = IsInterlaced();
		// SMPTE 2110-40 channels expect RTP-encapsulated ANC; the AJA SDK
		// exposes a separate transmit path (GetIPTransmitData / RTP buffer
		// format) for them. SDI / GUMP is the default for everything else.
		const bool isIP = Device->features().CanDo2110();

		// Route packets into per-SDI, per-field lists. Splitting by field
		// ourselves (rather than relying on GetTransmitData's line-number
		// heuristic) honors the caller-supplied is_field2 even when line_number
		// is 0/DontCare or out of the F2 raster range.
		std::vector<std::array<AJAAncillaryList, 2>> lists(channelCount);
		for (auto* pkt : *incoming->packets())
		{
			if (!pkt)
				continue;
			// ATC is driven by the dedicated RP188 hardware emitter via the
			// EnableTimecode / Timecode pins on DMAWrite (SetRP188Data). The
			// hardware auto-embeds ATC packets, and per AJA SDK guidance we
			// must not send the same packet through the inserter as well, or
			// the two paths collide on the wire. Drop 0x60/0x60 here.
			if (pkt->did() == 0x60 && pkt->sdid() == 0x60)
				continue;
			const uint32_t c = OffsetForStream(pkt->stream());
			AJAAncillaryData anc = DeserializePacket(*pkt);
			if (isIP)
				anc.SetBufferFormat(AJAAncBufferFormat_RTP);
			const uint8_t field = (interlaced && pkt->is_field2()) ? 1 : 0;
			lists[c][field].AddAncillaryData(anc);
		}

		for (uint32_t c = 0; c < channelCount; ++c)
		{
			const NTV2Channel ch = NTV2Channel(Channel + c);
			const UWord sdiIndex = UWord(ch);
			const uint32_t frameIndex = GetFrameBufferOffset(ch, LastDmaSlot) / Device->GetFBSize(ch);

			AncF1Buffer.Fill(uint8_t(0));
			AncF2Buffer.Fill(uint8_t(0));
			// inIsProgressive=true forces every packet into the first buffer
			// argument, so we can populate F1 and F2 independently from our
			// pre-split lists. GetIPTransmitData emits the same RTP framing
			// the 2110-40 ANC engine expects; GetTransmitData emits the
			// GUMP framing the SDI ANC engine expects.
			auto emit = [&](AJAAncillaryList& list, NTV2Buffer& dst) {
				if (isIP)
					list.GetIPTransmitData(dst, AncEmptyBuffer(), /*progressive*/ true, 0);
				else
					list.GetTransmitData(dst, AncEmptyBuffer(), /*progressive*/ true, 0);
			};
			emit(lists[c][0], AncF1Buffer);
			if (interlaced)
				emit(lists[c][1], AncF2Buffer);

			Device->DMAWriteAnc(frameIndex, AncF1Buffer, interlaced ? AncF2Buffer : AncEmptyBuffer(), ch);
			Device->AncInsertSetReadParams(sdiIndex, frameIndex, AncF1Buffer.GetByteCount(), ch);
			if (interlaced)
				Device->AncInsertSetField2ReadParams(sdiIndex, frameIndex, AncF2Buffer.GetByteCount(), ch);
		}

		// Defer inserter enable until the first real DMAWriteAnc + SetReadParams
		// has landed; mirrors the AJA SDK sample pattern (ntv2llburn) and avoids
		// a cold-boot race where enabling pre-stream causes the inserter to stay
		// silent until the path is restarted.
		if (!AncInserterEnabled)
		{
			for (uint32_t c = 0; c < channelCount; ++c)
				Device->AncInsertSetEnable(UWord(Channel + c), true);
			AncInserterEnabled = true;
		}
	}

	// Output: drive the AJA card's dedicated RP188 emitter from a decoded
	// Timecode struct. Mirrors ntv2llburn's SetRP188Data per-frame call.
	// Returns false if the channel's video format isn't a known SMPTE rate.
	bool WriteTimecode(const Timecode& tc)
	{
		if (!Device || Channel == NTV2_CHANNEL_INVALID)
			return false;
		const TimecodeFormat fmt = NTV2FormatToTimecodeFormat(Format);
		if (fmt == kTCFormatUnknown)
			return false;
		const NTV2FrameRate rate = GetNTV2FrameRateFromVideoFormat(Format);

		CRP188 rp188;
		rp188.SetRP188(ULWord(tc.frames()), ULWord(tc.seconds()),
			ULWord(tc.minutes()), ULWord(tc.hours()),
			rate, tc.drop_frame());
		rp188.SetSource(ATCSourceToRP188Filter(tc.source()));

		NTV2_RP188 reg;
		if (!rp188.GetRP188Reg(reg))
			return false;

		const uint32_t channelCount = AncChannelCount();
		for (uint32_t c = 0; c < channelCount; ++c)
			Device->SetRP188Data(NTV2Channel(Channel + c), reg);
		return true;
	}

	// Input: configure the RP188 receiver (mode + flavor filter) once per
	// path-run and per filter change.
	void ConfigureRP188Input(ATCSource source)
	{
		if (!Device || Channel == NTV2_CHANNEL_INVALID)
			return;
		const UByte filter = ATCSourceToRP188Filter(source);
		if (RP188Configured && filter == RP188Filter)
			return;
		const uint32_t channelCount = AncChannelCount();
		for (uint32_t c = 0; c < channelCount; ++c)
		{
			const NTV2Channel ch = NTV2Channel(Channel + c);
			Device->SetRP188Mode(ch, NTV2_RP188_INPUT);
			Device->SetRP188SourceFilter(ch, filter);
		}
		RP188Filter = filter;
		RP188Configured = true;
	}

	// Input: read the latest RP188 ATC seen on this SDI input and decode it
	// into a Timecode struct. Returns false when the hardware reports the
	// register as invalid (no fresh ATC since the last read).
	bool ReadTimecode(ATCSource source, Timecode& outTc)
	{
		if (!Device || Channel == NTV2_CHANNEL_INVALID)
			return false;
		ConfigureRP188Input(source);

		NTV2_RP188 reg;
		Device->GetRP188Data(Channel, reg);
		if (!reg.IsValid())
			return false;

		const TimecodeFormat fmt = NTV2FormatToTimecodeFormat(Format);
		CRP188 rp188(reg, fmt == kTCFormatUnknown ? kTCFormat30fps : fmt);

		ULWord h = 0, m = 0, s = 0, f = 0;
		if (!rp188.GetRP188Hrs(h) || !rp188.GetRP188Mins(m) ||
			!rp188.GetRP188Secs(s) || !rp188.GetRP188Frms(f))
			return false;

		outTc = Timecode(
			uint8_t(h), uint8_t(m), uint8_t(s), uint8_t(f),
			rp188.DropFrame(),
			RP188FilterToATCSource(rp188.GetSource()));
		return true;
	}

	void SetFrame(uint32_t doubleBufferIndex)
	{
		const uint32_t frameIndex = GetFrameBufferOffset(Channel, doubleBufferIndex) / Device->GetFBSize(Channel);
		const uint32_t channelCount = AncChannelCount();
		for (uint32_t c = 0; c < channelCount; ++c)
		{
			const NTV2Channel ch = NTV2Channel(Channel + c);
			IsInput() ? Device->SetInputFrame(ch, frameIndex)
				: Device->SetOutputFrame(ch, frameIndex);
		}
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