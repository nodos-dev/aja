// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include <Nodos/Plugin.hpp>

// External
#include <nosVulkanSubsystem/nosVulkanSubsystem.h>
#include <nosVulkanSubsystem/Helpers.hpp>
#include <Nodos/Utils/Stopwatch.hpp>

#include <nos.audio/Audio_generated.h>
#include "AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"
#include "DMANodeBase.hpp"

namespace nos::aja
{

struct DMAWriteNodeContext : DMANodeBase
{
	DMAWriteNodeContext() : DMANodeBase(DMA_WRITE)
	{
	}

	nos::Buffer LastChannelInfo = {};

	void GetScheduleInfo(nosScheduleInfo* out) override
	{
		*out = nosScheduleInfo{
			.Importance = 1,
			.DeltaSeconds = GetDeltaSeconds(Format, IsInterlaced()),
			.Type = NOS_SCHEDULE_TYPE_ON_DEMAND,
		};
	}
 
	void OnPinValueChanged(nos::Name pinName, uuid const& pinId, nosBuffer value) override
	{ 
		if (pinName == NOS_NAME_STATIC("Channel"))
		{
			if (LastChannelInfo.Size() == value.Size && memcmp(LastChannelInfo.Data(), value.Data, value.Size) == 0)
				return;
			auto* channelInfo = InterpretPinValue<ChannelInfo>(value);
			Device = nullptr;
			LastChannelInfo = {};
			if (!channelInfo || !channelInfo->device())
				return;
			Device = AJADevice::GetDeviceBySerialNumber(channelInfo->device()->serial_number());
			if (!Device || !channelInfo->channel_name())
				return;
			LastChannelInfo = value;
			ChannelName = channelInfo->channel_name()->c_str();
			Channel = ParseChannel(ChannelName);
			Format = NTV2VideoFormat(channelInfo->video_format_idx());
			PixelFormat = channelInfo->frame_buffer_format();
			if (channelInfo->is_quad())
				Mode = static_cast<AJADevice::Mode>(channelInfo->output_quad_link_mode());
			else
				Mode = AJADevice::SL;
			nosEngine.RecompilePath(NodeId);
		}
	}
	
