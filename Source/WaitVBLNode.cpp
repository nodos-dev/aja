// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include <Nodos/PluginHelpers.hpp>

#include "AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"

#include <nosSync/nosSync.h>

namespace nos::aja
{

NOS_REGISTER_NAME(VBLFailed)

nosResult WaitVBLEvent(void* ctx, uint64_t* outVblTimestampNs);

struct WaitVBLNodeContext : NodeContext
{
	WaitVBLNodeContext(nosFbNodePtr node) : NodeContext(node)
	{
	}

	nosResult WaitVBL(uint64_t* outVblTimestampNs)
	{
		if (auto device = GetDevice())
		{
			auto channel = GetChannel();
			WaitVBL(device.get(),
				channel,
				ChannelInfo.is_input,
				ChannelInfo.is_interlaced,
				VBLState.InterlacedWaitField);
			*outVblTimestampNs = device->GetLastVBLTimestamp(channel, ChannelInfo.is_input);
		}
		else
		{
			nosEngine.LogE("Tried to wait when channel not configured!");
			nosEngine.SendPathRestart(NodeId);
		}
		return NOS_RESULT_SUCCESS;
	}

	bool WaitVBL(AJADevice* device, NTV2Channel channel, bool isInput, bool isInterlaced, sys::vulkan::FieldType waitField)
	{
		if (isInterlaced)
		{
			if (waitField == sys::vulkan::FieldType::UNKNOWN || waitField == sys::vulkan::FieldType::PROGRESSIVE)
				VBLState.InterlacedWaitField =
					VBLState.InterlacedWaitField == sys::vulkan::FieldType::EVEN
						? sys::vulkan::FieldType::ODD
						: sys::vulkan::FieldType::EVEN; // Progressive <-> interlaced, keep track of field type
			else
				VBLState.InterlacedWaitField = waitField; // Use field type from pin
		}
		return device->WaitVBL(
			channel, isInput, isInterlaced ? GetFieldId(VBLState.InterlacedWaitField) : NTV2_FIELD_INVALID);
	}

	std::shared_ptr<AJADevice> GetDevice() const
	{
		if (!ChannelInfo.device)
			return nullptr;
		return AJADevice::GetDeviceBySerialNumber(ChannelInfo.device->serial_number);
	}

	NTV2Channel GetChannel()
	{ 
		if (ChannelInfo.channel_name.empty())
			return NTV2_CHANNEL_INVALID;
		return ParseChannel(ChannelInfo.channel_name);
	}

	ULWord GetVBLCount(AJADevice& device, NTV2Channel channel)
	{
		ULWord curVBLCount = 0;
		if (ChannelInfo.is_input)
			device.GetInputVerticalInterruptCount(curVBLCount, channel);
		else
			device.GetOutputVerticalInterruptCount(curVBLCount, channel);
		return curVBLCount;
	}

