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
		ChannelInfo* channelInfo = nullptr;
		nosResourceShareInfo outputBuffer{};
		const nosBuffer* outputPinData = nullptr;
		auto fieldType = nos::sys::vulkan::FieldType::UNKNOWN;
		for (size_t i = 0; i < args->PinCount; ++i)
		{
			auto& pin = args->Pins[i];
			if (pin.Name == NOS_NAME_STATIC("Channel"))
				channelInfo = InterpretPinValue<ChannelInfo>(*pin.Data);
			if (pin.Name == NOS_NAME_STATIC("Output"))
			{
				outputPinData = pin.Data;
				outputBuffer = vkss::ConvertToResourceInfo(*InterpretPinValue<sys::vulkan::Buffer>(*outputPinData));
			}
			if (pin.Name == NOS_NAME("FieldType"))
				fieldType = *InterpretPinValue<sys::vulkan::FieldType>(*pin.Data);
		}
		
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

		auto [_, bufferSize] = GetDMAInfo();

		constexpr nosMemoryFlags memoryFlags = nosMemoryFlags(NOS_MEMORY_FLAGS_HOST_VISIBLE);
		if (outputBuffer.Memory.Size != bufferSize || outputBuffer.Info.Buffer.MemoryFlags != memoryFlags)
		{
			nosResourceShareInfo bufInfo = {
				.Info = {
					.Type = NOS_RESOURCE_TYPE_BUFFER,
					.Buffer = nosBufferInfo{
						.Size = (uint32_t)bufferSize,
						.Usage = nosBufferUsage(NOS_BUFFER_USAGE_STORAGE_BUFFER | NOS_BUFFER_USAGE_TRANSFER_SRC),
						.MemoryFlags = memoryFlags,
					}}};
			auto bufferDesc = vkss::ConvertBufferInfo(bufInfo);
			nosEngine.SetPinValueByName(NodeId, NOS_NAME_STATIC("Output"), Buffer::From(bufferDesc));
			for (size_t i = 0; i < args->PinCount; ++i)
			{
				auto& pin = args->Pins[i];
				if (pin.Name == NOS_NAME_STATIC("Channel"))
					channelInfo = InterpretPinValue<ChannelInfo>(*pin.Data);
				if (pin.Name == NOS_NAME_STATIC("Output"))
					outputBuffer = vkss::ConvertToResourceInfo(*InterpretPinValue<sys::vulkan::Buffer>(*pin.Data));
			}
		}

		if(!outputBuffer.Memory.Handle)
			return NOS_RESULT_SUCCESS;

		u8* buffer = nosVulkan->Map(&outputBuffer);

		DMATransfer(fieldType, buffer);

		static_cast<nos::sys::vulkan::Buffer*>(outputPinData->Data)->mutate_field_type(fieldType);

		return NOS_RESULT_SUCCESS;
	}
};

nosResult RegisterDMAReadNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMARead"), DMAReadNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}