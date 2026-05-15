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

	enum class DMAReadNodeStatus { Unknown, Ok, Missing, SizeMismatch };
	DMAReadNodeStatus LastDMAReadNodeStatus = DMAReadNodeStatus::Unknown;

	// Only emit a node status update when the BufferToWrite state actually changes.
	// ExecuteNode runs at video frame rate, and SetNodeStatusMessage produces a
	// partial-node-update event on every call.
	void UpdateDMAReadNodeStatus(DMAReadNodeStatus newStatus, std::string message = {},
		fb::NodeStatusMessageType type = fb::NodeStatusMessageType::INFO)
	{
		if (newStatus == LastDMAReadNodeStatus)
			return;
		LastDMAReadNodeStatus = newStatus;
		if (newStatus == DMAReadNodeStatus::Ok)
			ClearNodeStatusMessages();
		else
			SetNodeStatusMessage(std::move(message), type);
	}

	void OnPinValueChanged(nos::Name pinName, uuid const& pinId, nosBuffer value) override
	{
		if (pinName == NOS_NAME_STATIC("EnableANC"))
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
		if (Format == NTV2_FORMAT_UNKNOWN)
			return NOS_RESULT_FAILED;
		auto [_, bufferSize] = GetDMAInfo();

		// BufferToWrite is optional. When connected but sized wrong, the DMA would
		// underrun or write past the destination — surface a hard error and bail.
		// When unconnected, fall through with a warning so ANC capture still runs
		// for downstream consumers that only need ANC.
		const bool bufferValid = bufferToWrite.Memory.Handle != 0;
		if (bufferValid && bufferToWrite.Info.Buffer.Size != bufferSize)
		{
			UpdateDMAReadNodeStatus(DMAReadNodeStatus::SizeMismatch,
				"BufferToWrite size " + std::to_string(bufferToWrite.Info.Buffer.Size) +
					" does not match the required " + std::to_string(bufferSize) + " bytes",
				fb::NodeStatusMessageType::FAILURE);
			return NOS_RESULT_FAILED;
		}
		if (!bufferValid)
			UpdateDMAReadNodeStatus(DMAReadNodeStatus::Missing,
				"BufferToWrite is not connected — only ANC will be read, video frames are not transferred to CPU",
				fb::NodeStatusMessageType::WARNING);
		else
			UpdateDMAReadNodeStatus(DMAReadNodeStatus::Ok);

		if (curVBLCount == 0)
			Device->GetInputVerticalInterruptCount(curVBLCount, Channel);

		// Pass nullptr when BufferToWrite is missing so DMATransfer skips the
		// CPU-side copy but still advances the frame-slot bookkeeping ReadAnc
		// relies on (LastDmaSlot, SetInputFrame via NextDoubleBuffer).
		uint8_t* buffer = bufferValid ? nosVulkan->Map(&bufferToWrite) : nullptr;
		uint64_t inputBufferSize = bufferValid ? bufferToWrite.Memory.Size : 0;
		DMATransfer(fieldType, curVBLCount, buffer, inputBufferSize);

		if (bufferValid)
		{
			bufferToWrite.Info.Buffer.FieldType = (nosTextureFieldType)fieldType;
			SetPinValue(NOS_NAME_STATIC("Output"), Buffer::From(vkss::ConvertBufferInfo(bufferToWrite)));
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