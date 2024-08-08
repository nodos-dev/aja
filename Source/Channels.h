/*
 * Copyright MediaZ Teknoloji A.S. All Rights Reserved.
 */

#pragma once

#include <Nodos/PluginHelpers.hpp>

#include <ntv2enums.h>

#include "AJA_generated.h"
#include "AJADevice.h"

namespace nos::aja
{
struct AJASelectChannelCommand
{
	uint32_t DeviceIndex : 4;
	NTV2Channel Channel : 5;
	NTV2VideoFormat Format : 12;
	uint32_t Input : 1;
	uint32_t IsQuad : 1;
	operator uint32_t() const { return *(uint32_t*)this; }
};
static_assert(sizeof(AJASelectChannelCommand) == sizeof(uint32_t));

struct Channel
{
	nosUUID ChannelPinId;
	NodeContext* Context;

	Channel(NodeContext* context) : Context(context) {}

	TChannelInfo Info{};

	bool IsOpen = false;

	std::shared_ptr<AJADevice> GetDevice() const;

	NTV2Channel GetChannel() const;

	AJADevice::Mode GetMode() const;

	bool Open();

	void Close();

	bool Update(TChannelInfo newChannelInfo, bool setPinValue);

	void UpdateStatus();

	enum class StatusType
	{
		Channel,
		Reference
	};

	void SetStatus(StatusType statusType, fb::NodeStatusMessageType msgType, std::string text);
	void ClearStatus(StatusType statusType);
	std::unordered_map<StatusType, fb::TNodeStatusMessage> StatusMessages;
};

void EnumerateOutputChannels(flatbuffers::FlatBufferBuilder& fbb, std::vector<flatbuffers::Offset<nos::ContextMenuItem>>& devices);
void EnumerateInputChannels(flatbuffers::FlatBufferBuilder& fbb, std::vector<flatbuffers::Offset<nos::ContextMenuItem>>& devices);
}