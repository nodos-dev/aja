#include <Nodos/PluginHelpers.hpp>

#include "AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"

namespace nos::aja
{

struct WaitVBLNodeContext : NodeContext
{
	WaitVBLNodeContext(const nosFbNode* node) : NodeContext(node)
	{
	}

	bool Interlaced() const
	{
		return false;
	}

	nosResult ExecuteNode(const nosNodeExecuteArgs* args) override
	{
		ChannelInfo* channelInfo = nullptr;
		nosUUID const* outId = nullptr;
		nosUUID inId;
		nos::sys::vulkan::FieldType waitField;
		nosUUID outFieldPinId;
		for (size_t i = 0; i < args->PinCount; ++i)
		{
			auto& pin = args->Pins[i];
			if (pin.Name == NOS_NAME_STATIC("Channel"))
				channelInfo = InterpretPinValue<ChannelInfo>(pin.Data->Data);
			if (pin.Name == NOS_NAME_STATIC("VBL"))
				outId = &pin.Id;
			if (pin.Name == NOS_NAME_STATIC("Run"))
				inId = pin.Id;
			if (pin.Name == NOS_NAME("WaitField"))
				waitField = *static_cast<nos::sys::vulkan::FieldType*>(pin.Data->Data);
			if (pin.Name == NOS_NAME("FieldType"))
				outFieldPinId = pin.Id;
		}
		if (!channelInfo->device())
			return NOS_RESULT_FAILED;
		auto device = AJADevice::GetDeviceBySerialNumber(channelInfo->device()->serial_number());
		if (!device)
			return NOS_RESULT_FAILED;
		auto channelStr = channelInfo->channel_name();
		if (!channelStr)
			return NOS_RESULT_FAILED;
		auto channel = ParseChannel(channelStr->string_view());
		auto videoFormat = static_cast<NTV2VideoFormat>(channelInfo->video_format_idx());
		if (IsProgressivePicture(videoFormat))
		{
			if (channelInfo->is_input())
				device->WaitForInputVerticalInterrupt(channel);
			else
				device->WaitForOutputVerticalInterrupt(channel);
			nosEngine.SetPinValue(outFieldPinId, nos::Buffer::From(sys::vulkan::FieldType::PROGRESSIVE));
		}
		else
		{
			if (waitField == sys::vulkan::FieldType::UNKNOWN || waitField == sys::vulkan::FieldType::PROGRESSIVE)
			{
				InterlacedWaitField = InterlacedWaitField == sys::vulkan::FieldType::EVEN ? sys::vulkan::FieldType::ODD :
					sys::vulkan::FieldType::EVEN;
			}
			else
			{
				InterlacedWaitField = waitField;
			}
			if (channelInfo->is_input())
				device->WaitForInputFieldID(GetFieldId(InterlacedWaitField), channel);
			else
				device->WaitForOutputFieldID(GetFieldId(InterlacedWaitField), channel);
			nosEngine.SetPinValue(outFieldPinId, nos::Buffer::From(InterlacedWaitField));
		}
		
		nosEngine.SetPinDirty(*outId); // This is unnecessary for now, but when we remove automatically setting outputs dirty on execute, this will be required.
		return NOS_RESULT_SUCCESS;
	}

	static NTV2FieldID GetFieldId(sys::vulkan::FieldType type)
	{
		return type == sys::vulkan::FieldType::EVEN ? NTV2_FIELD1 : 
			(type == sys::vulkan::FieldType::ODD ? NTV2_FIELD0 : NTV2_FIELD_INVALID);
	}
	
	sys::vulkan::FieldType InterlacedWaitField = sys::vulkan::FieldType::EVEN;
};

nosResult RegisterWaitVBLNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.WaitVBL"), WaitVBLNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}