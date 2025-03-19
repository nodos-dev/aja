// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include "Channels.h"
#include "AJAMain.h"

// TODO: Remove this node once things settle down.
namespace nos::aja
{
NOS_REGISTER_NAME(ChannelName);
NOS_REGISTER_NAME(IsInput);
NOS_REGISTER_NAME(Resolution);
NOS_REGISTER_NAME(FrameRate);
NOS_REGISTER_NAME(IsInterlaced);
NOS_REGISTER_NAME(IsQuad);
NOS_REGISTER_NAME(QuadLinkInputMode);
NOS_REGISTER_NAME(QuadLinkOutputMode);
NOS_REGISTER_NAME(IsOpen);
NOS_REGISTER_NAME(FrameBufferFormat);
NOS_REGISTER_NAME(ForceInterlaced);
NOS_REGISTER_NAME(AcquireDevice);

constexpr auto PIN_VALUE_NONE = "None";

enum class AJAChangedPinType
{
	IsInput,
	Device,
	ChannelName,
	Resolution,
	FrameRate
};

struct ChannelNodeContext : NodeContext
{
	uint32_t RefListenerId = 0;
	size_t DropCount = 0;
	ChannelNodeContext(nosFbNodePtr node) : NodeContext(node), CurrentChannel(this)
	{
		if (auto* pins = node->pins())
		{
			for (auto const* pin : *pins)
			{
				auto name = pin->name()->c_str();
				if (0 == strcmp(name, "Channel"))
					CurrentChannel.ChannelPinId = *pin->id();
			}
		}

		nosOrphanState orphan{ .Type = NOS_ORPHAN_STATE_TYPE_ORPHAN, .Message = "Channel is not open" };
		nosEngine.SetItemOrphanState(CurrentChannel.ChannelPinId, &orphan);

		UpdateStringList(GetReferenceStringListName(), {PIN_VALUE_NONE});
		UpdateStringList(GetChannelStringListName(), {PIN_VALUE_NONE});
		UpdateStringList(GetResolutionStringListName(), {PIN_VALUE_NONE});
		UpdateStringList(GetFrameRateStringListName(), {PIN_VALUE_NONE});
		UpdateStringList(GetInterlacedStringListName(), {PIN_VALUE_NONE});
		
		SetPinVisualizer(NSN_ReferenceSource, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetReferenceStringListName()});
		SetPinVisualizer(NSN_Device, {.type = nos::fb::VisualizerType::NAMED_VALUE, .name = sys::device::GetDeviceListNameForVendor(NSN_VendorName), .hide_value = true});
		SetPinVisualizer(NSN_ChannelName, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetChannelStringListName()});
		SetPinVisualizer(NSN_Resolution, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetResolutionStringListName()});
		SetPinVisualizer(NSN_FrameRate, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetFrameRateStringListName()});
		SetPinVisualizer(NSN_IsInterlaced, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetInterlacedStringListName()});

		AddPinValueWatcher(NSN_IsOpen, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			ShouldOpen = *InterpretPinValue<bool>(newVal);
			TryUpdateChannel();
		});
		AddPinValueWatcher(NSN_IsInput, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			IsInput = *InterpretPinValue<bool>(newVal);
			if (oldValue)
				ResetAfter(AJAChangedPinType::IsInput);
			UpdateAfter(AJAChangedPinType::IsInput, !oldValue);
		});
		AddPinValueWatcher(NSN_Device, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			DevicePinValue = newVal.As<sys::device::TDeviceInfo>();
			auto oldDevice = Device;

			nosDeviceInfo deviceInfoFromPin = sys::device::ConvertDeviceInfo(DevicePinValue);
			nosDeviceId deviceId{};
			uint64_t newDeviceSerial = -1;
			auto res = nosDevice->GetSuitableDevice(&deviceInfoFromPin, &deviceId);
			if (res != NOS_RESULT_SUCCESS)
			{
				if (deviceInfoFromPin.VendorName != NOS_NAME("None"))
					nosEngine.LogE("Failed to get suitable device");
			}
			else
			{
				uint64_t handle{};
				res = nosDevice->GetDeviceHandle(deviceId, &handle);
				assert(res == NOS_RESULT_SUCCESS);
				newDeviceSerial = handle;
			}
			
			Device = AJADevice::GetDeviceBySerialNumber(newDeviceSerial).get();

			std::string msg, msgDetails;
			if (Device && !Device->CheckFirmware(msg, msgDetails))
			{
				CurrentChannel.SetStatus(aja::Channel::StatusType::Firmware, fb::NodeStatusMessageType::WARNING, msg, msgDetails, 0, false);
			}

			if (oldDevice != Device)
			{
				if (oldDevice)
				{
					oldDevice->UnregisterNode(NodeId);
					oldDevice->RemoveReferenceSourceListener(RefListenerId);
					if (DeviceAcquired)
					{
						oldDevice->ReleaseDevice();
						DeviceAcquired = false;
					}
				}
				if(Device)
				{
					Device->RegisterNode(NodeId);
					RefListenerId = Device->AddReferenceSourceListener([this](NTV2ReferenceSource ref) {
						auto refStr = NTV2ReferenceSourceToString(ref, true);
						SetPinValue(NSN_ReferenceSource, nosBuffer{ .Data = (void*)refStr.c_str(), .Size = refStr.size() + 1 });
					});
					if (!DeviceAcquired && ShouldAcquireDevice)
					{
						Device->AcquireDevice();
						DeviceAcquired = true;
					}
				}
				else
					RefListenerId = 0;
			}
			if (DevicePinValue.vendor_name != PIN_VALUE_NONE && !Device)
				ResetDevicePin();
			else
			{
				if (oldValue)
					ResetAfter(AJAChangedPinType::Device);
				else if (DevicePinValue.vendor_name == PIN_VALUE_NONE)
					AutoSelectIfSingle(NSN_Device, GetPossibleDevices());
			}
			UpdateAfter(AJAChangedPinType::Device, !oldValue);
		});
		AddPinValueWatcher(NSN_AcquireDevice, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			bool acquire = *newVal.As<bool>();
			ShouldAcquireDevice = acquire;
			if (!Device)
				return;
			if (ShouldAcquireDevice)
			{
				if (!DeviceAcquired)
				{
					Device->AcquireDevice();
					DeviceAcquired = true;
				}
			}
			else
			{
				if (DeviceAcquired)
				{
					Device->ReleaseDevice();
					DeviceAcquired = false;
				}
			}
		});
		AddPinValueWatcher(NSN_ChannelName, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			ChannelPinValue = InterpretPinValue<const char>(newVal);
			auto [channel, mode] = GetChannelFromString(ChannelPinValue);
			Channel = channel;
			IsSingleLink = !AJADevice::IsQuad(mode);
			if (ChannelPinValue != PIN_VALUE_NONE && Channel == NTV2_CHANNEL_INVALID)
				SetPinValue(NSN_ChannelName, nosBuffer{.Data = (void*)PIN_VALUE_NONE, .Size = 5});
			else
			{
				if (oldValue)
					ResetAfter(AJAChangedPinType::ChannelName);
				else if (ChannelPinValue == PIN_VALUE_NONE)
					AutoSelectIfSingle(NSN_ChannelName, GetPossibleChannelNames());
			}
			UpdateAfter(AJAChangedPinType::ChannelName, !oldValue);
		});
		AddPinValueWatcher(NSN_Resolution, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			ResolutionPinValue = InterpretPinValue<const char>(newVal);
			Resolution = GetNTV2FrameGeometryFromString(ResolutionPinValue);
			if (ResolutionPinValue != PIN_VALUE_NONE && Resolution == NTV2_FG_INVALID)
				SetPinValue(NSN_Resolution, nosBuffer{.Data = (void*)PIN_VALUE_NONE, .Size = 5});
			else
			{
				if (oldValue)
					ResetAfter(AJAChangedPinType::Resolution);
				else if (ResolutionPinValue == PIN_VALUE_NONE)
					AutoSelectIfSingle(NSN_Resolution, GetPossibleResolutions());
			}
			UpdateAfter(AJAChangedPinType::Resolution, !oldValue);
		});
		AddPinValueWatcher(NSN_FrameRate, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			FrameRatePinValue = InterpretPinValue<const char>(newVal);
			FrameRate = GetNTV2FrameRateFromString(FrameRatePinValue);
			if (FrameRatePinValue != PIN_VALUE_NONE && FrameRate == NTV2_FRAMERATE_INVALID)
				SetPinValue(NSN_FrameRate, nosBuffer{.Data = (void*)PIN_VALUE_NONE, .Size = 5});
			else
			{
				if (oldValue)
					ResetAfter(AJAChangedPinType::FrameRate);
				else if (FrameRatePinValue == PIN_VALUE_NONE)
					AutoSelectIfSingle(NSN_FrameRate, GetPossibleFrameRates());
			}
			UpdateAfter(AJAChangedPinType::FrameRate, !oldValue);
		});
		AddPinValueWatcher(NSN_IsInterlaced, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			std::string interlaced = InterpretPinValue<const char>(newVal);
			InterlacedState = InterlacedState::NONE;
			if (interlaced == PIN_VALUE_NONE)
				InterlacedState = InterlacedState::NONE;
			else if (interlaced == "Interlaced")
				InterlacedState = InterlacedState::INTERLACED;
			else if (interlaced == "Progressive")
				InterlacedState = InterlacedState::PROGRESSIVE;

			if (interlaced != PIN_VALUE_NONE && InterlacedState == InterlacedState::NONE)
				SetPinValue(NSN_IsInterlaced, nosBuffer{.Data = (void*)PIN_VALUE_NONE, .Size = 5});

			if (!oldValue && interlaced == PIN_VALUE_NONE)
				AutoSelectIfSingle(NSN_IsInterlaced, GetPossibleInterlaced());

			TryUpdateChannel();
		});
		AddPinValueWatcher(NSN_FrameBufferFormat, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			CurrentPixelFormat = *InterpretPinValue<mediaio::YCbCrPixelFormat>(newVal);
			TryUpdateChannel();
		});
		AddPinValueWatcher(NSN_ReferenceSource, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			ReferenceSourcePinValue = InterpretPinValue<const char>(newVal);
			NTV2ReferenceSource curRef;
			if (ReferenceSourcePinValue == PIN_VALUE_NONE && Device && Device->GetReference(curRef))
			{
				auto refStr = NTV2ReferenceSourceToString(curRef, true);
				SetPinValue(NSN_ReferenceSource, nosBuffer{ .Data = (void*)refStr.c_str(), .Size = refStr.size() + 1 });
			}
			else
				TryUpdateChannel();
		});
		AddPinValueWatcher(NSN_QuadLinkOutputMode, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			OutputModePin = *InterpretPinValue<AJADevice::Mode>(newVal);
			TryUpdateChannel();
		});
		AddPinValueWatcher(NSN_QuadLinkInputMode, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			InputModePin = *InterpretPinValue<AJADevice::Mode>(newVal);
			TryUpdateChannel();
		});
		AddPinValueWatcher(NSN_ForceInterlaced, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			ForceInterlaced = *InterpretPinValue<bool>(newVal);
			TryUpdateChannel();
		});
	}

	~ChannelNodeContext() override
	{
		if (Device)
		{
			Device->UnregisterNode(NodeId);
			if(RefListenerId)
				Device->RemoveReferenceSourceListener(RefListenerId);
			if (DeviceAcquired)
				Device->ReleaseDevice();
		}
		CurrentChannel.Close();
	}

	static nosResult MigrateNode(nosFbNodePtr node, nosBuffer* outBuffer)
	{
		auto migrated = MigrateChannelNode(node);
		if (!migrated)
			return NOS_RESULT_SUCCESS;

		auto nodeBuffer = nos::EngineBuffer::CopyFrom(*migrated);
		*outBuffer = nodeBuffer.Release();
		return NOS_RESULT_SUCCESS;
	}
	
	mediaio::YCbCrPixelFormat CurrentPixelFormat = mediaio::YCbCrPixelFormat::YUV8;

	void UpdateReferenceSource()
	{
		if (IsInput)
			return;
		ReferenceSource = NTV2_REFERENCE_INVALID;
		if (ReferenceSourcePinValue.empty())
			nosEngine.LogE("Empty value received for reference pin!");
		else if (std::string::npos != ReferenceSourcePinValue.find("Reference In"))
			ReferenceSource = NTV2_REFERENCE_EXTERNAL;
		else if (std::string::npos != ReferenceSourcePinValue.find("Free Run"))
			ReferenceSource = NTV2_REFERENCE_FREERUN;
		else if(auto pos = ReferenceSourcePinValue.find("SDI In"); std::string::npos != pos)
			ReferenceSource = AJADevice::ChannelToRefSrc(NTV2Channel(ReferenceSourcePinValue[pos + 7] - '1'));
		if (ReferenceSource != NTV2_REFERENCE_INVALID)
		{
			NTV2ReferenceSource curRef{};
			if (Device->GetReference(curRef) && curRef != ReferenceSource)
				Device->SetReference(ReferenceSource);
			NTV2FrameRate refFrameRate{};
			Device->GetReferenceAndFrameRate(curRef, refFrameRate);
			
			if (GetFrameRateFamily(refFrameRate) != GetFrameRateFamily(FrameRate))
				CurrentChannel.SetStatus(aja::Channel::StatusType::ReferenceInvalid, fb::NodeStatusMessageType::WARNING, "Reference incompatible with frame rate", "", 5, false);
			else
				CurrentChannel.ClearStatus(Channel::StatusType::ReferenceInvalid);
			
			auto refStatusText = NTV2ReferenceSourceToString(ReferenceSource, true) + " (" + NTV2FrameRateToString(refFrameRate, true) + ")";
			CurrentChannel.SetStatus(aja::Channel::StatusType::Reference, fb::NodeStatusMessageType::INFO, "Reference: " + refStatusText, "", 5, false);
		}
		else
			CurrentChannel.SetStatus(aja::Channel::StatusType::Reference, fb::NodeStatusMessageType::FAILURE, "Reference: None", "", 5, false);
	}
	
	void TryUpdateChannel() 
	{
		if (!ShouldOpen)
		{
			if (CurrentChannel.IsOpen)
				CurrentChannel.Close();
			return;
		}
		CurrentChannel.Update({}, true);
		auto format = GetVideoFormat();
		if (format == NTV2_FORMAT_UNKNOWN)
			return;
		TChannelInfo channelPin{}; 
		channelPin.device = std::make_unique<TDevice>(TDevice{{}, Device->GetSerialNumber(), Device->GetDisplayName()});
		channelPin.channel_name = ChannelPinValue;
		channelPin.is_input = IsInput;
		channelPin.is_quad = !IsSingleLink;
		channelPin.video_format = NTV2VideoFormatToString(format, true); // TODO: Readonly.
		uint32_t width, height;
		if(IsInput)
			Device->GetExtent(format, GetEffectiveQuadMode(), width, height);
		else
		{
			auto geometry = GetNTV2FrameGeometryFromVideoFormat(format);
			width = GetNTV2FrameGeometryWidth(geometry);
			height = GetNTV2FrameGeometryHeight(geometry);
		}
		channelPin.resolution = std::make_unique<nos::fb::vec2u>(width, height);
		channelPin.video_format_idx = static_cast<int>(format);
		if (!IsSingleLink)
		{
			if(IsInput)
				channelPin.input_quad_link_mode = static_cast<nos::aja::QuadLinkInputMode>(GetEffectiveQuadMode());
			else 
				channelPin.output_quad_link_mode = static_cast<QuadLinkMode>(GetEffectiveQuadMode());
		}
		channelPin.frame_buffer_format = static_cast<mediaio::YCbCrPixelFormat>(CurrentPixelFormat);
		channelPin.is_interlaced = !IsProgressivePicture(format);
 		CurrentChannel.Update(std::move(channelPin), true);
		UpdateReferenceSource();
	}

	void CheckChannelConfig()
	{
		if (!Device)
			return;
		// Check if the current channel configuration & pin values are valid. If not, close the channel and reset invalid pins to NONE.
		if (CurrentChannel.IsOpen)
		{
			// Check if output && FPSFamily is suitable
			if (IsInput)
				return;
			if (GetVideoFormat() == NTV2_FORMAT_UNKNOWN)
			{
				ResetFrameRatePin();
				UpdateStringList(GetFrameRateStringListName(), GetPossibleFrameRates());
			}
			return;
		}

		// If the channel is not open, get string lists, if string list does not contain current value, set to NONE.
		auto channelList = GetPossibleChannelNames();
		UpdateStringList(GetChannelStringListName(), channelList);
		if (Channel == NTV2_CHANNEL_INVALID)
			return;
		if (std::find(channelList.begin(), channelList.end(), ChannelPinValue) == channelList.end())
		{
			ResetChannelNamePin();
			return;
		}

		auto resolutionList = GetPossibleResolutions();
		UpdateStringList(GetResolutionStringListName(), resolutionList);
		if (Resolution == NTV2_FG_INVALID)
			return;
		if (std::find(resolutionList.begin(), resolutionList.end(), ResolutionPinValue) == resolutionList.end())
		{
			ResetResolutionPin();
			return;
		}

		auto frameRateList = GetPossibleFrameRates();
		UpdateStringList(GetFrameRateStringListName(), frameRateList);
		if (FrameRate == NTV2_FRAMERATE_INVALID)
			return;
		if (std::find(frameRateList.begin(), frameRateList.end(), FrameRatePinValue) == frameRateList.end())
		{
			ResetFrameRatePin();
			return;
		}
	}

	void CheckChannelStatus()
	{
		if (!Device || !IsInput)
			return;
		if (CurrentChannel.IsOpen)
		{
			auto format = Device->GetSDIInputVideoFormat(Channel);
			if (ForceInterlaced)
				format = Device->ForceInterlace(format);
			if (format == CurrentChannel.Info.video_format_idx)
				return;
		}
		TryUpdateChannel();
		if(CurrentChannel.IsOpen)
			nosEngine.LogI("Input signal reconnected.");
	}

	void UpdateVisualizer(nos::Name pinName, std::string stringListName)
	{ 
		SetPinVisualizer(pinName, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = stringListName});
	}

	template <typename T>
	void AutoSelectIfSingle(nosName pinName, std::vector<T> const& list)
	{
		if constexpr (std::is_same_v<T, std::string>)
		{
			if (list.size() == 2)
				SetPinValue(pinName, nosBuffer{.Data = (void*)list[1].c_str(), .Size = list[1].size() + 1});
		}
		if constexpr (std::is_same_v<T, sys::device::TDeviceInfo>)
		{
			if (list.size() == 1)
				SetPinValue(pinName, nos::Buffer::From(list[0]));
		}
	}

	void UpdateAfter(AJAChangedPinType pin, bool first)
	{
		switch (pin)
		{
		case AJAChangedPinType::IsInput: {
			ChangePinReadOnly(NSN_Resolution, IsInput);
			ChangePinReadOnly(NSN_FrameRate, IsInput);
			ChangePinReadOnly(NSN_IsInterlaced, IsInput);
			ChangePinReadOnly(NSN_ReferenceSource, IsInput);

			if (!first)
			{
				auto deviceList = GetPossibleDevices();
				AutoSelectIfSingle(NSN_Device, deviceList);
			}
			break;
		}
		case AJAChangedPinType::Device: {
			auto channelList = GetPossibleChannelNames();
			UpdateStringList(GetChannelStringListName(), channelList);
			if (!first)
				AutoSelectIfSingle(NSN_ChannelName, channelList);
			if (IsInput || !Device)
			{
				UpdateStringList(GetReferenceStringListName(), { PIN_VALUE_NONE });
				SetPinValue(NSN_ReferenceSource, nosBuffer{ .Data = (void*)PIN_VALUE_NONE, .Size = 5 });
			}
			else
			{
				std::vector<std::string> list{"Reference In", "Free Run"};
				for (int i = 1; i <= NTV2DeviceGetNumVideoInputs(Device->ID); ++i)
					list.push_back("SDI In " + std::to_string(i));
				UpdateStringList(GetReferenceStringListName(), list);
				if (!first && Device->GetReference(ReferenceSource))
				{
					auto refStr = NTV2ReferenceSourceToString(ReferenceSource, true);
					SetPinValue(NSN_ReferenceSource, nosBuffer{ .Data = (void*)refStr.c_str(), .Size = refStr.size() + 1 });
				}
			}
			break;
		}
		case AJAChangedPinType::ChannelName: {
			if (IsInput)
				TryUpdateChannel();
			auto resolutionList = GetPossibleResolutions();
			UpdateStringList(GetResolutionStringListName(), resolutionList);
			if(!IsInput && !first )
				AutoSelectIfSingle(NSN_Resolution, resolutionList);
			break;
		}
		case AJAChangedPinType::Resolution: {
			auto frameRateList = GetPossibleFrameRates();
			UpdateStringList(GetFrameRateStringListName(), frameRateList);
			if (!first)
				AutoSelectIfSingle(NSN_FrameRate, frameRateList);
			break;
		}
		case AJAChangedPinType::FrameRate: {
			auto interlacedList = GetPossibleInterlaced();
			UpdateStringList(GetInterlacedStringListName(), interlacedList);
			if (!first)
				AutoSelectIfSingle(NSN_IsInterlaced, interlacedList);
			break;
		}
		}
	}

	void ResetAfter(AJAChangedPinType pin)
	{
		CurrentChannel.Update({}, true);
		nos::Name pinToSet;
		switch (pin)
		{
		case AJAChangedPinType::IsInput: 
			ResetDevicePin();
			return;
		case AJAChangedPinType::Device: 
			pinToSet = NSN_ChannelName; 
			break;
		case AJAChangedPinType::ChannelName: 
			pinToSet = NSN_Resolution; 
			break;
		case AJAChangedPinType::Resolution: 
			pinToSet = NSN_FrameRate; 
			break;
		case AJAChangedPinType::FrameRate: 
			pinToSet = NSN_IsInterlaced; 
			break;
		}
		SetPinValue(pinToSet, nosBuffer{.Data = (void*)PIN_VALUE_NONE, .Size = 5});
	}

	void ResetDevicePin()
	{ 
		SetPinValue(NSN_Device, nos::Buffer::From(sys::device::NoneDeviceInfo()));
	}
	void ResetChannelNamePin()
	{
		SetPinValue(NSN_ChannelName, nosBuffer{.Data = (void*)PIN_VALUE_NONE, .Size = 5});
	}
	void ResetResolutionPin()
	{
		SetPinValue(NSN_Resolution, nosBuffer{.Data = (void*)PIN_VALUE_NONE, .Size = 5});
	}
	void ResetFrameRatePin()
	{ 
		SetPinValue(NSN_FrameRate, nosBuffer{.Data = (void*)PIN_VALUE_NONE, .Size = 5});
	}
	void ResetInterlacedPin()
	{ 
		SetPinValue(NSN_IsInterlaced, nosBuffer{.Data = (void*)PIN_VALUE_NONE, .Size = 5});
	}

	std::string GetReferenceStringListName() { return "aja.ReferenceSource." + std::string(NodeId); }
	std::string GetChannelStringListName() { return "aja.ChannelList." + std::string(NodeId); }
	std::string GetResolutionStringListName() { return "aja.ResolutionList." + std::string(NodeId); }
	std::string GetFrameRateStringListName() { return "aja.FrameRateList." + std::string(NodeId); }
	std::string GetInterlacedStringListName() { return "aja.InterlacedList." + std::string(NodeId); }

	std::vector<sys::device::TDeviceInfo> GetPossibleDevices()
	{
		return sys::device::GetDevicesWithVendor(NSN_VendorName);
	}

	std::vector<std::string> GetPossibleChannelNames() 
	{
		std::vector<std::string> channels = {PIN_VALUE_NONE};
		if (!Device)
			return channels;
		for (uint32_t i = NTV2_CHANNEL1; i < NTV2_MAX_NUM_CHANNELS; ++i)
		{
			AJADevice::Mode modes[2] = {AJADevice::SL, AJADevice::AUTO};
			for (auto mode : modes)
			{
				if (IsInput)
				{
					if (AJADevice::IsQuad(mode))
					{
						if (Device->CanMakeQuadInputFromChannel(NTV2Channel(i)))
							channels.push_back(GetChannelName(NTV2Channel(i), mode));
					}
					else
					{
						if (Device->ChannelCanInput(NTV2Channel(i)))
							channels.push_back(GetChannelName(NTV2Channel(i), mode));
					}
				}
				else
				{
					if (AJADevice::IsQuad(mode))
					{
						if (Device->CanMakeQuadOutputFromChannel(NTV2Channel(i)))
							channels.push_back(GetChannelName(NTV2Channel(i), mode));
					}
					else
					{
						if (Device->ChannelCanOutput(NTV2Channel(i)))
							channels.push_back(GetChannelName(NTV2Channel(i), mode));
					}
				}
			}
		}
		return channels;
	}
	std::vector<std::string> GetPossibleResolutions() 
	{
		if (Channel == NTV2_CHANNEL_INVALID || !Device)
			return {PIN_VALUE_NONE};
		std::set<NTV2FrameGeometry> resolutions;
		for (int i = 0; i < NTV2_MAX_NUM_VIDEO_FORMATS; ++i)
		{
			NTV2VideoFormat format = NTV2VideoFormat(i);
			if (Device->CanChannelDoFormat(Channel, IsInput, format, GetEffectiveQuadMode()))
			{
				resolutions.insert(GetNTV2FrameGeometryFromVideoFormat(format));
			}
		}
		std::vector<std::string> possibleResolutions = {PIN_VALUE_NONE};
		for (auto res : resolutions)
		{
			possibleResolutions.push_back(NTV2FrameGeometryToString(res, true));
		}
		return possibleResolutions;
	}
	std::vector<std::string> GetPossibleFrameRates() 
	{
		std::vector<std::string> possibleFrameRates = {PIN_VALUE_NONE};
		if (!Device || Resolution == NTV2_FG_INVALID)
			return possibleFrameRates;
		std::set<NTV2FrameRate> frameRates;
		for (int i = 0; i < NTV2_MAX_NUM_VIDEO_FORMATS; ++i)
		{
			if (GetNTV2FrameGeometryFromVideoFormat(NTV2VideoFormat(i)) != Resolution)
				continue;
			NTV2VideoFormat format = NTV2VideoFormat(i);
			if (Device->CanChannelDoFormat(Channel, IsInput, format, GetEffectiveQuadMode()))
			{
				frameRates.insert(GetNTV2FrameRateFromVideoFormat(format));
			}
		}
		for (auto rate : frameRates)
		{
			possibleFrameRates.push_back(NTV2FrameRateToString(rate, true));
		}
		return possibleFrameRates;
	}

	std::vector<std::string> GetPossibleInterlaced()
	{
		std::vector<std::string> possibleInterlaced = {PIN_VALUE_NONE};
		if (!Device || FrameRate == NTV2_FRAMERATE_INVALID)
			return possibleInterlaced;
		std::set<bool> interlaced;
		for (int i = 0; i < NTV2_MAX_NUM_VIDEO_FORMATS; ++i)
		{
			NTV2VideoFormat format = NTV2VideoFormat(i);
			if (GetNTV2FrameGeometryFromVideoFormat(format) != Resolution)
				continue;
			if (GetNTV2FrameRateFromVideoFormat(format) != FrameRate)
				continue;
			if (Device->CanChannelDoFormat(Channel, IsInput, format, GetEffectiveQuadMode()))
			{
				interlaced.insert(!IsProgressiveTransport(format));
			}
		}
		for (auto inter : interlaced)
		{
			possibleInterlaced.push_back(inter ? "Interlaced" : "Progressive");
		}
		return possibleInterlaced;
	}

	NTV2VideoFormat GetVideoFormat()
	{
		if (!Device || Channel == NTV2_CHANNEL_INVALID || (!IsInput && (Resolution == NTV2_FG_INVALID || FrameRate == NTV2_FRAMERATE_INVALID || InterlacedState == InterlacedState::NONE)))
			return NTV2_FORMAT_UNKNOWN;
		if (IsInput)
		{
			if (!IsSingleLink)
				if (Device->CanMakeQuadInputFromChannel(Channel))
				{
					auto fmt = Device->GetSDIInputVideoFormat(Channel);
					if(ForceInterlaced)
						return Device->ForceInterlace(fmt);
					return fmt;
				}
			if (Device->ChannelCanInput(Channel))
			{
				auto fmt = Device->GetSDIInputVideoFormat(Channel);
				if(ForceInterlaced)
					return Device->ForceInterlace(fmt);
				return fmt;
			}
			return NTV2_FORMAT_UNKNOWN;
		}
		for (int i = 0; i < NTV2_MAX_NUM_VIDEO_FORMATS; ++i)
		{
			NTV2VideoFormat format = NTV2VideoFormat(i);
			if (GetNTV2FrameGeometryFromVideoFormat(format) == Resolution &&
				GetNTV2FrameRateFromVideoFormat(format) == FrameRate &&
				(InterlacedState != InterlacedState::NONE && IsProgressiveTransport(format) == (InterlacedState == InterlacedState::PROGRESSIVE)) &&
				Device->CanChannelDoFormat(Channel, IsInput, format, GetEffectiveQuadMode()))
				return format;
		}
		return NTV2_FORMAT_UNKNOWN;
	}

	std::pair<NTV2Channel, AJADevice::Mode> GetChannelFromString(const std::string& str)
	{
		for (uint32_t i = NTV2_CHANNEL1; i < NTV2_MAX_NUM_CHANNELS; ++i)
		{
			AJADevice::Mode modes[2] = {AJADevice::SL, AJADevice::AUTO};
			for (auto mode : modes)
			{
				if (GetChannelName(NTV2Channel(i), mode) == str)
					return {NTV2Channel(i), mode};
			}
		}
		return {NTV2_CHANNEL_INVALID, AJADevice::SL};
	}

	NTV2FrameGeometry GetNTV2FrameGeometryFromString(const std::string& str)
	{
		for (int i = 0; i < NTV2_FG_NUMFRAMEGEOMETRIES; ++i)
		{
			if (NTV2FrameGeometryToString(NTV2FrameGeometry(i), true) == str)
				return NTV2FrameGeometry(i);
		}
		return NTV2_FG_INVALID;
	}

	NTV2FrameRate GetNTV2FrameRateFromString(const std::string& str)
	{
		for (int i = 0; i < NTV2_NUM_FRAMERATES; ++i)
		{
			if (NTV2FrameRateToString(NTV2FrameRate(i), true) == str)
				return NTV2FrameRate(i);
		}
		return NTV2_FRAMERATE_INVALID;
	}

	AJADevice::Mode GetEffectiveQuadMode()
	{
		if(IsSingleLink)
			return AJADevice::SL;
		return IsInput ? InputModePin : OutputModePin;
	}

	std::atomic_bool TryFindChannel = false;

	nosResult ExecuteNode(nosNodeExecuteParams* execParams) override
	{
		execParams->MarkAllOutsDirty = false;
		return CurrentChannel.IsOpen ? NOS_RESULT_SUCCESS : NOS_RESULT_FAILED;
	}

	static nosResult GetFunctions(size_t* outCount, nosName* outFunctionNames, nosPfnNodeFunctionExecute* outFunction)
	{
		*outCount = 4;
		if (!outFunctionNames || !outFunction)
			return NOS_RESULT_SUCCESS;
		outFunctionNames[0] = NOS_NAME_STATIC("TryUpdateChannel");
		outFunction[0] = [](void* ctx, nosFunctionExecuteParams* params)
			{
				auto* context = static_cast<ChannelNodeContext*>(ctx);
				context->TryFindChannel = true;
				nosEngine.SendPathRestart(context->NodeId);
				nosEngine.LogW("Input signal lost.");
				return NOS_RESULT_SUCCESS;
			};

		outFunctionNames[1] = NOS_NAME_STATIC("CheckChannelConfig");
		outFunction[1] = [](void* ctx, nosFunctionExecuteParams* params)
			{
				auto* context = static_cast<ChannelNodeContext*>(ctx);
				context->CheckChannelConfig();
				return NOS_RESULT_SUCCESS;
			};

		outFunctionNames[2] = NOS_NAME_STATIC("CheckChannelStatus");
		outFunction[2] = [](void* ctx, nosFunctionExecuteParams* params)
			{
				auto* context = static_cast<ChannelNodeContext*>(ctx);
				context->CheckChannelStatus();
				return NOS_RESULT_SUCCESS;
			};

		outFunctionNames[3] = NOS_NAME_STATIC("Drop");
		outFunction[3] = [](void* ctx, nosFunctionExecuteParams* params)
			{
				auto* context = static_cast<ChannelNodeContext*>(ctx);
				context->CurrentChannel.IncrementDropCount();
				return NOS_RESULT_SUCCESS;
			};
		
		return NOS_RESULT_SUCCESS;
	}

	Channel CurrentChannel;

	std::optional<uuid> QuadLinkModePinId = std::nullopt;
	bool ShouldOpen = false;
	bool IsInput = false;
	bool ForceInterlaced = false;
	sys::device::TDeviceInfo DevicePinValue;
	std::string ChannelPinValue = PIN_VALUE_NONE;
	std::string ResolutionPinValue = PIN_VALUE_NONE;
	std::string FrameRatePinValue = PIN_VALUE_NONE;
	std::string InterlacedPinValue = PIN_VALUE_NONE;
	std::string ReferenceSourcePinValue = PIN_VALUE_NONE;

	AJADevice* Device{};
	NTV2Channel Channel = NTV2_CHANNEL_INVALID;
	NTV2FrameGeometry Resolution = NTV2_FG_INVALID;
	NTV2FrameRate FrameRate = NTV2_FRAMERATE_INVALID;
	enum class InterlacedState
	{
		NONE,
		INTERLACED,
		PROGRESSIVE
	} InterlacedState = InterlacedState::NONE;
	NTV2ReferenceSource ReferenceSource = NTV2_REFERENCE_INVALID;
	AJADevice::Mode InputModePin = AJADevice::SL, OutputModePin = AJADevice::SL;
	bool IsSingleLink = true;

	QuadLinkInputMode QuadLinkInputMode = QuadLinkInputMode::Tsi;
	QuadLinkMode QuadLinkOutputMode = QuadLinkMode::Tsi;
	
	bool DeltaSecondCompatible = true;

	bool DeviceAcquired = false;
	bool ShouldAcquireDevice = true;
};

nosResult RegisterChannelNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.Channel"), ChannelNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}
