#include <Nodos/PluginHelpers.hpp>

// External
#include <nosVulkanSubsystem/nosVulkanSubsystem.h>
#include <nosVulkanSubsystem/Helpers.hpp>

#include "AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"

namespace nos::aja
{

struct DMAWriteNodeContext : NodeContext
{
	DMAWriteNodeContext(const nosFbNode* node) : NodeContext(node)
	{
	}

	uint8_t DoubleBufferIdx = 0;
	NTV2Channel Channel = NTV2_CHANNEL_INVALID;
	std::shared_ptr<AJADevice> Device = nullptr;
	NTV2VideoFormat Format = NTV2_FORMAT_UNKNOWN;

	bool Interlaced() const
	{
		return false; // TODO: Implement
	}

	uint32_t GetFrameBufferIndex(uint8_t doubleBufferIndex) const
	{
		return 2 * Channel + doubleBufferIndex;
	}

	void SetFrame(uint8_t curDoubleBufferIndex)
	{
		u32 idx = GetFrameBufferIndex(curDoubleBufferIndex);
		Device->SetOutputFrame(Channel, idx);
		if (NTV2_IS_QUAD_FRAME_FORMAT(Format)) // TODO: Get from channel info
			for (u32 i = Channel + 1; i < Channel + 4; ++i)
				Device->SetOutputFrame(NTV2Channel(i), idx);
	}

	uint32_t StartDoubleBuffer() 
	{
		SetFrame(uint8_t(!Interlaced()));
		return 0; 
	}

	uint8_t NextDoubleBuffer(uint8_t curDoubleBuffer)
	{
		if (Interlaced())
			return curDoubleBuffer;
		SetFrame(curDoubleBuffer);
		return curDoubleBuffer ^ 1;
	}
 
	void GetScheduleInfo(nosScheduleInfo* out) override
	{
		*out = nosScheduleInfo {
			.Importance = 1,
			.DeltaSeconds = GetDeltaSeconds(Format, Interlaced()),
			.Type = NOS_SCHEDULE_TYPE_ON_DEMAND,
		};
	}
	
	nosResult ExecuteNode(const nosNodeExecuteArgs* args) override
	{
		ChannelInfo* channelInfo = nullptr;
		nosResourceShareInfo inputBuffer{};
		for (size_t i = 0; i < args->PinCount; ++i)
		{
			auto& pin = args->Pins[i];
			if (pin.Name == NOS_NAME_STATIC("Channel"))
				channelInfo = InterpretPinValue<ChannelInfo>(*pin.Data);
			if (pin.Name == NOS_NAME_STATIC("Input"))
				inputBuffer = vkss::ConvertToResourceInfo(*InterpretPinValue<sys::vulkan::Buffer>(*pin.Data));
		}

		if (!inputBuffer.Memory.Handle)
			return NOS_RESULT_FAILED;

		Device = AJADevice::GetDeviceBySerialNumber(channelInfo->device()->serial_number());
		if (!Device)
			return NOS_RESULT_FAILED;
		auto channelStr = channelInfo->channel_name();
		if (!channelStr)
			return NOS_RESULT_FAILED;
		Channel = ParseChannel(channelStr->string_view());
		Format = NTV2VideoFormat(channelInfo->video_format_idx());

		nosCmd cmd;
		nosVulkan->Begin("Flush Before AJA DMA Write", &cmd);
		nosGPUEvent event;
		nosCmdEndParams end {.ForceSubmit = NOS_TRUE, .OutGPUEventHandle = &event};
		nosVulkan->End(cmd, &end);
		nosVulkan->WaitGpuEvent(&event, UINT64_MAX);
		
		auto buffer = nosVulkan->Map(&inputBuffer);

		auto frameBufferIndex = GetFrameBufferIndex(DoubleBufferIdx);
		Device->DMAWriteFrame(frameBufferIndex, reinterpret_cast<uint32_t*>(buffer), inputBuffer.Info.Buffer.Size, Channel);

		DoubleBufferIdx = NextDoubleBuffer(DoubleBufferIdx);

		nosScheduleNodeParams schedule {
			.NodeId = NodeId,
			.AddScheduleCount = 1
		};
		nosEngine.ScheduleNode(&schedule);
		
		return NOS_RESULT_SUCCESS;
	}

	void OnPathStart() override
	{
		nosScheduleNodeParams schedule{.NodeId = NodeId, .AddScheduleCount = 1};
		nosEngine.ScheduleNode(&schedule);
	}
};

nosResult RegisterDMAWriteNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMAWrite"), DMAWriteNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}