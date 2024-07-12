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

	nosResult ExecuteNode(const nosNodeExecuteArgs* execArgs) override
	{
		NodeExecuteArgs args = execArgs;
		ChannelInfo* channelInfo = InterpretPinValue<ChannelInfo>(args[NOS_NAME_STATIC("Channel")].Data->Data);
		nosUUID const* outId = &args[NOS_NAME_STATIC("VBL")].Id;
		nosUUID inId = args[NOS_NAME_STATIC("Run")].Id;
		nos::sys::vulkan::FieldType waitField = *InterpretPinValue<nos::sys::vulkan::FieldType>(args[NOS_NAME("WaitField")].Data->Data);
		nosUUID outFieldPinId = args[NOS_NAME("FieldType")].Id;
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
		if (!isInterlaced)
		{
			if (channelInfo->is_input())
				vblSuccess = device->WaitForInputVerticalInterrupt(channel);
			else
				vblSuccess = device->WaitForOutputVerticalInterrupt(channel);
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
				vblSuccess = device->WaitForInputFieldID(GetFieldId(InterlacedWaitField), channel);
			else
				vblSuccess = device->WaitForOutputFieldID(GetFieldId(InterlacedWaitField), channel);
			nosEngine.SetPinValue(outFieldPinId, nos::Buffer::From(InterlacedWaitField));
		}

		if (!vblSuccess)
		{
			nosEngine.CallNodeFunction(NodeId, NSN_VBLFailed);
			return NOS_RESULT_FAILED;
		}

		if (channelInfo->is_input() && !VBLState.LastVBLCount)
		{
			uint64_t nanoseconds = device->GetLastInputVerticalInterruptTimestamp(channel, true);
			nosPathCommand firstVblAfterStart{ .Event = NOS_FIRST_VBL_AFTER_START, .VBLNanoseconds = nanoseconds };
			nosEngine.SendPathCommand(*outId, firstVblAfterStart);
		}

		ULWord curVBLCount = 0;
		if (channelInfo->is_input())
			device->GetInputVerticalInterruptCount(curVBLCount, channel);
		else
			device->GetOutputVerticalInterruptCount(curVBLCount, channel);

		if (VBLState.LastVBLCount)
		{
			int64_t vblDiff = (int64_t)curVBLCount - (int64_t)(VBLState.LastVBLCount + 1 + isInterlaced);
			if (vblDiff > 0)
			{
				VBLState.Dropped = true;
				VBLState.FramesSinceLastDrop = 0;
				nosEngine.LogW("%s: %s dropped %lld frames", channelInfo->is_input() ? "In" : "Out", channelInfo->channel_name()->c_str(), vblDiff);
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

	static nosResult GetFunctions(size_t* outCount, nosName* outFunctionNames, nosPfnNodeFunctionExecute* outFunction) 
	{
		*outCount = 1;
		if (!outFunctionNames || !outFunction)
			return NOS_RESULT_SUCCESS;

		outFunctionNames[0] = NSN_VBLFailed;
		*outFunction = [](void* ctx, const nosNodeExecuteArgs* nodeArgs, const nosNodeExecuteArgs* functionArgs)
		{
			NodeExecuteArgs funcArgs(functionArgs);
			nosEngine.SetPinDirty(funcArgs[NOS_NAME("OutTrigger")].Id);
		};

		return NOS_RESULT_SUCCESS; 
	}

};

nosResult RegisterWaitVBLNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.WaitVBL"), WaitVBLNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}