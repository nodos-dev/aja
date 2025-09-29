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
struct DMAReadNodeContext : DMANodeBase
{
	DMAReadNodeContext() : DMANodeBase(DMA_READ)
	{
	}

	void OnPathStop() override
	{
		DMANodeBase::OnPathStop();
		if (!Device || Channel == NTV2_CHANNEL_INVALID)
			return;
		Device->StopAudioInput(AudioSystem);
		Device->SetAudioCaptureEnable(AudioSystem, false);
	}

	nosResult ReadAudio(TypedObjectRef<sys::vulkan::Buffer> bufToWrite, audio::AudioPacketDescriptor& outAudioPacket)
	{
		outAudioPacket.mutate_bit_depth(audio::BitDepth::AUDIO_BIT_DEPTH_24_BIT);
		outAudioPacket.mutate_sample_rate(48000);
		if (!bufToWrite)
			return NOS_RESULT_FAILED;
		uint8_t* audioBuffer = nosVulkan->Map(bufToWrite);
		if (!audioBuffer)
			return NOS_RESULT_FAILED;
		bool audioRunning = false;
		Device->IsAudioInputRunning(AudioSystem, audioRunning);
		ULWord audioChannelCount = 0;
		Device->GetNumberAudioChannels(audioChannelCount, AudioSystem);
		if (!audioRunning)
		{
			AudioSystem = NTV2AudioSystem(Channel);
			Device->SetAudioCaptureEnable(AudioSystem, true);
			Device->StartAudioInput(AudioSystem, false);
			auto inSource = GetNTV2InputSourceForIndex(Channel, NTV2_INPUTSOURCES_SDI);
			auto audioSys = NTV2InputSourceToAudioSystem(inSource);
			Device->GetAudioReadOffset(AudioReadOffset, AudioSystem);
			NTV2AudioSource audioSource{};
			NTV2EmbeddedAudioInput embeddedIn{};
			NTV2AudioChannelPairs audioChannelPairs{};
			Device->GetDetectedAudioChannelPairs(AudioSystem, audioChannelPairs);
			Device->GetAudioSystemInputSource(AudioSystem, audioSource, embeddedIn);
			Device->GetAudioWrapAddress(AudioWrapAddress, AudioSystem);
			// AudioWrapAddress = AudioWrapAddress + AudioReadOffset;
			AudioInLastAddress = AudioReadOffset;
		}
		outAudioPacket.mutate_channel_count(audioChannelCount);
		outAudioPacket.mutate_sample_stride(sizeof(ULWord));
		uint32_t currentAudioInAddress = 0;
		Device->ReadAudioLastIn(currentAudioInAddress, AudioSystem);

		currentAudioInAddress =
			currentAudioInAddress -
			(currentAudioInAddress % (sizeof(ULWord) * audioChannelCount)); //	Force sample alignment
		currentAudioInAddress += AudioReadOffset;
		uint32_t audioBytesCaptured{};
		auto oldAudioInLastAddress = AudioInLastAddress;
		AudioInLastAddress = currentAudioInAddress;
		auto dstBufInfo = *sys::vulkan::GetResourceInfo(bufToWrite);
		if (currentAudioInAddress < oldAudioInLastAddress)
		{
			audioBytesCaptured = (AudioWrapAddress + AudioReadOffset) - oldAudioInLastAddress;

			if (audioBytesCaptured % (sizeof(ULWord) * audioChannelCount) != 0)
			{
				nosEngine.LogE("Audio read size is not DWORD aligned.");
			}

			if (audioBytesCaptured > dstBufInfo.Size)
			{
				nosEngine.LogE("Audio read size exceeds buffer size.");
			}
			Device->DMAReadAudio(
				AudioSystem, reinterpret_cast<uint32_t*>(audioBuffer), oldAudioInLastAddress, audioBytesCaptured);

			auto audioBytesRemaining = currentAudioInAddress - AudioReadOffset;
			audioBytesRemaining =
				audioBytesRemaining -
				(audioBytesRemaining % (sizeof(ULWord) * audioChannelCount)); //	Force sample alignment
			if (audioBytesRemaining > dstBufInfo.Size)
			{
				nosEngine.LogE("Audio read size exceeds buffer size.");
				return NOS_RESULT_FAILED;
			}
			Device->DMAReadAudio(AudioSystem,
								 reinterpret_cast<uint32_t*>(audioBuffer + audioBytesCaptured),
								 AudioReadOffset,
								 audioBytesRemaining);

			audioBytesCaptured += audioBytesRemaining;
		}
		else
		{
			audioBytesCaptured = currentAudioInAddress - oldAudioInLastAddress;
			if (audioBytesCaptured > dstBufInfo.Size)
			{
				nosEngine.LogE("Audio read size exceeds buffer size.");
				return NOS_RESULT_FAILED;
			}
			if (audioBytesCaptured > 0)
			{
				Device->DMAReadAudio(
					AudioSystem, reinterpret_cast<ULWord*>(audioBuffer), oldAudioInLastAddress, audioBytesCaptured);
			}
		}
		if (audioBytesCaptured % (sizeof(ULWord) * audioChannelCount) != 0)
		{
			nosEngine.LogE("Audio read size is not DWORD aligned.");
		}
		outAudioPacket.mutate_num_samples(audioBytesCaptured / sizeof(ULWord) / audioChannelCount);
		return NOS_RESULT_SUCCESS;
	}

