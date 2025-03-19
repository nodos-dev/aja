// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include "Channels.h"

#include "AJADevice.h"

#include <Nodos/PluginHelpers.hpp>
#include <ntv2utils.h>

namespace nos::aja
{
std::shared_ptr<AJADevice> Channel::GetDevice() const
{
	if (!Info.device)
		return nullptr;
	return AJADevice::GetDeviceBySerialNumber(Info.device->serial_number);
}

NTV2Channel Channel::GetChannel() const
{
	if (Info.channel_name.empty())
		return NTV2_CHANNEL_INVALID;
	return ParseChannel(Info.channel_name);
}

AJADevice::Mode Channel::GetMode() const
{
	if (!Info.is_quad)
		return AJADevice::SL;
	if (Info.is_input)
		return static_cast<AJADevice::Mode>(Info.input_quad_link_mode);
	return static_cast<AJADevice::Mode>(Info.output_quad_link_mode);
}

std::pair<bool, std::string> Channel::Open()
{
	DropCount = 0;
	ClearStatus(StatusType::DropCount);
	auto device = GetDevice();
	auto channel = GetChannel();
	if (!device || channel == NTV2_CHANNEL_INVALID)
	{
		std::string message = "Invalid channel";
		SetStatus(StatusType::Channel, fb::NodeStatusMessageType::FAILURE, message, "", 0, false);
		return {false, std::move(message)};
	}
	DeviceLock lock(device.get());
	NTV2VideoFormat fmt = static_cast<NTV2VideoFormat>(Info.video_format_idx); //AJADevice::GetMatchingFormat(Info.video_format, AJADevice::IsQuad(GetMode()));
	if (Info.is_input)
	{
		nosEngine.LogI("Route input %s", NTV2ChannelToString(channel, true).c_str());
	}
	else
	{
		nosEngine.LogI("Route output %s with framerate %s",
		               NTV2ChannelToString(channel, true).c_str(),
		               NTV2VideoFormatToString(fmt, true).c_str());
	}

	if (device->RouteSignal(channel,
	                        fmt,
	                        Info.is_input,
	                        GetMode(),
	                        Info.frame_buffer_format == mediaio::YCbCrPixelFormat::YUV8
		                        ? NTV2_FBF_8BIT_YCBCR
		                        : NTV2_FBF_10BIT_YCBCR))
	{
		device->SetRegisterWriteMode(
			IsProgressivePicture(fmt) ? NTV2_REGWRITE_SYNCTOFRAME : NTV2_REGWRITE_SYNCTOFIELD,
			channel);

		auto channelName = [&]()
		{
			std::string ch = NTV2ChannelToString(channel, true);

			assert(!Info.is_quad || (Info.is_quad && !Info.is_interlaced));

			if (Info.is_quad)
			{
				for (int i = channel + 1; i < channel + 4; ++i)
					ch += i + '1';
			}
			
			ch += ' ' + NTV2VideoFormatToString(fmt, true);

			if (Info.is_quad && !NTV2_IS_QUAD_FRAME_FORMAT(fmt))
			{
				if(auto it = ch.find("1080p"); it != string::npos)
					ch.replace(it, 5, "UHDp");
			}
			return ch;
		};

		SetStatus(StatusType::Channel, fb::NodeStatusMessageType::INFO, channelName() + " opened", "", 4, true);
		IsOpen = true;
		return {true, ""};
	}
	std::string msg = "Unable to open channel";
	SetStatus(StatusType::Channel, fb::NodeStatusMessageType::FAILURE, "Unable to open channel", "Failed channel is " + NTV2ChannelToString(channel, true), 4, false);
	return {false, msg};
}

void Channel::Close()
{
	SetStatus(StatusType::Channel, fb::NodeStatusMessageType::WARNING, "Channel closed", "", 4, false);
	ClearStatus(StatusType::DropCount);
	auto device = GetDevice();
	if (!device)
		return;
	DeviceLock lock(device.get());
	auto channel = GetChannel();
	device->CloseChannel(channel, Info.is_input, AJADevice::IsQuad(GetMode()));
	IsOpen = false;
	nosOrphanState orphanState{.Type = NOS_ORPHAN_STATE_TYPE_ORPHAN, .Message = "Channel closed"};
	nosEngine.SetItemOrphanState(ChannelPinId, &orphanState);
}

bool Channel::Update(TChannelInfo newChannelInfo, bool setPinValue)
{
	if (newChannelInfo != Info)
	{
		Close();
		Info = std::move(newChannelInfo);
		if (setPinValue)
			nosEngine.SetPinValue(ChannelPinId, Buffer::From(Info));
		nosEngine.SendPathRestart(ChannelPinId);
		auto [success, message] = Open();
		IsOpen = success;
		nosOrphanState orphanState{.Type = NOS_ORPHAN_STATE_TYPE_ORPHAN, .Message = message.c_str()};
		nosEngine.SetItemOrphanState(ChannelPinId, success ? nullptr : &orphanState);
	}
	return IsOpen;
}

void Channel::UpdateStatus()
{
	std::vector<fb::TNodeStatusMessage> messages;
	if (auto device = GetDevice())
		messages.push_back(fb::TNodeStatusMessage{{}, device->GetDisplayName(), fb::NodeStatusMessageType::INFO, "", 5, true, false});
	for (auto& [type, message] : StatusMessages)
		messages.push_back(message);
	Context->SetNodeStatusMessages(messages);
}

void Channel::SetStatus(StatusType statusType, fb::NodeStatusMessageType msgType, std::string text, std::string details, uint64_t messageTimeout, bool popup)
{
	StatusMessages[statusType] = fb::TNodeStatusMessage{ {}, std::move(text), msgType, std::move(details), messageTimeout, true, popup};
	UpdateStatus();
}

void Channel::ClearStatus(StatusType statusType)
{
	StatusMessages.erase(statusType);
	UpdateStatus();
}

}
