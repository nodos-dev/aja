// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include <Nodos/PluginHelpers.hpp>

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

	void OnPinValueChanged(nos::Name pinName, uuid const& pinId, nosBuffer value) override
	{
		if (pinName == NOS_NAME_STATIC("EnableTimecode") || pinName == NOS_NAME_STATIC("TimecodeSource"))
			nosEngine.SendPathRestart(NodeId);
	}

	nosResult ExecuteNode(nosNodeExecuteParams* params) override
	{
		NodeExecuteParams execParams = params;
		nosResourceShareInfo bufferToWrite = vkss::ConvertToResourceInfo(
			*execParams.GetPinData<sys::vulkan::Buffer>(NOS_NAME_STATIC("BufferToWrite")));
		auto fieldType = *execParams.GetPinData<sys::vulkan::FieldType>(NOS_NAME_STATIC("FieldType"));
		ChannelInfo* channelInfo = execParams.GetPinData<ChannelInfo>(NOS_NAME_STATIC("Channel"));
		uint32_t curVBLCount = *execParams.GetPinData<uint32_t>(NOS_NAME_STATIC("CurrentVBL"));
		bool enableRP188 = *execParams.GetPinData<bool>(NOS_NAME_STATIC("EnableTimecode"));
		auto timecodeSource = *execParams.GetPinData<TimecodeSource>(NOS_NAME_STATIC("TimecodeSource"));
		bool enableANC = false;
		if (auto* p = execParams.GetPinData<bool>(NOS_NAME_STATIC("EnableANC")))
			enableANC = *p;

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

		// Read RP188 before DMA to ensure timecode matches the frame about to be transferred.
		RP188Result rp188;
		if (enableRP188)
			rp188 = ReadRP188(GetRP188SourceFilterValue(timecodeSource));

		DMATransfer(fieldType, curVBLCount, buffer, inputBufferSize);

		bufferToWrite.Info.Buffer.FieldType = (nosTextureFieldType)fieldType;

		SetPinValue(NOS_NAME_STATIC("Output"), Buffer::From(vkss::ConvertBufferInfo(bufferToWrite)));

		if (enableRP188)
		{
			auto tcLabel = "AJA " + ChannelName + " TC In (" + EnumNameTimecodeSource(timecodeSource) + ")";
			if (!rp188.Valid)
			{
				nosEngine.WatchLog(tcLabel.c_str(), "No valid timecode");
				SetPinValue(NOS_NAME_STATIC("TimecodeFrameNumber"), Buffer::From(uint32_t(0)));
			}
			else
			{
				nosEngine.WatchLog(tcLabel.c_str(), rp188.TimecodeStr.c_str());
				SetPinValue(NOS_NAME_STATIC("TimecodeFrameNumber"), Buffer::From(rp188.FrameNumber));
			}
		}

		if (enableANC)
		{
			auto ancBuf = ReadAnc();
			if (ancBuf.Size())
				SetPinValue(NOS_NAME_STATIC("ANCFrame"), ancBuf);
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