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
struct Channel
{
	uuid ChannelPinId;
	NodeContext* Context;

	size_t DropCount = 0;

	void IncrementDropCount()
	{
		SetStatus(StatusType::DropCount, fb::NodeStatusMessageType::WARNING, "Drop Count: " + std::to_string(++DropCount));
	}
	
	Channel(NodeContext* context) : Context(context) {}

	TChannelInfo Info{};

	bool IsOpen = false;

	std::shared_ptr<AJADevice> GetDevice() const;

	NTV2Channel GetChannel() const;

	AJADevice::Mode GetMode() const;

	std::pair<bool, std::string> Open();

	void Close();

	bool Update(TChannelInfo newChannelInfo, bool setPinValue);

	void UpdateStatus();

	enum class StatusType
	{
		Channel,
		Reference,
		ReferenceInvalid,
		DeltaSecondsCompatible,
        Firmware,
		DropCount,
	};

	void SetStatus(StatusType statusType, fb::NodeStatusMessageType msgType, std::string text);
	void ClearStatus(StatusType statusType);
	std::unordered_map<StatusType, fb::TNodeStatusMessage> StatusMessages;
};
}