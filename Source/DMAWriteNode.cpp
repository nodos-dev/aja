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
			nosEngine.RecompilePath(NodeId);
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

		auto buffer = nosVulkan->Map(&inputBuffer);
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