	nosResult ExecuteNode(nosNodeExecuteParams* params) override
	{
		nosResourceShareInfo inputBuffer{}, audioPacket{};
		auto fieldType = nos::sys::vulkan::FieldType::UNKNOWN;
		uint32_t curVBLCount = 0;
		audio::AudioPacketDescriptor audioPacketDesc = {};
		for (size_t i = 0; i < params->PinCount; ++i)
		{
			auto& pin = *params->Pins[i];
			if (pin.Name == NOS_NAME_STATIC("Input"))
				inputBuffer = vkss::ConvertToResourceInfo(*InterpretPinValue<sys::vulkan::Buffer>(*pin.Data));
			if (pin.Name == NOS_NAME("FieldType"))
				fieldType = *InterpretPinValue<sys::vulkan::FieldType>(*pin.Data);
			if (pin.Name == NOS_NAME("CurrentVBL"))
				curVBLCount = *InterpretPinValue<uint32_t>(*pin.Data);
			if (pin.Name == NOS_NAME("AudioPacket"))
				audioPacket = vkss::ConvertToResourceInfo(*InterpretPinValue<sys::vulkan::Buffer>(*pin.Data));
			if (pin.Name == NOS_NAME("AudioPacketDescriptor"))
				audioPacketDesc = *InterpretPinValue<audio::AudioPacketDescriptor>(*pin.Data);
		}

		if (!inputBuffer.Memory.Handle || !Device || Format == NTV2_FORMAT_UNKNOWN)
			return NOS_RESULT_FAILED;

		bool audioPlaying = false;
		NTV2AudioSystem audioSys{};
		Device->GetSDIOutputAudioSystem(Channel, audioSys);
		Device->IsAudioOutputRunning(audioSys, audioPlaying);
		if (!audioPlaying)
		{
			Device->SetNumberAudioChannels(audioPacketDesc.channel_count(), audioSys);
			Device->StartAudioOutput(audioSys, true);
			Device->SetAudioOutputEraseMode(audioSys, true);
		}
		else
		{
			ULWord audioChannelCount{};
			Device->GetNumberAudioChannels(audioChannelCount, audioSys);
			if (audioChannelCount != audioPacketDesc.channel_count())
				if (!Device->SetNumberAudioChannels(audioPacketDesc.channel_count(), audioSys))
					nosEngine.LogE("Failed to set audio channel count for output");
		}

		auto buffer = nosVulkan->Map(&inputBuffer);
		auto inputSize = inputBuffer.Memory.Size;

		//nosVulkan->Begin("Flush before AJA DMA Write", &cmd);
		//nosCmdEndParams end{.ForceSubmit = NOS_TRUE, .OutGPUEventHandle = &event};
		//nosVulkan->End(cmd, &end);
		//nosVulkan->WaitGpuEvent(&event, UINT64_MAX);

		if (curVBLCount == 0)
			Device->GetOutputVerticalInterruptCount(curVBLCount, Channel);

		DMATransfer(fieldType, curVBLCount, buffer, inputSize);

		if (audioPacket.Memory.Handle && audioPacketDesc.num_samples() > 0)
		{
			auto audioBuffer = nosVulkan->Map(&audioPacket);
			if (audioBuffer)
			{
				ULWord wrapAddress = 0;
				Device->GetAudioWrapAddress(wrapAddress, audioSys);
				
				// audioBuffer contains 32-bit words with 24-bit samples in MSB
				ULWord byteCount = audioPacketDesc.num_samples() * audioPacketDesc.channel_count() *
								   sizeof(ULWord); // 4 bytes per sample
				if (LastWrittenAudioBufferOffset + byteCount > wrapAddress)
				{
					ULWord firstPartSize = wrapAddress - LastWrittenAudioBufferOffset;
					// Write audio data up to the wrap address
					Device->DMAWriteAudio(audioSys, (const ULWord*)audioBuffer, LastWrittenAudioBufferOffset, firstPartSize);
					// Write the remaining audio data from the start
					ULWord remainingSize = byteCount - firstPartSize;
					Device->DMAWriteAudio(
						audioSys, (const ULWord*)(audioBuffer + firstPartSize), 0, remainingSize);
					LastWrittenAudioBufferOffset = remainingSize;
				}
				else
				{
					// Write audio data ahead of the play head
					// Use the current play head position as the write offset
					Device->DMAWriteAudio(audioSys, (const ULWord*)audioBuffer, LastWrittenAudioBufferOffset, byteCount);
					LastWrittenAudioBufferOffset += byteCount;
				}
			}
		}

		nosScheduleNodeParams schedule {
			.NodeId = NodeId,
			.AddScheduleCount = 1
		};
		nosEngine.ScheduleNode(&schedule);
		
		return NOS_RESULT_SUCCESS;
	}

	void OnPathStart() override
	{
		DMANodeBase::OnPathStart();
		nosScheduleNodeParams schedule{.NodeId = NodeId, .AddScheduleCount = 1};
		nosEngine.ScheduleNode(&schedule);
		LastWrittenAudioBufferOffset = 0;
	}

	void OnPathStop() override
	{
		DMANodeBase::OnPathStop();
		if (!Device || Channel == NTV2_CHANNEL_INVALID)
			return;
		NTV2AudioSystem audioSys{};
		Device->GetSDIOutputAudioSystem(Channel, audioSys);
		Device->StopAudioOutput(audioSys);
	}

	ULWord LastWrittenAudioBufferOffset;
	ULWord LastAudioLastOut = 0;
};

nosResult RegisterDMAWriteNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMAWrite"), DMAWriteNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}