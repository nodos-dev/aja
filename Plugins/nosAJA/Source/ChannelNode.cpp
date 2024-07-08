// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include "Channels.h"
// TODO: Remove this node once things settle down.
namespace nos::aja
{
NOS_REGISTER_NAME(ReferenceSource);
NOS_REGISTER_NAME(Device);
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
NOS_REGISTER_NAME(QuadMode);

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
	ChannelNodeContext(const nosFbNode* node) : NodeContext(node), CurrentChannel(this)
	{
		AJADevice::Init();

		if (auto* pins = node->pins())
		{
			TChannelInfo info;
			for (auto const* pin : *pins)
			{
				auto name = pin->name()->c_str();
				if (0 == strcmp(name, "Channel"))
				{
					CurrentChannel.ChannelPinId = *pin->id();
					flatbuffers::GetRoot<ChannelInfo>(pin->data()->data())->UnPackTo(&info);
				}
			}
			CurrentChannel.Update(std::move(info), false);
		}

		UpdateStringList(GetReferenceStringListName(), {"NONE"});
		UpdateStringList(GetDeviceStringListName(), {"NONE"});
		UpdateStringList(GetChannelStringListName(), {"NONE"});
		UpdateStringList(GetResolutionStringListName(), {"NONE"});
		UpdateStringList(GetFrameRateStringListName(), {"NONE"});
		UpdateStringList(GetInterlacedStringListName(), {"NONE"});
		
		SetPinVisualizer(NSN_ReferenceSource, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetReferenceStringListName()});
		SetPinVisualizer(NSN_Device, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetDeviceStringListName()});
		SetPinVisualizer(NSN_Device, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetDeviceStringListName()});
		SetPinVisualizer(NSN_ChannelName, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetChannelStringListName()});
		SetPinVisualizer(NSN_Resolution, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetResolutionStringListName()});
		SetPinVisualizer(NSN_FrameRate, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetFrameRateStringListName()});
		SetPinVisualizer(NSN_IsInterlaced, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = GetInterlacedStringListName()});

		AddPinValueWatcher(NSN_IsOpen, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			IsOpen = *InterpretPinValue<bool>(newVal);
			TryUpdateChannel();
		});
		AddPinValueWatcher(NSN_IsInput, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			IsInput = *InterpretPinValue<bool>(newVal);
			if (oldValue)
				ResetAfter(AJAChangedPinType::IsInput);
			UpdateAfter(AJAChangedPinType::IsInput, !oldValue);
		});
		AddPinValueWatcher(NSN_Device, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			DevicePin = InterpretPinValue<const char>(newVal);
			Device = AJADevice::GetDevice(DevicePin).get();
			if (DevicePin != "NONE" && !Device)
				SetPinValue(NSN_Device, nosBuffer{.Data = (void*)"NONE", .Size = 5});
			else
			{
				if (oldValue)
					ResetAfter(AJAChangedPinType::Device);
				else if (DevicePin == "NONE")
					AutoSelectIfSingle(NSN_Device, GetPossibleDeviceNames());
			}
			UpdateAfter(AJAChangedPinType::Device, !oldValue);
		});
		AddPinValueWatcher(NSN_ChannelName, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			ChannelPin = InterpretPinValue<const char>(newVal);
			auto [channel, mode] = GetChannelFromString(ChannelPin);
			Channel = channel;
			Mode = mode;
			if (ChannelPin != "NONE" && Channel == NTV2_CHANNEL_INVALID)
				SetPinValue(NSN_ChannelName, nosBuffer{.Data = (void*)"NONE", .Size = 5});
			else
			{
				if (oldValue)
					ResetAfter(AJAChangedPinType::ChannelName);
				else if (ChannelPin == "NONE")
					AutoSelectIfSingle(NSN_ChannelName, GetPossibleChannelNames());
			}
			UpdateAfter(AJAChangedPinType::ChannelName, !oldValue);
		});
		AddPinValueWatcher(NSN_Resolution, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			ResolutionPin = InterpretPinValue<const char>(newVal);
			Resolution = GetNTV2FrameGeometryFromString(ResolutionPin);
			if (ResolutionPin != "NONE" && Resolution == NTV2_FG_INVALID)
				SetPinValue(NSN_Resolution, nosBuffer{.Data = (void*)"NONE", .Size = 5});
			else
			{
				if (oldValue)
					ResetAfter(AJAChangedPinType::Resolution);
				else if (ResolutionPin == "NONE")
					AutoSelectIfSingle(NSN_Resolution, GetPossibleResolutions());
			}
			UpdateAfter(AJAChangedPinType::Resolution, !oldValue);
		});
		AddPinValueWatcher(NSN_FrameRate, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			FrameRatePin = InterpretPinValue<const char>(newVal);
			FrameRate = GetNTV2FrameRateFromString(FrameRatePin);
			if (FrameRatePin != "NONE" && FrameRate == NTV2_FRAMERATE_INVALID)
				SetPinValue(NSN_FrameRate, nosBuffer{.Data = (void*)"NONE", .Size = 5});
			else
			{
				if (oldValue)
					ResetAfter(AJAChangedPinType::FrameRate);
				else if (FrameRatePin == "NONE")
					AutoSelectIfSingle(NSN_FrameRate, GetPossibleFrameRates());
			}
			UpdateAfter(AJAChangedPinType::FrameRate, !oldValue);
		});
		AddPinValueWatcher(NSN_IsInterlaced, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			std::string interlaced = InterpretPinValue<const char>(newVal);
			InterlacedState = InterlacedState::NONE;
			if (interlaced == "NONE")
				InterlacedState = InterlacedState::NONE;
			else if (interlaced == "Interlaced")
				InterlacedState = InterlacedState::INTERLACED;
			else if (interlaced == "Progressive")
				InterlacedState = InterlacedState::PROGRESSIVE;

			if (interlaced != "NONE" && InterlacedState == InterlacedState::NONE)
				SetPinValue(NSN_IsInterlaced, nosBuffer{.Data = (void*)"NONE", .Size = 5});

			if (!oldValue && interlaced == "NONE")
				AutoSelectIfSingle(NSN_IsInterlaced, GetPossibleInterlaced());

			TryUpdateChannel();
		});
		AddPinValueWatcher(NSN_FrameBufferFormat, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			CurrentPixelFormat = *InterpretPinValue<mediaio::YCbCrPixelFormat>(newVal);
			TryUpdateChannel();
		});
		AddPinValueWatcher(NSN_ReferenceSource, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			ReferenceSource = InterpretPinValue<const char>(newVal);
			TryUpdateChannel();
		});
		AddPinValueWatcher(NSN_QuadMode, [this](const nos::Buffer& newVal, std::optional<nos::Buffer> oldValue) {
			auto newQuadMode = *InterpretPinValue<AJADevice::Mode>(newVal);
			auto info = CurrentChannel.Info;
			info.output_quad_link_mode = static_cast<QuadLinkMode>(newQuadMode);
			CurrentChannel.Update(std::move(info), true);
		});
	}

	~ChannelNodeContext() override
	{
		CurrentChannel.Close();
	}
	
	mediaio::YCbCrPixelFormat CurrentPixelFormat = mediaio::YCbCrPixelFormat::YUV8;
	std::string ReferenceSource = "NONE";

	void UpdateReferenceSource()
	{
		if (IsInput)
			return;
		auto src = NTV2_REFERENCE_INVALID;
		if (ReferenceSource.empty())
			nosEngine.LogE("Empty value received for reference pin!");
		else if (std::string::npos != ReferenceSource.find("Reference In"))
			src = NTV2_REFERENCE_EXTERNAL;
		else if (std::string::npos != ReferenceSource.find("Free Run"))
			src = NTV2_REFERENCE_FREERUN;
		else if(auto pos = ReferenceSource.find("SDI In"); std::string::npos != pos)
			src = AJADevice::ChannelToRefSrc(NTV2Channel(ReferenceSource[pos + 7] - '1'));
		if (src != NTV2_REFERENCE_INVALID)
			Device->SetReference(src);
	}
	
	void TryUpdateChannel() 
	{ 
		if (!IsOpen)
		{
			CurrentChannel.Update({}, true);
			return;
		}
		CurrentChannel.Update({}, true);
		auto format = GetVideoFormat();
		if (format == NTV2_FORMAT_UNKNOWN)
		{
			CurrentChannel.Update({}, true);
			return;
		}
		TChannelInfo channelPin{}; 
		channelPin.device = std::make_unique<TDevice>(TDevice{{}, Device->GetSerialNumber(), Device->GetDisplayName()});
		channelPin.channel_name = ChannelPin;
		channelPin.is_input = IsInput;
		channelPin.is_quad = AJADevice::IsQuad(Mode);
		channelPin.video_format = NTV2VideoFormatToString(format, true); // TODO: Readonly.
		uint32_t width, height;
		Device->GetExtent(Channel, Mode, width, height);
		channelPin.resolution = std::make_unique<nos::fb::vec2u>(width, height);
		channelPin.video_format_idx = static_cast<int>(format);
		if (AJADevice::IsQuad(Mode))
		{
			if(IsInput)
				channelPin.input_quad_link_mode = static_cast<nos::aja::QuadLinkInputMode>(Mode);
			else 
				channelPin.output_quad_link_mode = static_cast<QuadLinkMode>(Mode);
		}
		channelPin.frame_buffer_format = static_cast<mediaio::YCbCrPixelFormat>(CurrentPixelFormat);
		channelPin.is_interlaced = !IsProgressivePicture(format);
 		CurrentChannel.Update(std::move(channelPin), true);
		UpdateReferenceSource();
	}

	void UpdateVisualizer(nos::Name pinName, std::string stringListName)
	{ 
		SetPinVisualizer(pinName, {.type = nos::fb::VisualizerType::COMBO_BOX, .name = stringListName});
	}

	void AutoSelectIfSingle(nosName pinName, std::vector<std::string> const& list)
	{
		if (list.size() == 2)
			SetPinValue(pinName, nosBuffer{.Data = (void*)list[1].c_str(), .Size = list[1].size() + 1});
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

			auto deviceList = GetPossibleDeviceNames();
			UpdateStringList(GetDeviceStringListName(), deviceList);

			if (!first)
				AutoSelectIfSingle(NSN_Device, deviceList);
			break;
		}
		case AJAChangedPinType::Device: {
			auto channelList = GetPossibleChannelNames();
			UpdateStringList(GetChannelStringListName(), channelList);
			if (!first)
				AutoSelectIfSingle(NSN_ChannelName, channelList);
			if (IsInput || !Device)
			{
				UpdateStringList(GetReferenceStringListName(), { "NONE" });
				SetPinValue(NSN_ReferenceSource, nosBuffer{ .Data = (void*)"NONE", .Size = 5 });
			}
			else
			{
				std::vector<std::string> list{"Reference In", "Free Run"};
				for (int i = 1; i <= NTV2DeviceGetNumVideoInputs(Device->ID); ++i)
					list.push_back("SDI In " + std::to_string(i));
				UpdateStringList(GetReferenceStringListName(), list);
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
			pinToSet = NSN_Device;
			break;
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
		SetPinValue(pinToSet, nosBuffer{.Data = (void*)"NONE", .Size = 5});
	}

	void ResetDevicePin()
	{ 
		SetPinValue(NSN_Device, nosBuffer{.Data = (void*)"NONE", .Size = 5});
	}
	void ResetChannelNamePin()
	{
		SetPinValue(NSN_ChannelName, nosBuffer{.Data = (void*)"NONE", .Size = 5});
	}
	void ResetResolutionPin()
	{
		SetPinValue(NSN_Resolution, nosBuffer{.Data = (void*)"NONE", .Size = 5});
	}
	void ResetFrameRatePin()
	{ 
		SetPinValue(NSN_FrameRate, nosBuffer{.Data = (void*)"NONE", .Size = 5});
	}
	void ResetInterlacedPin()
	{ 
		SetPinValue(NSN_IsInterlaced, nosBuffer{.Data = (void*)"NONE", .Size = 5});
	}

	std::string GetReferenceStringListName() { return "aja.ReferenceSource." + UUID2STR(NodeId); }
	std::string GetDeviceStringListName() { return "aja.DeviceList." + UUID2STR(NodeId); }
	std::string GetChannelStringListName() { return "aja.ChannelList." + UUID2STR(NodeId); }
	std::string GetResolutionStringListName() { return "aja.ResolutionList." + UUID2STR(NodeId); }
	std::string GetFrameRateStringListName() { return "aja.FrameRateList." + UUID2STR(NodeId); }
	std::string GetInterlacedStringListName() { return "aja.InterlacedList." + UUID2STR(NodeId); }

	std::vector<std::string> GetPossibleDeviceNames()
	{
		std::vector<std::string> devices = {"NONE"};
		for (auto& [name, serial] : AJADevice::EnumerateDevices())
		{
			devices.push_back(name);
		}
		return devices;
	}

	std::vector<std::string> GetPossibleChannelNames() 
	{
		std::vector<std::string> channels = {"NONE"};
		if (!Device)
			return channels;
		for (u32 i = NTV2_CHANNEL1; i < NTV2_MAX_NUM_CHANNELS; ++i)
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
			return {"NONE"};
		std::set<NTV2FrameGeometry> resolutions;
		for (int i = 0; i < NTV2_MAX_NUM_VIDEO_FORMATS; ++i)
		{
			NTV2VideoFormat format = NTV2VideoFormat(i);
			if (Device->CanChannelDoFormat(Channel, IsInput, format, Mode))
			{
				resolutions.insert(GetNTV2FrameGeometryFromVideoFormat(format));
			}
		}
		std::vector<std::string> possibleResolutions = {"NONE"};
		for (auto res : resolutions)
		{
			possibleResolutions.push_back(NTV2FrameGeometryToString(res, true));
		}
		return possibleResolutions;
	}
	std::vector<std::string> GetPossibleFrameRates() 
	{
		if (Resolution == NTV2_FG_INVALID)
			return {"NONE"};
		std::set<NTV2FrameRate> frameRates;
		for (int i = 0; i < NTV2_MAX_NUM_VIDEO_FORMATS; ++i)
		{
			if (GetNTV2FrameGeometryFromVideoFormat(NTV2VideoFormat(i)) != Resolution)
				continue;
				NTV2VideoFormat format = NTV2VideoFormat(i);
			if (Device->CanChannelDoFormat(Channel, IsInput, format, Mode))
			{
				frameRates.insert(GetNTV2FrameRateFromVideoFormat(format));
			}
		}
		std::vector<std::string> possibleFrameRates = {"NONE"};
		for (auto rate : frameRates)
		{
			possibleFrameRates.push_back(NTV2FrameRateToString(rate));
		}
		return possibleFrameRates;
	}
	std::vector<std::string> GetPossibleInterlaced()
	{
		if (FrameRate == NTV2_FRAMERATE_INVALID)
			return {"NONE"};
		std::set<bool> interlaced;
		for (int i = 0; i < NTV2_MAX_NUM_VIDEO_FORMATS; ++i)
		{
			NTV2VideoFormat format = NTV2VideoFormat(i);
			if (GetNTV2FrameGeometryFromVideoFormat(format) != Resolution)
				continue;
			if (GetNTV2FrameRateFromVideoFormat(format) != FrameRate)
				continue;
			if (Device->CanChannelDoFormat(Channel, IsInput, format, Mode))
			{
				interlaced.insert(!IsProgressiveTransport(format));
			}
		}
		std::vector<std::string> possibleInterlaced = {"NONE"};
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
			if (AJADevice::IsQuad(Mode))
				if (Device->CanMakeQuadInputFromChannel(Channel))
					return Device->GetSDIInputVideoFormat(Channel);
			if (Device->ChannelCanInput(Channel))
				return Device->GetSDIInputVideoFormat(Channel);
			return NTV2_FORMAT_UNKNOWN;
		}
		for (int i = 0; i < NTV2_MAX_NUM_VIDEO_FORMATS; ++i)
		{
			NTV2VideoFormat format = NTV2VideoFormat(i);
			if (GetNTV2FrameGeometryFromVideoFormat(format) == Resolution &&
				GetNTV2FrameRateFromVideoFormat(format) == FrameRate &&
				(InterlacedState != InterlacedState::NONE && IsProgressiveTransport(format) == (InterlacedState == InterlacedState::PROGRESSIVE)) &&
				Device->CanChannelDoFormat(Channel, IsInput, format, Mode))
				return format;
		}
		return NTV2_FORMAT_UNKNOWN;
	}

	std::pair<NTV2Channel, AJADevice::Mode> GetChannelFromString(const std::string& str)
	{
		for (u32 i = NTV2_CHANNEL1; i < NTV2_MAX_NUM_CHANNELS; ++i)
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
			if (NTV2FrameRateToString(NTV2FrameRate(i)) == str)
				return NTV2FrameRate(i);
		}
		return NTV2_FRAMERATE_INVALID;
	}

	std::atomic_bool TryFindChannel = false;

	nosResult ExecuteNode(const nosNodeExecuteArgs* execArgs) override
	{
		if (!TryFindChannel)
			return NOS_RESULT_SUCCESS;
		if (!Device)
			return NOS_RESULT_FAILED;
		TryUpdateChannel();
		if (Device->GetSDIInputVideoFormat(Channel) != NTV2_FORMAT_UNKNOWN)
		{
			nosEngine.LogI("Input signal reconnected.");
			TryFindChannel = false;
			return NOS_RESULT_SUCCESS;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		return NOS_RESULT_PENDING;
	}

	static nosResult GetFunctions(size_t* outCount, nosName* outFunctionNames, nosPfnNodeFunctionExecute* outFunction) 
	{
		*outCount = 1;
		if (!outFunctionNames || !outFunction)
			return NOS_RESULT_SUCCESS;
		*outFunction = [](void* ctx, const nosNodeExecuteArgs* nodeArgs, const nosNodeExecuteArgs* functionArgs)
			{
				auto* context = static_cast<ChannelNodeContext*>(ctx);
				context->TryFindChannel = true;
				nosEngine.SendPathRestart(context->NodeId);
				nosEngine.LogW("Input signal lost.");
			};
		*outFunctionNames = NOS_NAME_STATIC("TryUpdateChannel");
		return NOS_RESULT_SUCCESS; 
	}

	Channel CurrentChannel;

	std::optional<nosUUID> QuadLinkModePinId = std::nullopt;
	bool IsOpen = false;
	bool IsInput = false;
	std::string DevicePin = "NONE";
	std::string ChannelPin = "NONE";
	std::string ResolutionPin = "NONE";
	std::string FrameRatePin = "NONE";
	std::string InterlacedPin = "NONE";

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
	AJADevice::Mode Mode = AJADevice::SL;

	QuadLinkInputMode QuadLinkInputMode = QuadLinkInputMode::Tsi;
	QuadLinkMode QuadLinkOutputMode = QuadLinkMode::Tsi;
};

nosResult RegisterChannelNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.Channel"), ChannelNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}
