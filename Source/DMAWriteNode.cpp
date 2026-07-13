// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include <Nodos/Plugin.hpp>

#include <optional>

// External
#include <nosSysVulkan/nosVulkanSubsystem.h>
#include <nosSysVulkan/Helpers.hpp>
#include <Nodos/Utils/Stopwatch.hpp>

#include <nosAudio/Audio_generated.h>
#include "nosAja/AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"
#include "DMANodeBase.hpp"

namespace nos::aja
{
#define NOS_AJA_AUDIO_OUTPUT_DIAGNOSTICS 0

// Map the mediaio signalling enums (shared with the rest of the pipeline) onto the
// NTV2 VPID field enums. Curves/gamuts that SMPTE 352 can't represent are signalled
// as Unspecified/Unknown rather than mislabelled as SDR/Rec709.
static NTV2VPIDXferChars ToVPIDXfer(nos::mediaio::GammaCurve g)
{
	switch (g)
	{
	case nos::mediaio::GammaCurve::REC709: return NTV2_VPID_TC_SDR_TV;
	case nos::mediaio::GammaCurve::SRGB:   return NTV2_VPID_TC_SDR_TV;
	case nos::mediaio::GammaCurve::HLG:    return NTV2_VPID_TC_HLG;
	case nos::mediaio::GammaCurve::ST2084: return NTV2_VPID_TC_PQ;
	// IDENTITY (linear) and SLOG3 (camera log) have no broadcast transfer code.
	default:                               return NTV2_VPID_TC_Unspecified;
	}
}

static NTV2VPIDColorimetry ToVPIDColorimetry(nos::mediaio::ColorSpace c)
{
	switch (c)
	{
	// BT.601 has no VPID code; Rec709 is the standard-colorimetry default for its raster.
	case nos::mediaio::ColorSpace::REC709:  return NTV2_VPID_Color_Rec709;
	case nos::mediaio::ColorSpace::REC601:  return NTV2_VPID_Color_Rec709;
	case nos::mediaio::ColorSpace::REC2020: return NTV2_VPID_Color_UHDTV;
	// SGAMUT3 / SGAMUT3CINE are Sony wide gamuts with no VPID equivalent.
	default:                                return NTV2_VPID_Color_Unknown;
	}
}

static NTV2VPIDLuminance ToVPIDLuminance(nos::mediaio::VPIDLuminance l)
{
	return l == nos::mediaio::VPIDLuminance::ICtCp ? NTV2_VPID_Luminance_ICtCp : NTV2_VPID_Luminance_YCbCr;
}

static NTV2VPIDRGBRange ToVPIDRange(nos::mediaio::SignalRange r)
{
	return r == nos::mediaio::SignalRange::Full ? NTV2_VPID_Range_Full : NTV2_VPID_Range_Narrow;
}

struct DMAWriteNodeContext : DMANodeBase
{
	DMAWriteNodeContext()
		: DMANodeBase(DMA_WRITE)
		, LastWrittenAudioBufferOffset(0)
	{
	}

	nos::Buffer LastChannelInfo = {};
	// Cache last bool values so we don't restart the path when an upstream
	// node re-writes the same value every tick. Nosengine delivers every
	// pin write to OnPinValueChanged regardless of whether the bytes changed
	// (see the explicit memcmp dedup for Channel above).
	std::optional<bool> LastEnableANC;
	std::optional<bool> LastEnableTimecode;

	// HDR VPID signalling overrides. The driver auto-generates the output VPID and
	// exposes per-output override registers for the HDR fields; it reads those when
	// generating, so a single write persists — no per-frame re-assert, and no
	// fighting the driver the way a raw SetSDIOutVPID would. Applied on change.
	bool EnableVPID = true;
	nos::mediaio::ColorSpace ColorSpace = nos::mediaio::ColorSpace::REC709;
	nos::mediaio::GammaCurve GammaCurve = nos::mediaio::GammaCurve::REC709;
	nos::mediaio::VPIDLuminance Luminance = nos::mediaio::VPIDLuminance::YCbCr;
	nos::mediaio::SignalRange SignalRange = nos::mediaio::SignalRange::Narrow;
	bool VPIDDirty = true;

	void ApplyVPID()
	{
		if (!Device || Channel == NTV2_CHANNEL_INVALID)
			return;
		Device->SetOutputVPID(Channel, Mode, EnableVPID,
		                      ToVPIDXfer(GammaCurve), ToVPIDColorimetry(ColorSpace),
		                      ToVPIDLuminance(Luminance), ToVPIDRange(SignalRange));
		VPIDDirty = false;
	}

