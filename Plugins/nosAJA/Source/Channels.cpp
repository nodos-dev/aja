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

bool Channel::Open()
{
	auto device = GetDevice();
	auto channel = GetChannel();
	if (!device || channel == NTV2_CHANNEL_INVALID)
	{
		std::stringstream text;
		text << "Invalid channel";
		if (!device)
			text << ": Device not found";
		else if (channel == NTV2_CHANNEL_INVALID)
			text << "!";
		SetStatus(StatusType::Channel, fb::NodeStatusMessageType::FAILURE, text.str());
		return false;
	}
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
		auto text = NTV2ChannelToString(channel, true) + " " + NTV2VideoFormatToString(fmt, true);
		SetStatus(StatusType::Channel, fb::NodeStatusMessageType::INFO, text);
		IsOpen = true;
		return true;
	}
	SetStatus(StatusType::Channel, fb::NodeStatusMessageType::FAILURE, "Unable to open channel " + NTV2ChannelToString(channel, true));
	return false;
}

void Channel::Close()
{
	SetStatus(StatusType::Channel, fb::NodeStatusMessageType::INFO, "Channel closed");
	auto device = GetDevice();
	if (!device)
		return;
	auto channel = GetChannel();
	device->CloseChannel(channel, Info.is_input, AJADevice::IsQuad(GetMode()));
	IsOpen = false;
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
		if (Open())
		{
			nosEngine.SetItemOrphanState(ChannelPinId, nullptr);
			return true;
		}
		else
		{
			IsOpen = false;
			nosOrphanState orphanState{.IsOrphan = true, .Message = "Invalid channel"};
			nosEngine.SetItemOrphanState(ChannelPinId, &orphanState);
			return false;
		}
	}
	return IsOpen;
}

void Channel::UpdateStatus()
{
	std::vector<fb::TNodeStatusMessage> messages;
	if (auto device = GetDevice())
		messages.push_back(fb::TNodeStatusMessage{{}, device->GetDisplayName(), fb::NodeStatusMessageType::INFO});
	for (auto& [type, message] : StatusMessages)
		messages.push_back(message);
	Context->SetNodeStatusMessages(messages);
}

void Channel::SetStatus(StatusType statusType, fb::NodeStatusMessageType msgType, std::string text)
{
	StatusMessages[statusType] = fb::TNodeStatusMessage{{}, text, msgType};
	UpdateStatus();
}

void Channel::ClearStatus(StatusType statusType)
{
	StatusMessages.erase(statusType);
	UpdateStatus();
}

template <class K, class V> using SeqMap = std::vector<std::pair<K, V>>;
auto EnumerateFormats()
{
	struct FormatDescriptor
	{
		NTV2VideoFormat Format;
		NTV2FrameRate FPS;
		u32 Width, Height;
		u8 Interlaced : 1;
		u8 ALevel : 1;
		u8 BLevel : 1;
	};

	std::map<u64, std::map<NTV2FrameRate, std::vector<FormatDescriptor>>> re;

	for (auto fmt = NTV2_FORMAT_FIRST_HIGH_DEF_FORMAT; fmt < NTV2_MAX_NUM_VIDEO_FORMATS; fmt = NTV2VideoFormat(fmt + 1))
	{
		if (IsPSF(fmt))
			continue;
		u32 w = GetDisplayWidth(fmt);
		u32 h = GetDisplayHeight(fmt);
		NTV2FrameRate fps = GetNTV2FrameRateFromVideoFormat(fmt);
		auto desc = FormatDescriptor{
			.Format = fmt,
			.FPS = fps,
			.Width = w,
			.Height = h,
			.Interlaced = !IsProgressiveTransport(fmt),
			.ALevel = NTV2_VIDEO_FORMAT_IS_A(fmt),
			.BLevel = NTV2_VIDEO_FORMAT_IS_B(fmt),
		};

		u64 extent = ((u64(w) << u64(32)) | u64(h));
		re[extent][fps].push_back(desc);
	}

	SeqMap<fb::vec2u, SeqMap<f64, std::vector<FormatDescriptor>>> re2;

	std::transform(re.begin(), re.end(), std::back_inserter(re2), [](auto &p) {
		auto extent = p.first;
		auto &container = p.second;
		SeqMap<f64, std::vector<FormatDescriptor>> XX;
		std::transform(container.begin(), container.end(), std::back_inserter(XX),
					   [](auto &p) { return std::pair(GetFramesPerSecond(p.first), std::move(p.second)); });
		std::sort(XX.begin(), XX.end(), [](auto &a, auto &b) {
			if (a.first == 50)
				return true;
			if (b.first == 50)
				return false;
			return a.first > b.first;
		});
		return std::pair(fb::vec2u((extent >> 32) & 0xFFFFFFFF, extent & 0xFFFFFFFF), std::move(XX));
	});

	std::sort(re2.begin(), re2.end(), [](auto &a, auto &b) {
		if (a.first.x() == 1920)
			return true;
		if (b.first.x() == 1920)
			return false;
		return a.first.x() > b.first.x();
	});
	return re2;
}

