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
		for (size_t i = 0; i < args->PinCount; ++i)
		{
			auto& pin = args->Pins[i];
			if (pin.Name == NOS_NAME_STATIC("Channel"))
				channelInfo = InterpretPinValue<ChannelInfo>(pin.Data->Data);
			if (pin.Name == NOS_NAME_STATIC("VBL"))
				outId = &pin.Id;
			if (pin.Name == NOS_NAME_STATIC("Run"))
				inId = pin.Id;
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
		if (channelInfo->is_input())
			device->WaitForInputVerticalInterrupt(channel);
		else
			device->WaitForOutputVerticalInterrupt(channel);
		
		nosEngine.SetPinDirty(*outId); // This is unnecessary for now, but when we remove automatically setting outputs dirty on execute, this will be required.
		return NOS_RESULT_SUCCESS;
	}
};

nosResult RegisterWaitVBLNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.WaitVBL"), WaitVBLNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}