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
		AddPinValueWatcher(NOS_NAME_STATIC("BufferToWrite"), [this](nos::Buffer const& newVal, std::optional<nos::Buffer> oldVal) {
			nosEngine.SetPinValue(PinName2Id[NOS_NAME_STATIC("Output")], newVal);
		});
	}

	nosResult ExecuteNode(const nosNodeExecuteArgs* args) override
	{
		NodeExecuteArgs execArgs = args;
		nosResourceShareInfo bufferToWrite = vkss:: ConvertToResourceInfo(*InterpretPinValue<sys::vulkan::Buffer>(*execArgs[NOS_NAME_STATIC("BufferToWrite")].Data));
		auto fieldType = *InterpretPinValue<sys::vulkan::FieldType>(*execArgs[NOS_NAME_STATIC("FieldType")].Data);
		ChannelInfo* channelInfo = InterpretPinValue<ChannelInfo>(*execArgs[NOS_NAME_STATIC("Channel")].Data);
		
		if (!channelInfo->device())
			return NOS_RESULT_FAILED;

		Device = AJADevice::GetDeviceBySerialNumber(channelInfo->device()->serial_number());
		if (!Device)
			return NOS_RESULT_FAILED;
		auto channelStr = channelInfo->channel_name();
		if (!channelStr)
			return NOS_RESULT_FAILED;
		ChannelName = channelStr->str();
		Channel = ParseChannel(ChannelName);
		Format = NTV2VideoFormat(channelInfo->video_format_idx());
		PixelFormat = channelInfo->frame_buffer_format();
		Mode = static_cast<AJADevice::Mode>(channelInfo->input_quad_link_mode());
		auto [_, bufferSize] = GetDMAInfo();

		if (!bufferToWrite.Memory.Handle || bufferToWrite.Info.Buffer.Size != bufferSize || Format == NTV2_FORMAT_UNKNOWN)
			return NOS_RESULT_FAILED;

		u8* buffer = nosVulkan->Map(&bufferToWrite);
		auto inputBufferSize = bufferToWrite.Memory.Size;

		DMATransfer(fieldType, buffer, inputBufferSize);

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