	nosResult ExecuteNode(nosNodeExecuteParams* execParams) override
	{
		NodeExecuteParams params = execParams;
		InterpretPinValue<aja::ChannelInfo>(params[NOS_NAME_STATIC("Channel")].Data->Data)->UnPackTo(&ChannelInfo);
		uuid const& outId = params[NOS_NAME_STATIC("VBL")].Id;
		uuid const& outVBLCountId = params[NOS_NAME_STATIC("CurrentVBL")].Id;
		nos::sys::vulkan::FieldType waitField = *InterpretPinValue<nos::sys::vulkan::FieldType>(params[NOS_NAME("WaitField")].Data->Data);
		uuid outFieldPinId = params[NOS_NAME("FieldType")].Id;
		auto device = GetDevice();
		if (!device)
			return NOS_RESULT_FAILED;
		auto channelStr = ChannelInfo.channel_name;
		if (channelStr.empty())
			return NOS_RESULT_FAILED;
		auto channel = ParseChannel(ChannelInfo.channel_name);

		auto videoFormat = static_cast<NTV2VideoFormat>(ChannelInfo.video_format_idx);
		bool isInterlaced = !IsProgressivePicture(videoFormat);
		bool vblSuccess = false;
		{
			ScopedProfilerEvent _(ChannelInfo.channel_name + " Wait VBL");
			vblSuccess = WaitVBL(device.get(), channel, ChannelInfo.is_input, isInterlaced, waitField);
#if NOS_AJA_DIAGNOSTICS
			uint64_t timepoint = 0;
			if (ChannelInfo.is_input)
			{
				timepoint = device->GetLastInputVerticalInterruptTimestamp(channel);
			}
			else
			{
				timepoint = device->GetLastOutputVerticalInterruptTimestamp(channel);
			}
			if (!VBLState.SysClockTimeDiff)
				VBLState.SysClockTimeDiff = std::chrono::duration_cast<std::chrono::nanoseconds>(
							   std::chrono::system_clock::now().time_since_epoch())
							   .count() -
						   timepoint;
			std::chrono::system_clock::duration time = std::chrono::duration_cast<std::chrono::system_clock::duration>(
				std::chrono::nanoseconds(timepoint + VBLState.SysClockTimeDiff));
			std::chrono::system_clock::duration startTime =
				std::chrono::duration_cast<std::chrono::system_clock::duration>(
					std::chrono::nanoseconds(VBLState.FirstVBLTimestamp + VBLState.SysClockTimeDiff));

			nosEngine.LogI("%s: %s VBL %lld at %s (Start: %s)",
						   ChannelInfo.is_input ? "In " : "Out",
						   channelStr.c_str(),
						   VBLState.FrameCountSincePathStart,
						   std::format("{:%H:%M:%S}", time).c_str(),
						   std::format("{:%H:%M:%S}", startTime).c_str());
		VBLState.FrameCountSincePathStart++;
#endif
		}
		nosEngine.SetPinValue(outFieldPinId, nos::Buffer::From(isInterlaced ? VBLState.InterlacedWaitField : sys::vulkan::FieldType::PROGRESSIVE));
		ULWord curVBLCount = GetVBLCount(*device, channel);
		if (!vblSuccess)
		{
			nosEngine.TriggerNodeEvent(NodeId, NSN_VBLFailed);
			return NOS_RESULT_FAILED;
		}

		if (ChannelInfo.is_input && VBLState.FirstVBL)
		{
			uint64_t nanoseconds = device->GetLastInputVerticalInterruptTimestamp(channel);
			nosPathCommand firstVblAfterStart{ .Event = NOS_FIRST_VBL_AFTER_START, .VBLTimestampNs = nanoseconds };
			nosEngine.SendPathCommand(outId, firstVblAfterStart);
		}

		if (VBLState.LastVBLCount)
		{
			int64_t vblDiff = (int64_t)curVBLCount - (int64_t)(VBLState.LastVBLCount + 1 + isInterlaced);
			if (vblDiff > 0)
			{
				assert(vblDiff <= UINT32_MAX);
				FrameDropped(static_cast<uint32_t>(vblDiff), true);
			} 
			else
			{
				if (VBLState.Dropped)
				{
					if (VBLState.FramesSinceLastDrop++ > 50)
					{
						VBLState.Dropped = false;
						VBLState.FramesSinceLastDrop = 0;
						nosEngine.SendPathRestart(outId);
					}
				}
			}
		}
		VBLState.FirstVBL = false;
		VBLState.LastVBLCount = curVBLCount;
		
		nosEngine.SetPinDirty(outId); // This is unnecessary for now, but when we remove automatically setting outputs dirty on execute, this will be required.
		nosEngine.SetPinValue(outVBLCountId, nos::Buffer::From(curVBLCount));
		return NOS_RESULT_SUCCESS;
	}

	static NTV2FieldID GetFieldId(sys::vulkan::FieldType type)
	{
		return type == sys::vulkan::FieldType::EVEN ? NTV2_FIELD1 : 
			(type == sys::vulkan::FieldType::ODD ? NTV2_FIELD0 : NTV2_FIELD_INVALID);
	}
	
	struct {
		bool FirstVBL = true;
		ULWord LastVBLCount = 0;
		bool Dropped = false;
		int FramesSinceLastDrop = 0;
		sys::vulkan::FieldType InterlacedWaitField = sys::vulkan::FieldType::EVEN; // Field flipped first, so start with even
#if NOS_AJA_DIAGNOSTICS
		uint64_t FrameCountSincePathStart = 0;
		uint64_t FirstVBLTimestamp = 0;
		uint64_t SysClockTimeDiff = 0;
#endif
	} VBLState;

	void OnPathStartInitiated() override
	{
		VBLState = {};
		// TODO: Pass path ID.
		nosSync->RegisterEventWaiter(0, this, WaitVBLEvent, &WaitId);
	}

	void OnPathStart() override
	{
		nosSync->WaitForConsensus(0, 1000000);
		if (auto device = GetDevice())
		{
			auto channel = GetChannel();
			VBLState.LastVBLCount = GetVBLCount(*device, channel);
#if NOS_AJA_DIAGNOSTICS
			VBLState.FirstVBLTimestamp = device->GetLastVBLTimestamp(channel, ChannelInfo.is_input);
#endif
		}
	}

	void OnPathStop() override
	{
		nosSync->UnregisterEventWaiter(WaitId);
	}

	void FrameDropped(uint32_t dropCount, bool vblMissed)
	{
		VBLState.Dropped = true;
		VBLState.FramesSinceLastDrop = 0;
		nosEngine.LogW("%s: %s dropped %lld frames (%s missed)", ChannelInfo.is_input ? "In" : "Out", ChannelInfo.channel_name.c_str(), dropCount, vblMissed ? "VBL" : "DMA");
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

	TChannelInfo ChannelInfo{};
	uint64_t WaitId = 0;
};

nosResult WaitVBLEvent(void* ctx, uint64_t* outVblTimestampNs)
{
	return (static_cast<struct WaitVBLNodeContext*>(ctx))->WaitVBL(outVblTimestampNs);
}

nosResult RegisterWaitVBLNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.WaitVBL"), WaitVBLNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}