	nosResult ExecuteNode(NodeExecuteParams const& params) override
	{
		TypedObjectRef dstBufferObject = params.GetPinObject<sys::vulkan::Buffer>(NOS_NAME("BufferToWrite"));
		auto fieldType = *params.GetPinData<sys::vulkan::FieldType>(NOS_NAME("FieldType"));
		const ChannelInfo* channelInfo = params.GetPinData<ChannelInfo>(NOS_NAME("Channel"));
		uint32_t curVBLCount = *params.GetPinData<uint32_t>(NOS_NAME("CurrentVBL"));

		if (!channelInfo->device())
			return NOS_RESULT_FAILED;

		Device = AJADevice::GetDeviceBySerialNumber(channelInfo->device()->serial_number());
		if (!Device) {
			nosEngine.LogE("Device not found!");
			return NOS_RESULT_FAILED;
		}
		auto channelStr = channelInfo->channel_name();
		if (!channelStr)
			return NOS_RESULT_FAILED;
		ChannelName = channelStr->str();
		Channel = ParseChannel(ChannelName);
		Format = NTV2VideoFormat(channelInfo->video_format_idx());
		PixelFormat = channelInfo->frame_buffer_format();
		if (channelInfo->is_quad())
			Mode = static_cast<AJADevice::Mode>(channelInfo->input_quad_link_mode());
		else 
			Mode = AJADevice::SL;
		auto [_, bufferSize] = GetDMAInfo();

		if (!dstBufferObject.IsValid())
		{
			nosEngine.LogE("DMA read target buffer is not valid.");
			return NOS_RESULT_FAILED;
		}
		const auto& dstBufferInfo = *sys::vulkan::GetResourceInfo(dstBufferObject);
		if (dstBufferInfo.Size != bufferSize || Format == NTV2_FORMAT_UNKNOWN)
		{
			nosEngine.LogE("DMA read target buffer size or format is not valid.");
			return NOS_RESULT_FAILED;
		}

		uint8_t* buffer = nosVulkan->Map(dstBufferObject);
		auto inputBufferSize = dstBufferInfo.Size;

		auto audioBufferToWrite = params.GetPinObject<sys::vulkan::Buffer>(NOS_NAME("AudioBufferToWrite"));
		audio::AudioPacketDescriptor audioPacketDesc{};
		ReadAudio(audioBufferToWrite, audioPacketDesc);

		if (curVBLCount == 0)
			Device->GetInputVerticalInterruptCount(curVBLCount, Channel);

		DMATransfer(fieldType, curVBLCount, buffer, inputBufferSize);

		nosVulkan->SetResourceFieldType(dstBufferObject, (nosTextureFieldType)fieldType);

		SetPinObject(NOS_NAME("Output"), dstBufferObject);
		SetPinObject(NOS_NAME_STATIC("AudioOutput"), audioBufferToWrite);
		SetPinValue(NOS_NAME_STATIC("AudioPacketDescriptor"), audioPacketDesc);

		return NOS_RESULT_SUCCESS;
	}

	NTV2AudioSystem AudioSystem{};
	uint32_t AudioReadOffset{};
	uint32_t AudioWrapAddress{};
	uint32_t AudioInLastAddress{};
};

nosResult RegisterDMAReadNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMARead"), DMAReadNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}