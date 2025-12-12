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

#define NOS_AJA_AUDIO_INPUT_DIAGNOSTICS 0

struct DMAReadNodeContext : DMANodeBase
{
	DMAReadNodeContext() : DMANodeBase(DMA_READ)
	{}

	void OnPathStart() override
	{
		LastReadAudioBufferOffset = 0;
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
		outAudioPacket.mutate_sample_stride(sizeof(ULWord));
		if (!bufToWrite)
			return NOS_RESULT_FAILED;
		auto dstBufInfo = *sys::vulkan::GetResourceInfo(bufToWrite);
		auto audioBuffer = nosVulkan->Map(bufToWrite);
		if (!audioBuffer)
			return NOS_RESULT_FAILED;
		bool audioRunning = false;
		auto inSource = GetNTV2InputSourceForIndex(Channel, NTV2_INPUTSOURCES_SDI);
		auto embeddedIn = NTV2InputSourceToEmbeddedAudioInput(inSource);
		AudioSystem = NTV2InputSourceToAudioSystem(inSource);
		Device->IsAudioInputRunning(AudioSystem, audioRunning);

		if (!audioRunning)
		{
			auto itemPath = nos::GetItemPath(NodeId).value_or("<unknown>");
			if (!Device->SetAudioSystemInputSource(AudioSystem, NTV2_AUDIO_EMBEDDED, embeddedIn))
				nosEngine.LogE("%s: Failed to set audio system input source", itemPath.c_str());
			if (!Device->SetEmbeddedAudioInput(embeddedIn, AudioSystem))
				nosEngine.LogE("%s: Failed to set embedded audio input", itemPath.c_str());
			if (!Device->SetAudioCaptureEnable(AudioSystem, true))
				nosEngine.LogE("%s: Failed to enable audio capture", itemPath.c_str());
			if (!Device->StartAudioInput(AudioSystem, true))
				nosEngine.LogE("%s: Failed to start audio input", itemPath.c_str());
			Device->GetAudioReadOffset(AudioReadOffset, AudioSystem);
			Device->SetNumberAudioChannels(6, AudioSystem);
			Device->SetAudioRate(NTV2_AUDIO_48K, AudioSystem);
			outAudioPacket.mutate_channel_count(6);

			// Get raw hardware wrap address
			ULWord wrapAddress = 0;
			Device->GetAudioWrapAddress(wrapAddress, AudioSystem);

			// Normalize wrap address so it lives in the same coordinate space
			AudioInWrapAddress = wrapAddress + AudioReadOffset;

			// Initialize last-read pointer to current hardware location
			Device->ReadAudioLastIn(LastReadAudioBufferOffset, AudioSystem);
			LastReadAudioBufferOffset &= ~0x3UL;
			LastReadAudioBufferOffset += AudioReadOffset;
			outAudioPacket.mutate_num_samples(0);
			return NOS_RESULT_SUCCESS;
		}
		ULWord audioChannelCount = 0;
		Device->GetNumberAudioChannels(audioChannelCount, AudioSystem);
		outAudioPacket.mutate_channel_count(audioChannelCount);

		uint32_t currentAudioInAddress = 0;
		Device->ReadAudioLastIn(currentAudioInAddress, AudioSystem);
		//currentAudioInAddress &= ~0x3UL; //	Force DWORD alignment
		currentAudioInAddress -= (currentAudioInAddress % (sizeof(ULWord) * audioChannelCount)); //	Force sample alignment
		currentAudioInAddress += AudioReadOffset;

		uint32_t bytesArrived = 0;
		if (currentAudioInAddress < LastReadAudioBufferOffset)
		{
			//	Audio address has wrapped around the end of the buffer.
			//	Do the calculations and transfer from the last address to the end of the buffer...
			ULWord firstPartSize = AudioInWrapAddress - LastReadAudioBufferOffset;
			ULWord remainingSize = currentAudioInAddress - AudioReadOffset;
			bytesArrived = firstPartSize + remainingSize;

			if (bytesArrived > dstBufInfo.Size)
			{
				nosEngine.LogE("%s: Audio read size exceeds buffer size! Discarding excess.",
								nos::GetItemPath(NodeId).value_or("<unknown>").c_str());
				if (firstPartSize >= dstBufInfo.Size)
				{
					firstPartSize = dstBufInfo.Size;
					remainingSize = 0;
				}
				else
				{
					remainingSize = dstBufInfo.Size - firstPartSize;
				}
				bytesArrived = firstPartSize + remainingSize;
			}
			remainingSize -= (remainingSize % (sizeof(ULWord) * audioChannelCount)); //	Force sample alignment
			// Read audio data up to the wrap address
			Device->DMAReadAudio(AudioSystem, (ULWord*)audioBuffer, LastReadAudioBufferOffset, firstPartSize);
			// Read the remaining audio data from the start
			Device->DMAReadAudio(AudioSystem, (ULWord*)(audioBuffer + firstPartSize), AudioReadOffset, remainingSize);
		}
		else
		{
			bytesArrived = currentAudioInAddress - LastReadAudioBufferOffset;
			if (bytesArrived > dstBufInfo.Size)
			{
				nosEngine.LogE("%s: Audio read size exceeds buffer size! Discarding excess.",
								nos::GetItemPath(NodeId).value_or("<unknown>").c_str());
				bytesArrived = dstBufInfo.Size;
			}
			// Read audio data ahead of the record head
			// Use the current record head position as the read offset
			Device->DMAReadAudio(AudioSystem, (ULWord*)audioBuffer, LastReadAudioBufferOffset, bytesArrived);
		}
		outAudioPacket.mutate_num_samples(bytesArrived / sizeof(ULWord) / audioChannelCount);
		LastReadAudioBufferOffset = currentAudioInAddress;

#if NOS_AJA_AUDIO_INPUT_DIAGNOSTICS
		ULWord recordHeadPos{};
		Device->ReadAudioLastIn(recordHeadPos, AudioSystem);
		recordHeadPos &= ~0x3UL;
		recordHeadPos += AudioReadOffset;
		float recordHead = 100.f * (float(recordHeadPos) / float(AudioInWrapAddress));
		float lastRead = 100.f * (float(LastReadAudioBufferOffset) / float(AudioInWrapAddress));
		auto nodePath = nos::GetItemPath(NodeId).value_or("<unknown>");
		nosEngine.LogI("%s: Record Head: %.2f, Last Read: %.2f", nodePath.c_str(), recordHead, lastRead);
#endif
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

		std::unordered_map<nos::Name, nos::ObjectRef> audioPacketFields;
		audioPacketFields[NOS_NAME("desc")] = PrimitiveObjectRef::Create(NOS_NAME("nos.audio.AudioPacketDescriptor"), nos::Buffer::From(audioPacketDesc)).value_or(ObjectRef());
		audioPacketFields[NOS_NAME("buffer")] = audioBufferToWrite;
		auto packetObj = CompositeObjectRef::Create(NOS_NAME("nos.audio.AudioPacket"), audioPacketFields);

		SetPinObject(NOS_NAME("Output"), dstBufferObject);
		SetPinObject(NOS_NAME("AudioPacket"), packetObj.value_or(ObjectRef()));

		return NOS_RESULT_SUCCESS;
	}

	NTV2AudioSystem AudioSystem{};
	ULWord LastReadAudioBufferOffset;
	ULWord AudioReadOffset;
	ULWord AudioInWrapAddress;
};

nosResult RegisterDMAReadNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.DMARead"), DMAReadNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}