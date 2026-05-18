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
struct DMAReadNodeContext : DMANodeBase
{
	DMAReadNodeContext(nosFbNodePtr node) : DMANodeBase(node, DMA_READ)
	{
	}

	// Cache last values so we don't restart the path on every redundant pin
	// write. The engine delivers every pin write to OnPinValueChanged whether
	// or not the bytes changed.
	std::optional<bool> LastEnableANC;
	std::optional<bool> LastEnableTimecode;
	std::optional<ATCSource> LastTimecodeSource;

	void OnPinValueChanged(nos::Name pinName, uuid const& pinId, nosBuffer value) override
	{
		if (pinName == NOS_NAME_STATIC("EnableANC"))
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
		else if (pinName == NOS_NAME_STATIC("TimecodeSource"))
		{
			if (value.Size < sizeof(ATCSource))
				return;
			const ATCSource v = *static_cast<const ATCSource*>(value.Data);
			if (LastTimecodeSource && *LastTimecodeSource == v)
				return;
			LastTimecodeSource = v;
			nosEngine.SendPathRestart(NodeId);
		}
	}

	nosResult ExecuteNode(nosNodeExecuteParams* params) override
	{
		NodeExecuteParams execParams = params;
		nosResourceShareInfo bufferToWrite = vkss::ConvertToResourceInfo(
			*execParams.GetPinData<sys::vulkan::Buffer>(NOS_NAME_STATIC("BufferToWrite")));
		auto fieldType = *execParams.GetPinData<sys::vulkan::FieldType>(NOS_NAME_STATIC("FieldType"));
		ChannelInfo* channelInfo = execParams.GetPinData<ChannelInfo>(NOS_NAME_STATIC("Channel"));
		uint32_t curVBLCount = *execParams.GetPinData<uint32_t>(NOS_NAME_STATIC("CurrentVBL"));
		bool enableANC = false;
		if (auto* p = execParams.GetPinData<bool>(NOS_NAME_STATIC("EnableANC")))
			enableANC = *p;
		bool enableTimecode = false;
		if (auto* p = execParams.GetPinData<bool>(NOS_NAME_STATIC("EnableTimecode")))
			enableTimecode = *p;
		ATCSource timecodeSource = ATCSource::Auto;
		if (auto* p = execParams.GetPinData<ATCSource>(NOS_NAME_STATIC("TimecodeSource")))
			timecodeSource = *p;

		if (!channelInfo->device())
			return NOS_RESULT_FAILED;

		Device = AJADevice::GetDeviceBySerialNumber(channelInfo->device()->serial_number());
		if (!Device) {
			nosEngine.LogE("Device not found!");
			return NOS_RESULT_FAILED;
		}
		auto channelStr = channelInfo->channel_name();
		if (!channelStr)
			return NOS_RESULT_FAILED;
		ChannelName = channelStr->str();
		Channel = ParseChannel(ChannelName);
		Format = NTV2VideoFormat(channelInfo->video_format_idx());
		PixelFormat = channelInfo->frame_buffer_format();
		if (channelInfo->is_quad())
			Mode = static_cast<AJADevice::Mode>(channelInfo->input_quad_link_mode());
		else
			Mode = AJADevice::SL;
		auto [_, bufferSize] = GetDMAInfo();

		if (!bufferToWrite.Memory.Handle)
		{
			nosEngine.LogE("DMA read target buffer is not valid.");
			return NOS_RESULT_FAILED;
		}
		if (bufferToWrite.Info.Buffer.Size != bufferSize || Format == NTV2_FORMAT_UNKNOWN)
		{
			nosEngine.LogE("DMA read target buffer size or format is not valid.");
			return NOS_RESULT_FAILED;
		}

		uint8_t* buffer = nosVulkan->Map(&bufferToWrite);
		auto inputBufferSize = bufferToWrite.Memory.Size;

		if (curVBLCount == 0)
			Device->GetInputVerticalInterruptCount(curVBLCount, Channel);

		DMATransfer(fieldType, curVBLCount, buffer, inputBufferSize);

		bufferToWrite.Info.Buffer.FieldType = (nosTextureFieldType)fieldType;

		SetPinValue(NOS_NAME_STATIC("Output"), Buffer::From(vkss::ConvertBufferInfo(bufferToWrite)));

		if (enableANC)
		{
			auto ancBuf = ReadAnc();
			if (ancBuf.Size())
				SetPinValue(NOS_NAME_STATIC("ANCFrame"), ancBuf);
		}
		if (enableTimecode)
		{
			Timecode tc{};
			if (ReadTimecode(timecodeSource, tc))
				SetPinValue(NOS_NAME_STATIC("Timecode"), nos::Buffer::From(tc));
		}

		return NOS_RESULT_SUCCESS;
	}
};

nosResult RegisterDMAReadNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMARead"), DMAReadNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}