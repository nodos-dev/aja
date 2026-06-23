// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include <Nodos/Plugin.hpp>

#include <optional>

// External
#include <nosSysVulkan/nosVulkanSubsystem.h>
#include <nosSysVulkan/Helpers.hpp>
#include <Nodos/Utils/Stopwatch.hpp>

#include <nosAudio/Audio_generated.h>
#include "nosAja/AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"
#include "DMANodeBase.hpp"

namespace nos::aja
{
#define NOS_AJA_AUDIO_OUTPUT_DIAGNOSTICS 0

struct DMAWriteNodeContext : DMANodeBase
{
	DMAWriteNodeContext()
		: DMANodeBase(DMA_WRITE)
		, LastWrittenAudioBufferOffset(0)
	{
	}

	nos::Buffer LastChannelInfo = {};
	// Cache last bool values so we don't restart the path when an upstream
	// node re-writes the same value every tick. Nosengine delivers every
	// pin write to OnPinValueChanged regardless of whether the bytes changed
	// (see the explicit memcmp dedup for Channel above).
	std::optional<bool> LastEnableANC;
	std::optional<bool> LastEnableTimecode;

	void OnPinValueChanged(nos::Name pinName, uuid const& pinId, nosBuffer value) override
	{
		if (pinName == NOS_NAME_STATIC("Channel"))
		{
			if (LastChannelInfo.Size() == value.Size && memcmp(LastChannelInfo.Data(), value.Data, value.Size) == 0)
				return;
			auto* channelInfo = InterpretObjectData<ChannelInfo>(value);
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
		else if (pinName == NOS_NAME_STATIC("EnableANC"))
		{
			if (value.Size < sizeof(bool))
				return;
			const bool v = *static_cast<const bool*>(value.Data);
			if (LastEnableANC && *LastEnableANC == v)
				return;
			LastEnableANC = v;
			nosEngine.SendPathRestart(NodeId);
		}
		else if (pinName == NOS_NAME_STATIC("EnableTimecode"))
		{
			if (value.Size < sizeof(bool))
				return;
			const bool v = *static_cast<const bool*>(value.Data);
			if (LastEnableTimecode && *LastEnableTimecode == v)
				return;
			LastEnableTimecode = v;
			nosEngine.SendPathRestart(NodeId);
		}
	}

	nosResult ExecuteNode(NodeExecuteParams const& params) override
	{
		TypedObjectRef inputBufferObject = params.GetPinObject<sys::vulkan::Buffer>(NOS_NAME("Input"));
		auto fieldType = *params.GetPinValue<sys::vulkan::FieldType>(NOS_NAME("FieldType"));
		uint32_t curVBLCount = *params.GetPinValue<uint32_t>(NOS_NAME("CurrentVBL"));
		bool enableANC = false;
		if (auto* p = params.GetPinValue<bool>(NOS_NAME("EnableANC")))
			enableANC = *p;
		const ANCFrame* ancIncoming = nullptr;
		if (enableANC)
			ancIncoming = params.GetPinValue<ANCFrame>(NOS_NAME("ANCFrame"));
		bool enableTimecode = false;
		if (auto* p = params.GetPinValue<bool>(NOS_NAME("EnableTimecode")))
			enableTimecode = *p;
		const Timecode* timecode = nullptr;
		if (enableTimecode)
			timecode = params.GetPinValue<Timecode>(NOS_NAME("Timecode"));

		if (!inputBufferObject.IsValid() || !Device || Format == NTV2_FORMAT_UNKNOWN)
			return NOS_RESULT_FAILED;

		CompositeObjectRef audioPacket = params.GetPinObject(NOS_NAME("AudioPacket"));
		auto descObject = audioPacket.GetField<TypedObjectRef<audio::AudioPacketDescriptor>>(NOS_NAME("desc"));
		auto audioBufferObject = audioPacket.GetField<TypedObjectRef<sys::vulkan::Buffer>>(NOS_NAME("buffer"));
		bool receivingAudio = false;
		const audio::AudioPacketDescriptor* audioPacketDesc = nullptr;
		if (audioPacket && descObject && audioBufferObject)
		{
			audioPacketDesc = descObject->InterpretBuffer();
			receivingAudio = audioPacketDesc->num_samples() > 0;
		}

		bool audioPlaying = false;
		auto audioSys = NTV2AudioSystem(Channel);
		Device->SetSDIOutputAudioSystem(Channel,audioSys);
		Device->IsAudioOutputRunning(audioSys, audioPlaying);
		if (!audioPlaying && receivingAudio)
		{
			Device->SetNumberAudioChannels(audioPacketDesc->channel_count(), audioSys);
			// Start writing audio data from 0.2 seconds ahead of the play head
			LastWrittenAudioBufferOffset = 48000 / 5 * sizeof(ULWord) * audioPacketDesc->channel_count();
			Device->StartAudioOutput(audioSys, false);
			Device->SetAudioOutputEraseMode(audioSys, true);
		}
		else if (receivingAudio)
		{
			ULWord audioChannelCount{};
			Device->GetNumberAudioChannels(audioChannelCount, audioSys);
			if (audioChannelCount != audioPacketDesc->channel_count())
				if (!Device->SetNumberAudioChannels(audioPacketDesc->channel_count(), audioSys))
					nosEngine.LogE("Failed to set audio channel count for output");
		}

		auto buffer = nosVulkan->Map(inputBufferObject);
		auto inputBufferInfo = *sys::vulkan::GetResourceInfo(inputBufferObject);
		auto inputSize = inputBufferInfo.Size;

		if (curVBLCount == 0)
			Device->GetOutputVerticalInterruptCount(curVBLCount, Channel);

		DMATransfer(fieldType, curVBLCount, buffer, inputSize);

		if (enableANC && ancIncoming)
			WriteAnc(ancIncoming);
		if (enableTimecode && timecode)
			WriteTimecode(*timecode);

		ULWord wrapAddress = 0;
		Device->GetAudioWrapAddress(wrapAddress, audioSys);
		const char* status = "Skipped";
		if (receivingAudio && audioPacketDesc->num_samples() > 0)
		{
			auto audioBuffer = nosVulkan->Map(*audioBufferObject);
			if (audioBuffer)
			{
				status = "Written";
				// audioBuffer contains 32-bit words with 24-bit samples in MSB
				ULWord byteCount = audioPacketDesc->num_samples() * audioPacketDesc->channel_count() *
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

#if NOS_AJA_AUDIO_OUTPUT_DIAGNOSTICS
		ULWord playheadPos{};
		Device->ReadAudioLastOut(playheadPos, audioSys);
		float playhead = 100.f * (float(playheadPos) / float(wrapAddress));
		float lastWritten = 100.f * (float(LastWrittenAudioBufferOffset) / float(wrapAddress));
		nosEngine.LogI("%s, Playhead: %.2f, LastWritten: %.2f", status, playhead, lastWritten);
#endif
		return NOS_RESULT_SUCCESS;
	}

	void OnPathStart() override
	{
		DMANodeBase::OnPathStart();
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
};

nosResult RegisterDMAWriteNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMAWrite"), DMAWriteNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}
}