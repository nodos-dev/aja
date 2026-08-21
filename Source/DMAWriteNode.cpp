// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include <Nodos/PluginHelpers.hpp>

#include <optional>

// External
#include <nosVulkanSubsystem/nosVulkanSubsystem.h>
#include <nosVulkanSubsystem/Helpers.hpp>
#include <nosUtil/Stopwatch.hpp>

#include "AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"
#include "DMANodeBase.hpp"

namespace nos::aja
{

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
	DMAWriteNodeContext(nosFbNodePtr node) : DMANodeBase(node, DMA_WRITE)
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

	void GetScheduleInfo(nosScheduleInfo* out) override
	{
		*out = nosScheduleInfo{
			.Importance = 1,
			.DeltaSeconds = GetDeltaSeconds(Format, IsInterlaced()),
			.Type = NOS_SCHEDULE_TYPE_ON_DEMAND,
		};
	}
 
	void OnPinValueChanged(nos::Name pinName, uuid const& pinId, nosBuffer value) override
	{
		if (pinName == NOS_NAME_STATIC("Channel"))
		{
			if (LastChannelInfo.Size() == value.Size && memcmp(LastChannelInfo.Data(), value.Data, value.Size) == 0)
				return;
			auto* channelInfo = InterpretPinValue<ChannelInfo>(value);
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
			const auto v = *InterpretPinValue<nos::mediaio::ColorSpace>(value);
			if (v == ColorSpace)
				return;
			ColorSpace = v;
			VPIDDirty = true;
		}
		else if (pinName == NOS_NAME_STATIC("GammaCurve"))
		{
			const auto v = *InterpretPinValue<nos::mediaio::GammaCurve>(value);
			if (v == GammaCurve)
				return;
			GammaCurve = v;
			VPIDDirty = true;
		}
		else if (pinName == NOS_NAME_STATIC("Luminance"))
		{
			const auto v = *InterpretPinValue<nos::mediaio::VPIDLuminance>(value);
			if (v == Luminance)
				return;
			Luminance = v;
			VPIDDirty = true;
		}
		else if (pinName == NOS_NAME_STATIC("SignalRange"))
		{
			const auto v = *InterpretPinValue<nos::mediaio::SignalRange>(value);
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
	
	nosResult ExecuteNode(nosNodeExecuteParams* params) override
	{
		NodeExecuteParams execParams = params;
		nosResourceShareInfo inputBuffer = vkss::ConvertToResourceInfo(
			*execParams.GetPinData<sys::vulkan::Buffer>(NOS_NAME_STATIC("Input")));
		auto fieldType = *execParams.GetPinData<sys::vulkan::FieldType>(NOS_NAME_STATIC("FieldType"));
		auto curVBLCount = *execParams.GetPinData<uint32_t>(NOS_NAME_STATIC("CurrentVBL"));
		bool enableANC = false;
		if (auto* p = execParams.GetPinData<bool>(NOS_NAME_STATIC("EnableANC")))
			enableANC = *p;
		const ANCFrame* ancIncoming = nullptr;
		if (enableANC)
			ancIncoming = execParams.GetPinData<ANCFrame>(NOS_NAME_STATIC("ANCFrame"));

		bool enableTimecode = false;
		if (auto* p = execParams.GetPinData<bool>(NOS_NAME_STATIC("EnableTimecode")))
			enableTimecode = *p;
		const Timecode* timecode = nullptr;
		if (enableTimecode)
			timecode = execParams.GetPinData<Timecode>(NOS_NAME_STATIC("Timecode"));

		if (!inputBuffer.Memory.Handle || !Device || Format == NTV2_FORMAT_UNKNOWN)
			return NOS_RESULT_FAILED;

		// Apply HDR VPID overrides when changed; the driver keeps them in the
		// VPID it generates, so there's no need to re-write every frame.
		if (VPIDDirty)
			ApplyVPID();

		auto buffer = nosVulkan->Map(&inputBuffer);
		if (!buffer)
		{
			nosEngine.LogE("AJA %s DMA Write could not map the input buffer for reading", ChannelName.c_str());
			return NOS_RESULT_FAILED;
		}
		auto inputSize = inputBuffer.Memory.Size;

		if (curVBLCount == 0)
			Device->GetOutputVerticalInterruptCount(curVBLCount, Channel);

		DMATransfer(fieldType, curVBLCount, buffer, inputSize);

		if (enableANC && ancIncoming)
			WriteAnc(ancIncoming);
		if (enableTimecode && timecode)
			WriteTimecode(*timecode);

		nosScheduleNodeParams schedule{.NodeId = NodeId, .AddScheduleCount = 1};
		nosEngine.ScheduleNode(&schedule);
		return NOS_RESULT_SUCCESS;
	}

	void OnPathStart() override
	{
		DMANodeBase::OnPathStart();
		nosScheduleNodeParams schedule{.NodeId = NodeId, .AddScheduleCount = 1};
		nosEngine.ScheduleNode(&schedule);
	}
};

nosResult RegisterDMAWriteNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMAWrite"), DMAWriteNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}