	void OnPinValueChanged(nos::Name pinName, uuid const& pinId, nosBuffer value) override
	{
		if (pinName == NOS_NAME_STATIC("Channel"))
		{
			if (LastChannelInfo.Size() == value.Size && memcmp(LastChannelInfo.Data(), value.Data, value.Size) == 0)
				return;
			auto* channelInfo = InterpretObjectData<ChannelInfo>(value);
			Device = nullptr;
			LastChannelInfo = {};
			if (!channelInfo || !channelInfo->device())
				return;
			Device = AJADevice::GetDeviceBySerialNumber(channelInfo->device()->serial_number());
			if (!Device || !channelInfo->channel_name())
				return;
			LastChannelInfo = value;
			ChannelName = channelInfo->channel_name()->c_str();
			Channel = ParseChannel(ChannelName);
			Format = NTV2VideoFormat(channelInfo->video_format_idx());
			PixelFormat = channelInfo->frame_buffer_format();
			if (channelInfo->is_quad())
				Mode = static_cast<AJADevice::Mode>(channelInfo->output_quad_link_mode());
			else
				Mode = AJADevice::SL;
			// Routing re-generates the VPID, so re-apply the HDR overrides after it.
			VPIDDirty = true;
			nosEngine.RecompilePath(NodeId);
		}
		else if (pinName == NOS_NAME_STATIC("EnableVPID"))
		{
			if (value.Size < sizeof(bool))
				return;
			const bool v = *static_cast<const bool*>(value.Data);
			if (v == EnableVPID)
				return;
			EnableVPID = v;
			VPIDDirty = true;
		}
		else if (pinName == NOS_NAME_STATIC("ColorSpace"))
		{
			if (value.Size < sizeof(nos::mediaio::ColorSpace))
				return;
			const auto v = *static_cast<const nos::mediaio::ColorSpace*>(value.Data);
			if (v == ColorSpace)
				return;
			ColorSpace = v;
			VPIDDirty = true;
		}
		else if (pinName == NOS_NAME_STATIC("GammaCurve"))
		{
			if (value.Size < sizeof(nos::mediaio::GammaCurve))
				return;
			const auto v = *static_cast<const nos::mediaio::GammaCurve*>(value.Data);
			if (v == GammaCurve)
				return;
			GammaCurve = v;
			VPIDDirty = true;
		}
		else if (pinName == NOS_NAME_STATIC("Luminance"))
		{
			if (value.Size < sizeof(nos::mediaio::VPIDLuminance))
				return;
			const auto v = *static_cast<const nos::mediaio::VPIDLuminance*>(value.Data);
			if (v == Luminance)
				return;
			Luminance = v;
			VPIDDirty = true;
		}
		else if (pinName == NOS_NAME_STATIC("SignalRange"))
		{
			if (value.Size < sizeof(nos::mediaio::SignalRange))
				return;
			const auto v = *static_cast<const nos::mediaio::SignalRange*>(value.Data);
			if (v == SignalRange)
				return;
			SignalRange = v;
			VPIDDirty = true;
		}
		else if (pinName == NOS_NAME_STATIC("EnableANC"))
		{
			if (value.Size < sizeof(bool))
				return;
			const bool v = *static_cast<const bool*>(value.Data);
			if (LastEnableANC && *LastEnableANC == v)
				return;
			LastEnableANC = v;
			nosEngine.SendPathRestart(NodeId);
		}
		else if (pinName == NOS_NAME_STATIC("EnableTimecode"))
		{
			if (value.Size < sizeof(bool))
				return;
			const bool v = *static_cast<const bool*>(value.Data);
			if (LastEnableTimecode && *LastEnableTimecode == v)
				return;
			LastEnableTimecode = v;
			nosEngine.SendPathRestart(NodeId);
		}
	}

