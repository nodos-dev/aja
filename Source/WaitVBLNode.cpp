// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include <Nodos/PluginHelpers.hpp>

#include "AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"

namespace nos::aja
{

NOS_REGISTER_NAME(VBLFailed)

struct WaitVBLNodeContext : NodeContext
{
	WaitVBLNodeContext(const nosFbNode* node) : NodeContext(node)
	{
	}

	bool WaitVBL(AJADevice* device, NTV2Channel channel, bool isInput, bool isInterlaced, sys::vulkan::FieldType waitField)
	{
		if (isInterlaced)
		{
			if (waitField == sys::vulkan::FieldType::UNKNOWN || waitField == sys::vulkan::FieldType::PROGRESSIVE)
				InterlacedWaitField = InterlacedWaitField == sys::vulkan::FieldType::EVEN ? sys::vulkan::FieldType::ODD : sys::vulkan::FieldType::EVEN; // Progressive <-> interlaced, keep track of field type
			else
				InterlacedWaitField = waitField; // Use field type from pin
		}
		return device->WaitVBL(channel, isInput, isInterlaced ? GetFieldId(InterlacedWaitField) : NTV2_FIELD_INVALID);
	}

	nosResult ExecuteNode(nosNodeExecuteParams* execParams) override
	{
		NodeExecuteParams params = execParams;
		ChannelInfo* channelInfo = InterpretPinValue<ChannelInfo>(params[NOS_NAME_STATIC("Channel")].Data->Data);
		nosUUID const* outId = &params[NOS_NAME_STATIC("VBL")].Id;
		nosUUID const* outVBLCountId = &params[NOS_NAME_STATIC("CurrentVBL")].Id;
		nos::sys::vulkan::FieldType waitField = *InterpretPinValue<nos::sys::vulkan::FieldType>(params[NOS_NAME("WaitField")].Data->Data);
		nosUUID outFieldPinId = params[NOS_NAME("FieldType")].Id;
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
		bool isInterlaced = !IsProgressivePicture(videoFormat);
		bool vblSuccess = false;
		for (int i = 0; i < (VBLState.LastVBLCount == 0 ? 2 : 1); ++i) // Wait one more VBL after restart so that we don't start DMA in the middle of a frame.
			vblSuccess = WaitVBL(device.get(), channel, channelInfo->is_input(), isInterlaced, waitField);
		nosEngine.SetPinValue(outFieldPinId, nos::Buffer::From(isInterlaced ? InterlacedWaitField : sys::vulkan::FieldType::PROGRESSIVE));
		ULWord curVBLCount = 0;
		if (channelInfo->is_input())
			device->GetInputVerticalInterruptCount(curVBLCount, channel);
		else
			device->GetOutputVerticalInterruptCount(curVBLCount, channel);
		if (!vblSuccess)
		{
			nosEngine.CallNodeFunction(NodeId, NSN_VBLFailed);
			return NOS_RESULT_FAILED;
		}

		if (channelInfo->is_input() && !VBLState.LastVBLCount)
		{
			uint64_t nanoseconds = device->GetLastInputVerticalInterruptTimestamp(channel);
			nosPathCommand firstVblAfterStart{ .Event = NOS_FIRST_VBL_AFTER_START, .VBLTimestampNs = nanoseconds };
			nosEngine.SendPathCommand(*outId, firstVblAfterStart);
		}
		ChannelStr = channelInfo->channel_name()->c_str();
		IsInput = channelInfo->is_input();

		if (VBLState.LastVBLCount)
		{
			int64_t vblDiff = (int64_t)curVBLCount - (int64_t)(VBLState.LastVBLCount + 1 + isInterlaced);
			if (vblDiff > 0)
			{
				FrameDropped(vblDiff, true);
			} 
			else
			{
				if (VBLState.Dropped)
				{
					if (VBLState.FramesSinceLastDrop++ > 50)
					{
						VBLState.Dropped = false;
						VBLState.FramesSinceLastDrop = 0;
						nosEngine.SendPathRestart(*outId);
					}
				}
			}
		}
		VBLState.LastVBLCount = curVBLCount;
		
		nosEngine.SetPinDirty(*outId); // This is unnecessary for now, but when we remove automatically setting outputs dirty on execute, this will be required.
		nosEngine.SetPinValue(*outVBLCountId, nos::Buffer::From(curVBLCount));
		return NOS_RESULT_SUCCESS;
	}

	static NTV2FieldID GetFieldId(sys::vulkan::FieldType type)
	{
		return type == sys::vulkan::FieldType::EVEN ? NTV2_FIELD1 : 
			(type == sys::vulkan::FieldType::ODD ? NTV2_FIELD0 : NTV2_FIELD_INVALID);
	}
	
	sys::vulkan::FieldType InterlacedWaitField = sys::vulkan::FieldType::EVEN;
	struct {
		ULWord LastVBLCount = 0;
		bool Dropped = false;
		int FramesSinceLastDrop = 0;
	} VBLState;

	void OnPathStart() override
	{
		VBLState = {};
	}

	void FrameDropped(uint32_t dropCount, bool vblMissed)
	{
		VBLState.Dropped = true;
		VBLState.FramesSinceLastDrop = 0;
		nosEngine.LogW("%s: %s dropped %lld frames (%s missed)", IsInput ? "In" : "Out", ChannelStr.c_str(), dropCount, vblMissed ? "VBL" : "DMA");
	}

	static nosResult GetFunctions(size_t* outCount, nosName* outFunctionNames, nosPfnNodeFunctionExecute* outFunction) 
	{
		*outCount = 1;
		if (!outFunctionNames || !outFunction)
			return NOS_RESULT_SUCCESS;

		outFunctionNames[0] = NOS_NAME("Drop");
		outFunction[0] = [](void* ctx, nosFunctionExecuteParams* params)
		{
			WaitVBLNodeContext* context = reinterpret_cast<WaitVBLNodeContext*>(ctx);
			context->FrameDropped(1, false);
			return NOS_RESULT_SUCCESS;
		};

		return NOS_RESULT_SUCCESS; 
	}

	std::string ChannelStr;
	bool IsInput = false;
};

nosResult RegisterWaitVBLNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.WaitVBL"), WaitVBLNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}