void EnumerateOutputChannels(flatbuffers::FlatBufferBuilder& fbb, std::vector<flatbuffers::Offset<nos::ContextMenuItem>>& devices)
{
	for (auto& [serial, device] : AJADevice::Devices)
	{
		std::vector<flatbuffers::Offset<nos::ContextMenuItem>> channels;
		static auto Descriptors = EnumerateFormats();
		for (u32 i = NTV2_CHANNEL1; i < NTV2_MAX_NUM_CHANNELS; ++i)
		{
			AJADevice::Mode modes[2] = {AJADevice::SL, AJADevice::AUTO};
			std::vector<flatbuffers::Offset<nos::ContextMenuItem>> outs;
			for (auto mode : modes)
			{
				NTV2Channel channel = NTV2Channel(AJADevice::SL == mode ? i : NTV2_MAX_NUM_CHANNELS - i - 1);
				AJASelectChannelCommand action = {
					.DeviceIndex = device->GetIndexNumber(),
					.Channel = channel,
					.Input = false,
					.IsQuad = (AJADevice::SL == mode) ? 0u : 1u
				};
				auto it = AJADevice::SL != mode ? channels.begin() : channels.end();
				std::vector<flatbuffers::Offset<nos::ContextMenuItem>> extents;
				for (auto& [extent, Container0] : Descriptors)
				{
					std::vector<flatbuffers::Offset<nos::ContextMenuItem>> frameRates;
					for (auto& [fps, Container1] : Container0)
					{
						std::vector<flatbuffers::Offset<nos::ContextMenuItem>> formats;
						for (auto& desc : Container1)
						{
							if (device->ChannelIsValid(channel, false, desc.Format, mode))
							{
								action.Format = desc.Format;
								std::string name = (desc.Interlaced ? "Interlaced" : "Progressive");
								if (desc.ALevel)
									name += "-A";
								if (desc.BLevel)
									name += "-B";
								formats.push_back(nos::CreateContextMenuItemDirect(fbb, name.c_str(), action));
							}
						}

						if (!formats.empty())
						{
							char buf[16] = {};
							std::sprintf(buf, "%.2f", fps);
							frameRates.push_back(nos::CreateContextMenuItemDirect(fbb, buf, 0, &formats));
						}
					}

					if (!frameRates.empty())
					{
						char buf[32] = {};
						std::sprintf(buf, "%dx%d", extent.x(), extent.y());
						extents.push_back(nos::CreateContextMenuItemDirect(fbb, buf, 0, &frameRates));
					}
				}
				
				if (!extents.empty())
					channels.insert(it, nos::CreateContextMenuItemDirect(fbb, GetChannelName(channel, mode).c_str(), 0, &extents));
			}
		}
		if (!channels.empty())
			devices.push_back(nos::CreateContextMenuItemDirect(fbb, device->GetDisplayName().c_str(), 0, &channels));
	}
}

void EnumerateInputChannels(flatbuffers::FlatBufferBuilder& fbb, std::vector<flatbuffers::Offset<nos::ContextMenuItem>>& devices)
{
	for (auto& [serial, device] : AJADevice::Devices)
	{
		std::vector<flatbuffers::Offset<nos::ContextMenuItem>> channels;
		static auto Descriptors = EnumerateFormats();
		for (u32 i = NTV2_CHANNEL1; i < NTV2_MAX_NUM_CHANNELS; ++i)
		{
			AJADevice::Mode modes[2] = {AJADevice::SL, AJADevice::AUTO};
			std::vector<flatbuffers::Offset<nos::ContextMenuItem>> outs;
			for (auto mode : modes)
			{
				NTV2Channel channel = NTV2Channel(AJADevice::SL == mode ? i : NTV2_MAX_NUM_CHANNELS - i - 1);
				if (!device->ChannelIsValid(channel, true, NTV2_FORMAT_UNKNOWN, mode))
					continue;
				AJASelectChannelCommand action = {
					.DeviceIndex = device->GetIndexNumber(),
					.Channel = channel,
					.Input = true,
					.IsQuad = (AJADevice::SL == mode) ? 0u : 1u
				};
				auto it = AJADevice::SL != mode ? channels.begin() : channels.end();
				channels.insert(it, nos::CreateContextMenuItemDirect(fbb, GetChannelName(channel, mode).c_str(), action));
			}
		}
		if (!channels.empty())
			devices.push_back(nos::CreateContextMenuItemDirect(fbb, device->GetDisplayName().c_str(), 0, &channels));
	}
}
}
