// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include <Nodos/Plugin.hpp>

// External
#include <nosVulkanSubsystem/nosVulkanSubsystem.h>
#include <nosVulkanSubsystem/Helpers.hpp>
#include <Nodos/Utils/Stopwatch.hpp>

#include "AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"
#include "DMANodeBase.hpp"

namespace nos::aja
{
struct DMAReadNodeContext : DMANodeBase
{
	DMAReadNodeContext() : DMANodeBase(DMA_READ)
	{
	}

	nosResult ExecuteNode(NodeExecuteParams const& params) override
	{
		TypedObjectRef dstBufferObject = params.GetPinObject<vkss::Buffer>(NOS_NAME("BufferToWrite"));
		auto fieldType = *params.GetPinData<sys::vulkan::FieldType>(NOS_NAME("FieldType"));
		const ChannelInfo* channelInfo = params.GetPinData<ChannelInfo>(NOS_NAME("Channel"));
		uint32_t curVBLCount = *params.GetPinData<uint32_t>(NOS_NAME("CurrentVBL"));

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

		if (!dstBufferObject.IsValid())
		{
			nosEngine.LogE("DMA read target buffer is not valid.");
			return NOS_RESULT_FAILED;
		}
		const auto& dstBufferInfo = *vkss::GetResourceInfo(dstBufferObject);
		if (dstBufferInfo.Size != bufferSize || Format == NTV2_FORMAT_UNKNOWN)
		{
			nosEngine.LogE("DMA read target buffer size or format is not valid.");
			return NOS_RESULT_FAILED;
		}

		uint8_t* buffer = nosVulkan->Map(dstBufferObject);
		auto inputBufferSize = dstBufferInfo.Size;

		if (curVBLCount == 0)
			Device->GetInputVerticalInterruptCount(curVBLCount, Channel);

		DMATransfer(fieldType, curVBLCount, buffer, inputBufferSize);

		nosVulkan->SetResourceFieldType(dstBufferObject, (nosTextureFieldType)fieldType);

		SetPinObject(NOS_NAME("Output"), dstBufferObject);

		return NOS_RESULT_SUCCESS;
	}
};

nosResult RegisterDMAReadNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMARead"), DMAReadNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}