	nosResult ExecuteNode(NodeExecuteParams const& params) override
	{
		TypedObjectRef inputBufferObject = params.GetPinObject<sys::vulkan::Buffer>(NOS_NAME("Input"));
		auto fieldType = *params.GetPinValue<sys::vulkan::FieldType>(NOS_NAME("FieldType"));
		uint32_t curVBLCount = *params.GetPinValue<uint32_t>(NOS_NAME("CurrentVBL"));
		bool enableANC = false;
		if (auto* p = params.GetPinValue<bool>(NOS_NAME("EnableANC")))
			enableANC = *p;
		const ANCFrame* ancIncoming = nullptr;
		if (enableANC)
			ancIncoming = params.GetPinValue<ANCFrame>(NOS_NAME("ANCFrame"));
		bool enableTimecode = false;
		if (auto* p = params.GetPinValue<bool>(NOS_NAME("EnableTimecode")))
			enableTimecode = *p;
		const Timecode* timecode = nullptr;
		if (enableTimecode)
			timecode = params.GetPinValue<Timecode>(NOS_NAME("Timecode"));

		if (!inputBufferObject.IsValid() || !Device || Format == NTV2_FORMAT_UNKNOWN)
			return NOS_RESULT_FAILED;

		// Apply HDR VPID overrides when changed; the driver keeps them in the
		// VPID it generates, so there's no need to re-write every frame.
		if (VPIDDirty)
			ApplyVPID();

		CompositeObjectRef audioPacket = params.GetPinObject(NOS_NAME("AudioPacket"));
		auto descObject = audioPacket.GetField<TypedObjectRef<audio::AudioPacketDescriptor>>(NOS_NAME("desc"));
		auto audioBufferObject = audioPacket.GetField<TypedObjectRef<sys::vulkan::Buffer>>(NOS_NAME("buffer"));
		bool receivingAudio = false;
		const audio::AudioPacketDescriptor* audioPacketDesc = nullptr;
		if (audioPacket && descObject && audioBufferObject)
		{
			audioPacketDesc = descObject->InterpretBuffer();
			receivingAudio = audioPacketDesc->num_samples() > 0;
		}

		bool audioPlaying = false;
		auto audioSys = NTV2AudioSystem(Channel);
		Device->SetSDIOutputAudioSystem(Channel,audioSys);
		Device->IsAudioOutputRunning(audioSys, audioPlaying);
		if (!audioPlaying && receivingAudio)
		{
			Device->SetNumberAudioChannels(audioPacketDesc->channel_count(), audioSys);
			// Start writing audio data from 0.2 seconds ahead of the play head
			LastWrittenAudioBufferOffset = 48000 / 5 * sizeof(ULWord) * audioPacketDesc->channel_count();
			Device->StartAudioOutput(audioSys, false);
			Device->SetAudioOutputEraseMode(audioSys, true);
		}
		else if (receivingAudio)
		{
			ULWord audioChannelCount{};
			Device->GetNumberAudioChannels(audioChannelCount, audioSys);
			if (audioChannelCount != audioPacketDesc->channel_count())
				if (!Device->SetNumberAudioChannels(audioPacketDesc->channel_count(), audioSys))
					nosEngine.LogE("Failed to set audio channel count for output");
		}

		auto buffer = nosVulkan->Map(inputBufferObject);
		auto inputBufferInfo = *sys::vulkan::GetResourceInfo(inputBufferObject);
		auto inputSize = inputBufferInfo.Size;

		if (curVBLCount == 0)
			Device->GetOutputVerticalInterruptCount(curVBLCount, Channel);

		DMATransfer(fieldType, curVBLCount, buffer, inputSize);

		if (enableANC && ancIncoming)
			WriteAnc(ancIncoming);
		if (enableTimecode && timecode)
			WriteTimecode(*timecode);

		ULWord wrapAddress = 0;
		Device->GetAudioWrapAddress(wrapAddress, audioSys);
		const char* status = "Skipped";
		if (receivingAudio && audioPacketDesc->num_samples() > 0)
		{
			auto audioBuffer = nosVulkan->Map(*audioBufferObject);
			if (audioBuffer)
			{
				status = "Written";
				// audioBuffer contains 32-bit words with 24-bit samples in MSB
				ULWord byteCount = audioPacketDesc->num_samples() * audioPacketDesc->channel_count() *
								   sizeof(ULWord); // 4 bytes per sample
				if (LastWrittenAudioBufferOffset + byteCount > wrapAddress)
				{
					ULWord firstPartSize = wrapAddress - LastWrittenAudioBufferOffset;
					// Write audio data up to the wrap address
					Device->DMAWriteAudio(audioSys, (const ULWord*)audioBuffer, LastWrittenAudioBufferOffset, firstPartSize);
					// Write the remaining audio data from the start
					ULWord remainingSize = byteCount - firstPartSize;
					Device->DMAWriteAudio(
						audioSys, (const ULWord*)(audioBuffer + firstPartSize), 0, remainingSize);
					LastWrittenAudioBufferOffset = remainingSize;
				}
				else
				{
					// Write audio data ahead of the play head
					// Use the current play head position as the write offset
					Device->DMAWriteAudio(audioSys, (const ULWord*)audioBuffer, LastWrittenAudioBufferOffset, byteCount);
					LastWrittenAudioBufferOffset += byteCount;
				}
			}
		}

#if NOS_AJA_AUDIO_OUTPUT_DIAGNOSTICS
		ULWord playheadPos{};
		Device->ReadAudioLastOut(playheadPos, audioSys);
		float playhead = 100.f * (float(playheadPos) / float(wrapAddress));
		float lastWritten = 100.f * (float(LastWrittenAudioBufferOffset) / float(wrapAddress));
		nosEngine.LogI("%s, Playhead: %.2f, LastWritten: %.2f", status, playhead, lastWritten);
#endif
		return NOS_RESULT_SUCCESS;
	}

	void OnPathStart() override
	{
		DMANodeBase::OnPathStart();
		LastWrittenAudioBufferOffset = 0;
	}

	void OnPathStop() override
	{
		DMANodeBase::OnPathStop();
		if (!Device || Channel == NTV2_CHANNEL_INVALID)
			return;
		NTV2AudioSystem audioSys{};
		Device->GetSDIOutputAudioSystem(Channel, audioSys);
		Device->StopAudioOutput(audioSys);
	}

	ULWord LastWrittenAudioBufferOffset;
};

nosResult RegisterDMAWriteNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMAWrite"), DMAWriteNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}
}