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
	DMAReadNodeContext(const nosFbNode* node) : DMANodeBase(node, DMA_READ)
	{
	}

	nosResult ExecuteNode(const nosNodeExecuteArgs* args) override
	{
		NodeExecuteArgs execArgs = args;
		nosResourceShareInfo bufferToWrite = vkss:: ConvertToResourceInfo(*InterpretPinValue<sys::vulkan::Buffer>(*execArgs[NOS_NAME_STATIC("BufferToWrite")].Data));
		auto fieldType = *InterpretPinValue<sys::vulkan::FieldType>(*execArgs[NOS_NAME_STATIC("FieldType")].Data);
		ChannelInfo* channelInfo = InterpretPinValue<ChannelInfo>(*execArgs[NOS_NAME_STATIC("Channel")].Data);
		uint32_t curVBLCount = *InterpretPinValue<uint32_t>(*execArgs[NOS_NAME_STATIC("CurrentVBL")].Data);

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

		u8* buffer = nosVulkan->Map(&bufferToWrite);
		auto inputBufferSize = bufferToWrite.Memory.Size;

		if (curVBLCount == 0)
			Device->GetInputVerticalInterruptCount(curVBLCount, Channel);

		DMATransfer(fieldType, curVBLCount, buffer, inputBufferSize);

		bufferToWrite.Info.Buffer.FieldType = (nosTextureFieldType)fieldType;

		nosEngine.SetPinValue(execArgs[NOS_NAME_STATIC("Output")].Id, Buffer::From(vkss::ConvertBufferInfo(bufferToWrite)));

		return NOS_RESULT_SUCCESS;
	}
};

nosResult RegisterDMAReadNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMARead"), DMAReadNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}