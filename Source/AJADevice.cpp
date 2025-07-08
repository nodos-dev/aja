// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include "AJADevice.h"
#include "nosDefines.h"
#include "ntv2enums.h"
#include "ntv2signalrouter.h"
#include "ntv2utils.h"
#include <ntv2devicescanner.h>
#include <ranges>
#include <system/process.h>

#include "firmware.hpp"

#include <nosDeviceSubsystem/nosDeviceSubsystem.h>
#include <nosSettingsSubsystem/nosSettingsSubsystem.h>

#undef min
#undef max
#if !defined(_WIN32)
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))
#endif
std::map<std::string, uint64_t> AJADevice::AvailableDevices;

DeviceLock::DeviceLock(AJADevice* card) : Card(card)
{
    Acquired = Card->AcquireDevice();
}

DeviceLock::~DeviceLock()
{
    if (Acquired)
        Card->ReleaseDevice();
}

std::map<std::string, uint64_t> AJADevice::EnumerateDevices()
{
    CNTV2DeviceScanner scanner{};
    std::map<std::string, uint64_t>  re;
    CNTV2Card dev;
    for (ULWord i = 0; scanner.GetDeviceAtIndex(i, dev); i++)
    {
        re[dev.GetDisplayName()] = dev.GetSerialNumber();
    }
    return re;
}

std::unordered_map<std::string, std::set<NTV2VideoFormat>> AJADevice::StringToFormat()
{
    std::unordered_map<std::string, std::set<NTV2VideoFormat>> re;
    for (uint32_t i = 0; i < NTV2_MAX_NUM_VIDEO_FORMATS; ++i)
    {
        re[NTV2VideoFormatToString(NTV2VideoFormat(i), true)].insert(NTV2VideoFormat(i));
    }
    return re;
}

uint64_t AJADevice::FindDeviceSerial(const char* ident)
{
    if(auto it = AvailableDevices.find(ident); it != AvailableDevices.end())
    {
        return it->second;
    }
    
    return 0;
}

bool AJADevice::DeviceAvailable(const char* ident, bool input)
{
    if(FindDeviceSerial(ident))
    {
        auto dev = GetDevice(ident);
        return !dev || (input ? !dev->HasInput : !dev->HasOutput);
    }
    return false;
}

bool AJADevice::GetAvailableDevice(bool input, AJADevice** pOut)
{
    if(AvailableDevices.empty()) return false;
    if(Devices.empty()) return true;
    for(auto& [_, dev] : Devices)
        if(input ? !dev->HasInput : !dev->HasOutput)
        {
            if(pOut) *pOut = dev.get();
            return true;
        }
    return false;
}

void AJADevice::Init()
{
    if(AvailableDevices.empty() || !Devices.empty()) 
    {
        return;
    }
    
    CNTV2DeviceScanner scanner;
    CNTV2Card dev;
    for (ULWord i = 0; scanner.GetDeviceAtIndex(i, dev); i++)
        Devices[dev.GetSerialNumber()] = (std::make_shared<AJADevice>(dev.GetSerialNumber())); // TODO: Error check on AJADevice ctor.
    for (auto& device : Devices)
        device.second->RegisterSettings();
}

void AJADevice::Deinit()
{
    for(auto& [_, dev] : Devices)
    {
        if(dev->HasInput || dev->HasOutput)
        {
            return;
        }
    }
    Devices.clear();
}

std::shared_ptr<AJADevice> AJADevice::GetDevice(std::string_view const& name)
{
    for(auto& [_, dev]: Devices)
    {
        if(name == dev->GetDisplayName())
        {
            return dev;
        }
    }
    return 0;
}

std::shared_ptr<AJADevice> AJADevice::GetDevice(uint32_t index)
{
	for(auto& [_, dev]: Devices)
	{
		if(index == dev->GetIndexNumber())
		{
			return dev;
		}
	}
	return 0;
}

std::shared_ptr<AJADevice> AJADevice::GetDeviceBySerialNumber(uint64_t serial)
{
	auto it = Devices.find(serial);
	if (it != Devices.end())
		return it->second;
	return nullptr;
}

CNTV2VPID AJADevice::GetVPID(NTV2Channel channel, CNTV2VPID* B)
{
    ULWord a, b;
    ReadSDIInVPID(channel, a, b);
    if (B) *B = CNTV2VPID(b);
    return CNTV2VPID(a);
}

NTV2VideoFormat AJADevice::ForceInterlace(NTV2VideoFormat format)
{
	if (NTV2_VIDEO_FORMAT_HAS_PROGRESSIVE_PICTURE(format))
	{
		auto frameRate = GetNTV2FrameRateFromVideoFormat(format);
		switch (frameRate)
		{
			case NTV2_FRAMERATE_6000:	return NTV2_FORMAT_1080i_6000;
			case NTV2_FRAMERATE_5994:	return NTV2_FORMAT_1080i_5994;
			case NTV2_FRAMERATE_5000:	return NTV2_FORMAT_1080i_5000;
			case NTV2_FRAMERATE_3000:	return NTV2_FORMAT_1080i_6000;
			case NTV2_FRAMERATE_2997:	return NTV2_FORMAT_1080i_5994;
			case NTV2_FRAMERATE_2500:	return NTV2_FORMAT_1080i_5000;
			default:					return format;
		}
	}
	return format;
}

NTV2VideoFormat AJADevice::GetInputVideoFormat(NTV2Channel channel)
{
    NTV2VideoFormat fmt = GetSDIInputVideoFormat(channel, GetSDIInputIsProgressive(channel));
    if (fmt == NTV2_FORMAT_UNKNOWN)
    {
        ULWord a, b;
        ReadSDIInVPID(channel, a, b);
        if (CNTV2Card::GetVPIDValidA(channel))
            fmt = CNTV2VPID(a).GetVideoFormat();
        else if (CNTV2Card::GetVPIDValidB(channel))
            fmt = CNTV2VPID(b).GetVideoFormat();
    }
    if (fmt == NTV2_FORMAT_UNKNOWN) this->GetVideoFormat(fmt, channel);
    return GetSupportedNTV2VideoFormatFromInputVideoFormat(fmt);
}

bool AJADevice::IsTSI(NTV2Channel channel)
{
    ULWord a, b;
    ReadSDIInVPID(channel, a, b);
    return CNTV2VPID(a).IsStandardTwoSampleInterleave();
}

AJADevice::Mode AJADevice::GetMode(NTV2Channel channel)
{
    return IsTSI(channel) ? TSI : CNTV2VPID::VPIDStandardIsQuadLink(GetVPID(channel).GetStandard()) ? SQD : SL;
}

void AJADevice::ClearState()
{
    CNTV2Card::ClearRouting();
    for (int i = 0; i < 8; ++i)
    {
        auto channel = NTV2Channel(i);
        UnsubscribeInputVerticalEvent(channel);
        UnsubscribeOutputVerticalEvent(channel);
        DisableInputInterrupt(channel);
        DisableOutputInterrupt(channel);
        DisableChannel(channel);
        SetSDITransmitEnable(channel, false);
        SetMode(channel, NTV2_MODE_INVALID);
    }
    SetReference(NTV2_REFERENCE_EXTERNAL);
}

uint32_t AJADevice::GetFBSize(NTV2Channel channel)
{
    NTV2Framesize fsz = NTV2_FRAMESIZE_INVALID;
    bool quad = false;
    GetFrameBufferSize(channel, fsz);
    GetQuadFrameEnable(quad, channel);
    bool quadquad = false;
    GetQuadQuadFrameEnable(quadquad, channel);
    return NTV2FramesizeToByteCount(fsz) * (quad ? 4 : 1) * (quadquad ? 4 : 1);
}

AJADevice::~AJADevice()
{
	DeviceLock lock(this);
    nosDevice->UnregisterDevice(GlobalDeviceId);
    ClearState();
    Close();
}

static constexpr char REFERENCE_ENTRY_EDITOR_ITEM_NAME[] = "Out Reference";
NOS_REGISTER_NAME(Reference);
NOS_REGISTER_NAME(string);

std::string GetReferenceStringListName(uint64_t serialNumber) { return "aja.ReferenceSource." + std::to_string(serialNumber); }

static constexpr const char* REFERENCE_DEFAULTS[2] = { "Reference In", "Free Run" };

nosResult AJADevice::UpdateSettingsCallback(const char* entryName, nosBuffer itemValue) {
    if (!nos::Name(entryName).AsString().starts_with(NSN_Reference))
        return NOS_RESULT_FAILED;

    std::string serialNumberStr = nos::Name(entryName).AsCStr() + NSN_Reference.AsString().length() + 1;
    uint64_t serialNum = std::stoull(serialNumberStr);
   
    auto device = Devices.find(serialNum);
    if (device == Devices.end())
        return NOS_RESULT_FAILED;

    device->second->UpdateReferenceSource(nos::InterpretPinValue<const char>(itemValue), false);
    return NOS_RESULT_SUCCESS;
}

void AJADevice::RegisterSettings() {
    nosSettingsEntryParams params{};
    std::string defaultReference = REFERENCE_DEFAULTS[0];
    nosBuffer defaultReferenceVal = { .Data = &defaultReference[0], .Size = defaultReference.length() + 1 };
    nos::fb::TVisualizer visualizer = { .type = nos::fb::VisualizerType::COMBO_BOX, .name = GetReferenceStringListName(GetSerialNumber()) };
    nos::sys::settings::RegisterEntry(
        NSN_Reference.AsString() + "\\" + std::to_string(GetSerialNumber()),
        NSN_string.AsCStr(),
        UpdateSettingsCallback,
        defaultReferenceVal,
        REFERENCE_ENTRY_EDITOR_ITEM_NAME,
        std::string(NOS_DEVICE_SUBSYSTEM_NAME) + "\\" + std::to_string(GetSerialNumber()),
        visualizer
    );
    UpdateReferenceStringList();
}

AJADevice::AJADevice(uint64_t serial)
{
    AJAStatus	status	(AJA_STATUS_SUCCESS);

    //	Open the device...
    if (!CNTV2DeviceScanner::GetDeviceWithSerial (serial, *this))
    {
        nosEngine.LogE("## ERROR:  Device '%ull' not found", serial);
        return;
    }

    if (!IsDeviceReady(false))
    {
        nosEngine.LogE("## ERROR:  Device '%ull' not ready", serial);
        return;
    }

    ID =  GetDeviceID();

    if (!::NTV2DeviceCanDoCapture(ID))
    {
        nosEngine.LogE("## ERROR:  Device '%ull' cannot capture", serial);
        return;
    }
    AJA_ASSERT(SetEveryFrameServices(NTV2_OEM_TASKS));			//	Since this is an OEM demo, use the OEM service level
    AJA_ASSERT(SetMultiFormatMode(true));
    AJA_ASSERT(SetReference(NTV2_REFERENCE_EXTERNAL));

    ClearState();
    std::string firmwareMsg, firmwareMsgDetails;
    nosDeviceProperty driverProp{};
	bool isFirmwareValid = true;
    if (!CheckFirmware(firmwareMsg, firmwareMsgDetails)) {
        std::string driverPropMessage = firmwareMsg + "\n Details: " + firmwareMsgDetails;
        driverProp = {.Name = nos::Name("Firmware Info"), .Value = driverPropMessage.c_str()};
        isFirmwareValid = false;
    }
    // Register to device subsys
    nosRegisterDeviceParams params{
        .Device = {
            .VendorName = NSN_VendorName,
            .ModelName = nos::Name(GetModelName()),
            .TopologicalId = GetIndexNumber(),
            .SerialNumber = nos::Name(std::to_string(serial)),
            .Flags = nosDeviceFlags(NOS_DEVICE_FLAG_PCI | NOS_DEVICE_FLAG_VIDEO_IO)
        },
        .DisplayName = nos::Name(GetModelName()),
        .Handle = serial,
        .Properties = isFirmwareValid ? nullptr : &driverProp ,
        .PropertyCount = isFirmwareValid ? 0ull : 1ull
    };
    nosDevice->RegisterDevice(&params, &GlobalDeviceId);
}

bool AJADevice::ChannelIsValid(NTV2Channel channel, bool isInput, NTV2VideoFormat fmt, Mode mode)
{
	if (!CanChannelDoFormat(channel, isInput, fmt, mode))
		return false;
    
    bool (AJADevice::*Arr[2][2])(NTV2Channel) = {
        {&AJADevice::ChannelCanOutput, &AJADevice::CanMakeQuadOutputFromChannel},
        {&AJADevice::ChannelCanInput,  &AJADevice::CanMakeQuadInputFromChannel},
    };
    
    return (this->*Arr[isInput][SL != mode])(channel);
}

bool AJADevice::CanChannelDoFormat(NTV2Channel channel, bool isInput, NTV2VideoFormat fmt, Mode mode)
{
    if (isInput)
        return true;
    if ((NTV2_FRAMERATE_INVALID != FPSFamily) && (GetFrameRateFamily(GetNTV2FrameRateFromVideoFormat(fmt)) != GetFrameRateFamily(FPSFamily)))
        return false;

    if (mode == SL)
    {
        if ((NTV2_IS_QUAD_FRAME_FORMAT(fmt) || NTV2_IS_QUAD_QUAD_FORMAT(fmt)) && !(
            fmt >= NTV2_FORMAT_FIRST_UHD_TSI_DEF_FORMAT && fmt <= NTV2_FORMAT_END_4K_TSI_DEF_FORMATS))
            return false;
    }
    else
    {
        if (!(NTV2_IS_QUAD_FRAME_FORMAT(fmt) || NTV2_IS_QUAD_QUAD_FORMAT(fmt)))
			return false;
    }

    return NTV2DeviceCanDoVideoFormat(ID, fmt);
}

bool AJADevice::ChannelCanInput(NTV2Channel channel)
{
    {
        std::shared_lock lock(ChannelsMutex);
        if (Channels.contains(channel))
        {
            return false;
        }
    }
    NTV2InputSource src = NTV2ChannelToInputSource(channel, NTV2_INPUTSOURCES_SDI);
    // Validate channel
    if(!NTV2DeviceCanDoInputSource(ID, src)) return false;
    if(!NTV2_INPUT_SOURCE_IS_SDI(src)) return false;
    if(!NTV2_IS_VALID_CHANNEL(channel)) return false;
    if (!EnableChannel(channel)) return false;
    if (!SetSDITransmitEnable(channel, false)) return false;
    if (!SetMode(channel, NTV2_MODE_INPUT)) return false;

    if(!EnableInputInterrupt(channel)) {
        DisableChannel(channel);
        return false;
    }
    
    if (!SubscribeInputVerticalEvent(channel)) {
        DisableInputInterrupt(channel);
        DisableChannel(channel);
        return false;
    }

    // bool re = WaitForInputVerticalInterrupt(channel, 10);
    UnsubscribeInputVerticalEvent(channel);
    DisableInputInterrupt(channel);
    DisableChannel(channel);
    return true;
}

bool AJADevice::ChannelCanOutput(NTV2Channel channel)
{
    {
        std::shared_lock lock(ChannelsMutex);
        if (Channels.contains(channel))
        {
            return false;
        }
    }
    NTV2OutputDestination dst = NTV2ChannelToOutputDestination(channel);

    // Validate channel
    if(!NTV2DeviceCanDoOutputDestination(ID, dst)) return false;
    if(!NTV2_OUTPUT_DEST_IS_SDI(dst)) return false;
    if(!NTV2_IS_VALID_CHANNEL(channel)) return false;
    if(!EnableChannel(channel)) return false;
    if (!EnableOutputInterrupt(channel)) {
        DisableChannel(channel);
        return false;
    }

    if (!SubscribeOutputVerticalEvent(channel)) {
        DisableOutputInterrupt(channel);
        DisableChannel(channel);
        return false;
    }

    bool re = SetSDITransmitEnable(channel, true);
    re &= SetMode(channel, NTV2_MODE_OUTPUT);
    // re &= WaitForInputVerticalInterrupt(channel);
    UnsubscribeOutputVerticalEvent(channel);
    DisableOutputInterrupt(channel);
    DisableChannel(channel);
    return re;
}

bool AJADevice::CanMakeQuadInputFromChannel(NTV2Channel channel)
{
    if(channel & 3)
    {
        // Channel has to be multiple of 4
        return false;
    }
    const NTV2Channel channels[] = {
        NTV2Channel(channel + 0), NTV2Channel(channel + 1),
        NTV2Channel(channel + 2), NTV2Channel(channel + 3),
    };

    for(auto c : channels)
    {
        if(!ChannelCanInput(c)) 
        {
            return false;
        }
    }
    bool qf = false;
    GetQuadFrameEnable(qf, channel);
    if (!qf && !SetQuadFrameEnable(true, channel))
        return false;
    SetQuadFrameEnable(qf, channel);
    return true;
}

bool AJADevice::CanMakeQuadOutputFromChannel(NTV2Channel channel)
{
    const auto nfb = NTV2DeviceGetNumVideoOutputs(ID);
    if (nfb <= channel)
    {
        return false;
    }

    if(channel & 3)
    {
        // Channel has to be multiple of 4
        return false;
    }

    
    const NTV2Channel channels[] = {
        NTV2Channel(channel + 0), NTV2Channel(channel + 1),
        NTV2Channel(channel + 2), NTV2Channel(channel + 3),
    };

    for(auto c : channels)
    {
        if(!ChannelCanOutput(c)) 
        {
            return false;
        }
    }

    bool qf = false;
    GetQuadFrameEnable(qf, channel);
    if (!qf && !SetQuadFrameEnable(true, channel))
        return false;
    SetQuadFrameEnable(qf, channel);
    return true;
}

uint64_t AJADevice::GetLastInputVerticalInterruptTimestamp(NTV2Channel channel)
{
    VirtualRegisterNum loRegisterNum = kVRegTimeStampLastInput1VerticalLo;
    switch (channel)
    {
    case NTV2_CHANNEL1:
    case NTV2_CHANNEL2:
        loRegisterNum = VirtualRegisterNum(kVRegTimeStampLastInput1VerticalLo + (channel - NTV2_CHANNEL1) * 2);
		break;
    case NTV2_CHANNEL3:
    case NTV2_CHANNEL4:
    case NTV2_CHANNEL5:
    case NTV2_CHANNEL6:
    case NTV2_CHANNEL7:
    case NTV2_CHANNEL8:
        loRegisterNum = VirtualRegisterNum(kVRegTimeStampLastInput3VerticalLo + (channel - NTV2_CHANNEL3) * 2);
        break;
    default:
        break;
    }
    ULWord nanosecondsLo = 0;
	ULWord nanosecondsHi = 0;
	ReadRegister(loRegisterNum, nanosecondsLo);
	ReadRegister(VirtualRegisterNum(loRegisterNum+1), nanosecondsHi);
    return ((uint64_t(nanosecondsHi) << 32) | nanosecondsLo)*100;
}
uint64_t AJADevice::GetLastOutputVerticalInterruptTimestamp(NTV2Channel channel)
{
	VirtualRegisterNum loRegisterNum = kVRegTimeStampLastOutputVerticalLo;
	switch (channel)
	{
	case NTV2_CHANNEL1:
		loRegisterNum = VirtualRegisterNum(kVRegTimeStampLastOutputVerticalLo + (channel - NTV2_CHANNEL1) * 2);
		break;
	case NTV2_CHANNEL2:
	case NTV2_CHANNEL3:
	case NTV2_CHANNEL4:
	case NTV2_CHANNEL5:
	case NTV2_CHANNEL6:
	case NTV2_CHANNEL7:
	case NTV2_CHANNEL8:
		loRegisterNum = VirtualRegisterNum(kVRegTimeStampLastOutput2VerticalLo + (channel - NTV2_CHANNEL2) * 2);
		break;
	default: break;
	}
	ULWord nanosecondsLo = 0;
	ULWord nanosecondsHi = 0;
	ReadRegister(loRegisterNum, nanosecondsLo);
	ReadRegister(VirtualRegisterNum(loRegisterNum + 1), nanosecondsHi);
	return ((uint64_t(nanosecondsHi) << 32) | nanosecondsLo) * 100;
}

uint64_t AJADevice::GetLastVBLTimestamp(NTV2Channel channel, bool isInput)
{
    if (isInput)
        return GetLastInputVerticalInterruptTimestamp(channel);
    return GetLastOutputVerticalInterruptTimestamp(channel);
}

static bool GetTSIMUXPins(NTV2Channel channel, NTV2InputCrosspointID& in, NTV2OutputCrosspointID& out)
{
    switch(channel)
    {
        default: return false;
        case NTV2_CHANNEL1: in = NTV2_Xpt425Mux1AInput; out = NTV2_Xpt425Mux1AYUV; break;
        case NTV2_CHANNEL2: in = NTV2_Xpt425Mux1BInput; out = NTV2_Xpt425Mux1BYUV; break;
        case NTV2_CHANNEL3: in = NTV2_Xpt425Mux2AInput; out = NTV2_Xpt425Mux2AYUV; break;
        case NTV2_CHANNEL4: in = NTV2_Xpt425Mux2BInput; out = NTV2_Xpt425Mux2BYUV; break;
        case NTV2_CHANNEL5: in = NTV2_Xpt425Mux3AInput; out = NTV2_Xpt425Mux3AYUV; break;
        case NTV2_CHANNEL6: in = NTV2_Xpt425Mux3BInput; out = NTV2_Xpt425Mux3BYUV; break;
        case NTV2_CHANNEL7: in = NTV2_Xpt425Mux4AInput; out = NTV2_Xpt425Mux4AYUV; break;
        case NTV2_CHANNEL8: in = NTV2_Xpt425Mux4BInput; out = NTV2_Xpt425Mux4BYUV; break;
    }
    return true;
}

static NTV2InputCrosspointID GetInputTSIFB(NTV2Channel channel)
{
    switch(channel)
    {
        default: return NTV2_FIRST_INPUT_CROSSPOINT;
        case NTV2_CHANNEL1: return NTV2_XptFrameBuffer1Input;
        case NTV2_CHANNEL2: return NTV2_XptFrameBuffer1DS2Input;
        case NTV2_CHANNEL3: return NTV2_XptFrameBuffer2Input;
        case NTV2_CHANNEL4: return NTV2_XptFrameBuffer2DS2Input;
        case NTV2_CHANNEL5: return NTV2_XptFrameBuffer5Input;
        case NTV2_CHANNEL6: return NTV2_XptFrameBuffer5DS2Input;
        case NTV2_CHANNEL7: return NTV2_XptFrameBuffer6Input;
        case NTV2_CHANNEL8: return NTV2_XptFrameBuffer6DS2Input;
    }
}

static NTV2OutputCrosspointID GetOutputTSIFB(NTV2Channel channel)
{
    switch(channel)
    {
        default: return NTV2_FIRST_OUTPUT_CROSSPOINT;
        case NTV2_CHANNEL1: return NTV2_XptFrameBuffer1YUV;
        case NTV2_CHANNEL2: return NTV2_XptFrameBuffer1_DS2YUV;
        case NTV2_CHANNEL3: return NTV2_XptFrameBuffer2YUV;
        case NTV2_CHANNEL4: return NTV2_XptFrameBuffer2_DS2YUV;
        case NTV2_CHANNEL5: return NTV2_XptFrameBuffer5YUV;
        case NTV2_CHANNEL6: return NTV2_XptFrameBuffer5_DS2YUV;
        case NTV2_CHANNEL7: return NTV2_XptFrameBuffer6YUV;
        case NTV2_CHANNEL8: return NTV2_XptFrameBuffer6_DS2YUV;
    }
}

bool AJADevice::RouteQuadInputSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, Mode mode, NTV2FrameBufferFormat fbFmt)
{
    std::unique_lock lock(ChannelsMutex);

    if(channel & 3)
    {
        // Channel has to be multiple of 4
        return false;
    }

    const NTV2Channel channels[] = {
        NTV2Channel(channel + 0), NTV2Channel(channel + 1),
        NTV2Channel(channel + 2), NTV2Channel(channel + 3),
    };

    for(auto c : channels)
    {
        if(Channels.contains(c)) 
        {
            // Channels should not be already in use
            return false;
        }
    }

    const bool isTsi = IsTSI(channel);

    if (mode == AUTO)
    {
        mode = (isTsi ? TSI : SQD);
    }
    
    if (((mode == TSI) != isTsi) || ((mode == SQD) != !isTsi))
        nosEngine.LogE("Warning: Detected signal is %s but requested config is %s", isTsi ? "TSI" : "Squares", (mode == TSI) ? "TSI" : "Squares");;
    
    bool re = SetQuadFrameEnable(true, channel);

    for(int i = 0; i < ARRAYSIZE(channels); ++i)
    {
        NTV2VideoFormat fmt = GetInputVideoFormat(channels[i]);
        if (!NTV2_IS_QUAD_FRAME_FORMAT(fmt))
        {
            NTV2FrameRate fps;
            re &= GetFrameRate(fps, channels[i]);
            // GetFirstMatchingVideoFormat()
        }
  
        auto src = NTV2ChannelToInputSource(channels[i], NTV2_INPUTSOURCES_SDI);
        re &= EnableChannel(channels[i]);
        re &= EnableInputInterrupt(channels[i]);
        re &= SubscribeInputVerticalEvent(channels[i]);
        re &= SetSDITransmitEnable(channels[i], false);
        re &= SetEnableVANCData(false, false, channels[i]);
        re &= SetMode(channels[i], NTV2_MODE_INPUT);
        re &= SetVideoFormat(fmt, false, false, channels[i]);
        re &= SetFrameBufferFormat(channels[i], fbFmt);
        
        switch (mode)
        {
        case TSI:
            re &= Set4kSquaresEnable(false, channels[i]);
            re &= SetTsiFrameEnable(true, channels[i]);
            NTV2InputCrosspointID in;
            NTV2OutputCrosspointID out;
            re &= GetTSIMUXPins(channels[i], in, out);
            re &= Connect(GetInputTSIFB(channels[i]), out, true);
            re &= Connect(in, GetInputSourceOutputXpt(src), true);
            break;
        case SQD:
            re &= SetTsiFrameEnable(false, channels[i]);
            re &= Set4kSquaresEnable(true, channels[i]);
            re &= Connect(GetFrameBufferInputXptFromChannel(channels[i]), GetInputSourceOutputXpt(src));
            break;
        default:
            return false;
        }
    }
    
    if(re)
        for(auto c : channels)
            Channels[c] = true;
    return re;
}

bool AJADevice::RouteQuadOutputSignal(NTV2Channel channel, NTV2VideoFormat fmt, Mode mode, NTV2FrameBufferFormat fbFmt)
{
    std::unique_lock lock(ChannelsMutex);

    if(channel & 3)
    {
        // Channel has to be multiple of 4
        return false;
    }

    const NTV2Channel channels[] = {
        NTV2Channel(channel + 0), NTV2Channel(channel + 1),
        NTV2Channel(channel + 2), NTV2Channel(channel + 3),
    };

    for(auto c : channels)
    {
        if(Channels.contains(c)) 
        {
            // Channels should not be already in use
            return false;
        }
    }

    if (mode == AUTO)
    {
        mode = TSI;
    }

    bool re = SetQuadFrameEnable(true, channel);

    for(int i = 0; i < ARRAYSIZE(channels); ++i)
    {
        re &= (EnableChannel(channels[i]));
        re &= (EnableOutputInterrupt(channels[i]));
        re &= (SubscribeOutputVerticalEvent(channels[i]));
        re &= (SetSDIOutputStandard(channels[i], GetNTV2StandardFromVideoFormat(fmt)));
        re &= (SetSDITransmitEnable(channels[i], true));
        re &= (SetEnableVANCData(false, false, channels[i]));
        re &= (SetMode(channels[i], NTV2_MODE_OUTPUT));
        re &= (SetVideoFormat(fmt, false, false, channels[i]));
        re &= (SetFrameBufferFormat(channels[i], fbFmt));
        auto dst = NTV2ChannelToOutputDestination(channels[i]);
        
        switch (mode)
        {
        case TSI:
            re &= SetTsiFrameEnable(true, channels[i]);
            NTV2InputCrosspointID in;
            NTV2OutputCrosspointID out;
            re &= GetTSIMUXPins(channels[i], in, out);
            re &= (Connect(GetOutputDestInputXpt(dst), out));
            re &= (Connect(in, GetOutputTSIFB(channels[i])));
            break;
        case SQD:
            re &= Set4kSquaresEnable(true, channels[i]);
            re &= Connect(GetOutputDestInputXpt(dst), GetFrameBufferOutputXptFromChannel(channels[i]));
            break;
        default:
            return false;
        }
    }
    
    if(re)
        for(auto c : channels)
            Channels[c] = false;

    return re;
}

bool AJADevice::RouteSLInputSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, NTV2FrameBufferFormat fbFmt)
{
    std::unique_lock lock(ChannelsMutex);
    NTV2InputSource src = NTV2ChannelToInputSource(channel, NTV2_INPUTSOURCES_SDI);

    // Validate channel
    // AJA_ASSERT(ChannelCanInput(channel));
    bool re = true;
    
    re &= (EnableChannel(channel));
    re &= (EnableInputInterrupt(channel));
    re &= (SubscribeInputVerticalEvent(channel));
    re &= (SetSDITransmitEnable(channel, false));
    re &= (SetEnableVANCData(false, false, channel));
    re &= (SetMode(channel, NTV2_MODE_INPUT));
    NTV2VideoFormat effectiveFormat = videoFmt;
    if (NTV2_VIDEO_FORMAT_IS_B(videoFmt))
    {
        re &= SetSDIInLevelBtoLevelAConversion(channel, true);
        //Find the corresponding A format
        effectiveFormat = GetFirstMatchingVideoFormat(GetNTV2FrameRateFromVideoFormat(videoFmt), GetDisplayHeight(videoFmt), GetDisplayWidth(videoFmt), IsProgressiveTransport(videoFmt), IsPSF(videoFmt), false);
    }
    else
        re &= SetSDIInLevelBtoLevelAConversion(channel, false);
    re &= (SetVideoFormat(effectiveFormat, false, false, channel));
    re &= (SetFrameBufferFormat(channel, fbFmt));
    re &= (Connect(GetFrameBufferInputXptFromChannel(channel), GetInputSourceOutputXpt(src)));
    // re &= (SetReference(NTV2InputSourceToReferenceSource(src)));
    if (re) Channels[channel] = true;
    return re;
}

bool AJADevice::RouteSLOutputSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, NTV2FrameBufferFormat fbFmt)
{
    std::unique_lock lock(ChannelsMutex);
    NTV2OutputDestination dst = NTV2ChannelToOutputDestination(channel);
            
    // Validate channel
    // AJA_ASSERT(ChannelCanOutput(channel));
    bool re = true;
    re &= (EnableChannel(channel));
    re &= (EnableOutputInterrupt(channel));
    re &= (SubscribeOutputVerticalEvent(channel));
    re &= (SetSDIOutputStandard(channel, GetNTV2StandardFromVideoFormat(videoFmt)));
    re &= (SetSDITransmitEnable(channel, true));
    re &= (SetEnableVANCData(false, false, channel));
    re &= (SetMode(channel, NTV2_MODE_OUTPUT));
    re &= (SetVideoFormat(videoFmt, false, false, channel));
    re &= (SetFrameBufferFormat(channel, fbFmt));
    re &= (Connect(GetOutputDestInputXpt(dst), GetFrameBufferOutputXptFromChannel(channel), true));
    if(re) Channels[channel] = false;
    return re;
}	

void AJADevice::CloseChannel(NTV2Channel channel, bool isInput,  bool isQuad)
{
    std::unique_lock lock(ChannelsMutex);
    if (isQuad)
    {
        CloseQLChannel(NTV2Channel(channel + 0), isInput);
        CloseQLChannel(NTV2Channel(channel + 1), isInput);
        CloseQLChannel(NTV2Channel(channel + 2), isInput);
        CloseQLChannel(NTV2Channel(channel + 3), isInput);
    }
    else
    {
        CloseSLChannel(channel, isInput);
    }

    if(Channels.empty())
    {
        FPSFamily = NTV2_FRAMERATE_INVALID;
    }
    SendCheckConfigurationToNodes();
}

void AJADevice::CloseSLChannel(NTV2Channel channel, bool isInput)
{
    AJA_ASSERT(Disconnect(isInput ? GetFrameBufferInputXptFromChannel(channel) : GetOutputDestInputXpt(NTV2ChannelToOutputDestination(channel))));
    AJA_ASSERT(isInput ? UnsubscribeInputVerticalEvent(channel) : UnsubscribeOutputVerticalEvent(channel));
    AJA_ASSERT(isInput ? DisableInputInterrupt(channel) : DisableOutputInterrupt(channel));
    AJA_ASSERT(DisableChannel(channel));
    Channels.erase(channel);
}


void AJADevice::CloseQLChannel(NTV2Channel channel, bool isInput)
{
    SetTsiFrameEnable(false, channel);
    Set4kSquaresEnable(false, channel);
    NTV2InputCrosspointID in;
    NTV2OutputCrosspointID out;
    GetTSIMUXPins(channel, in, out);
    Disconnect(in);
    Disconnect(GetOutputDestInputXpt(NTV2ChannelToOutputDestination(channel)));
    Disconnect(GetInputTSIFB(channel));
    Disconnect(GetFrameBufferInputXptFromChannel(channel));
    AJA_ASSERT(isInput ? UnsubscribeInputVerticalEvent(channel) : UnsubscribeOutputVerticalEvent(channel));
    AJA_ASSERT(isInput ? DisableInputInterrupt(channel) : DisableOutputInterrupt(channel));
    AJA_ASSERT(DisableChannel(channel));
    Channels.erase(channel);
}

void AJADevice::SendCheckConfigurationToNodes()
{
    std::unique_lock lock(RegisteredNodesMutex);
    for (auto& id : RegisteredNodes)
        nosEngine.CallNodeFunction(id, NOS_NAME("CheckChannelConfig"));
}

bool AJADevice::GetExtent(NTV2Channel channel, Mode mode, uint32_t& width, uint32_t& height)
{
    return GetExtent(GetInputVideoFormat(channel), mode, width, height);
}

bool AJADevice::GetExtent(NTV2VideoFormat fmt, Mode mode, uint32_t& width, uint32_t& height)
{
    const NTV2FormatDescriptor fd (fmt, NTV2_FBF_8BIT_YCBCR);
    width  = fd.GetRasterWidth();
    height = fd.GetRasterHeight();

    // we do this because input is most likely quad squares
    // and vpid can't tell us if the channel is a part of a multilink
    if (IsQuad(mode) && !(NTV2_IS_QUAD_FRAME_FORMAT(fmt) || NTV2_IS_QUAD_QUAD_FORMAT(fmt)))
    {
        width  *= 2;
        height *= 2;
    }
    return true;
}

bool AJADevice::RouteSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, bool isInput, Mode mode, NTV2FrameBufferFormat fbFmt)
{
    if (isInput)
    {
        videoFmt = GetInputVideoFormat(channel);
        if (mode != SL && !NTV2_IS_QUAD_FRAME_FORMAT(videoFmt))
        {
            uint32_t w, h;
            GetExtent(channel, mode, w, h);
            videoFmt = GetFirstMatchingVideoFormat(GetNTV2FrameRateFromVideoFormat(videoFmt), h, w, false, false, false);
        }
    }

    if (isInput ? RouteInputSignal(channel, videoFmt, mode, fbFmt) : RouteOutputSignal(channel, videoFmt, mode, fbFmt))
    {
        if (NTV2_FRAMERATE_INVALID == FPSFamily || (isInput && (mode == Mode::SL && GetFilteredChannels(true).size() <= 1) || (mode != Mode::AUTO && GetFilteredChannels(true).size() <= 4)))
            FPSFamily = GetFrameRateFamily(GetNTV2FrameRateFromVideoFormat(videoFmt));
        SendCheckConfigurationToNodes();
        return true;
    }
    return false;
}

void AJADevice::GetReferenceAndFrameRate(NTV2ReferenceSource& reference, NTV2FrameRate& framerate)
{
    GetReference(reference);
    framerate = NTV2FrameRate::NTV2_FRAMERATE_UNKNOWN;
    switch (reference)
    {
    case NTV2_REFERENCE_EXTERNAL:   framerate = GetNTV2FrameRateFromVideoFormat(GetReferenceVideoFormat()); break;
    case NTV2_REFERENCE_INPUT1:     framerate = GetNTV2FrameRateFromVideoFormat(GetInputVideoFormat(NTV2_CHANNEL1)); break;
    case NTV2_REFERENCE_INPUT2:     framerate = GetNTV2FrameRateFromVideoFormat(GetInputVideoFormat(NTV2_CHANNEL2)); break;
    case NTV2_REFERENCE_INPUT3:     framerate = GetNTV2FrameRateFromVideoFormat(GetInputVideoFormat(NTV2_CHANNEL3)); break;
    case NTV2_REFERENCE_INPUT4:     framerate = GetNTV2FrameRateFromVideoFormat(GetInputVideoFormat(NTV2_CHANNEL4)); break;
    case NTV2_REFERENCE_INPUT5:     framerate = GetNTV2FrameRateFromVideoFormat(GetInputVideoFormat(NTV2_CHANNEL5)); break;
    case NTV2_REFERENCE_INPUT6:     framerate = GetNTV2FrameRateFromVideoFormat(GetInputVideoFormat(NTV2_CHANNEL6)); break;
    case NTV2_REFERENCE_INPUT7:     framerate = GetNTV2FrameRateFromVideoFormat(GetInputVideoFormat(NTV2_CHANNEL7)); break;
    case NTV2_REFERENCE_INPUT8:     framerate = GetNTV2FrameRateFromVideoFormat(GetInputVideoFormat(NTV2_CHANNEL8)); break;
    // default: device->GetFrameRate(framerate); break;
    }
}

void AJADevice::UpdateReferenceStringList() {
    std::vector<std::string> list{ REFERENCE_DEFAULTS[0], REFERENCE_DEFAULTS[1] };
    for (int i = 1; i <= NTV2DeviceGetNumVideoInputs(ID); ++i)
        list.push_back("SDI In " + std::to_string(i));

    nos::UpdateStringList(GetReferenceStringListName(GetSerialNumber()), list);
}

bool AJADevice::SetReference(const NTV2ReferenceSource inRefSource, const bool inKeepFramePulseSelect)
{
    return CNTV2Card::SetReference(inRefSource, inKeepFramePulseSelect);
}

std::unordered_set<NTV2Channel> AJADevice::GetFilteredChannels(bool isInput)
{
    std::unordered_set<NTV2Channel> re;
    for (auto [c, isChannelInput] : Channels)
        if (isInput == isChannelInput)
			re.insert(c);
	return re;
}

bool AJADevice::WaitVBL(NTV2Channel channel, bool isInput, NTV2FieldID fieldId)
{
    if (fieldId == NTV2_FIELD_INVALID) // Progressive
    {
        if (isInput)
            return WaitForInputVerticalInterrupt(channel);
        else
            return WaitForOutputVerticalInterrupt(channel);
    }
    else // Interlaced
    {
        if (isInput)
            return WaitForInputFieldID(fieldId, channel);
        else
            return WaitForOutputFieldID(fieldId, channel);
    }
}

void AJADevice::UpdateReferenceSource(std::string referenceValue, bool updateSettingsEntry)
{
    auto ReferenceSource = NTV2_REFERENCE_INVALID;
    if (referenceValue.empty())
        nosEngine.LogE("Empty value received for reference pin!");
    else if (std::string::npos != referenceValue.find("Reference In"))
        ReferenceSource = NTV2_REFERENCE_EXTERNAL;
    else if (std::string::npos != referenceValue.find("Free Run"))
        ReferenceSource = NTV2_REFERENCE_FREERUN;
    else if (auto pos = referenceValue.find("SDI In"); std::string::npos != pos)
        ReferenceSource = AJADevice::ChannelToRefSrc(NTV2Channel(referenceValue[pos + 7] - '1'));
    if (ReferenceSource != NTV2_REFERENCE_INVALID)
    {
        NTV2ReferenceSource curRef{};
        if (GetReference(curRef) && curRef != ReferenceSource) {
            SetReference(ReferenceSource);
            if (updateSettingsEntry)
                nosSettings->UpdateEntryValue((NSN_Reference.AsString() + "\\" + std::to_string(GetSerialNumber())).c_str(), nosBuffer{ .Data = &referenceValue[0], .Size = referenceValue.length() + 1 });
        }
    }
}

void AJADevice::RegisterNode(nos::uuid id)
{
    std::unique_lock lock(RegisteredNodesMutex);
	RegisteredNodes.insert(id);
}

void AJADevice::UnregisterNode(nos::uuid id)
{
	std::unique_lock lock(RegisteredNodesMutex);
	RegisteredNodes.erase(id);
}

bool AJADevice::CheckFirmware(std::string& msg, std::string& msgDetails)
{
    std::string date, time;
    BITFILE_INFO_STRUCT bitFileInfo{};
    bitFileInfo.whichFPGA = eFPGAVideoProc;
    if (DriverGetBitFileInformation(bitFileInfo))
    {
		date = bitFileInfo.dateStr;
		time = bitFileInfo.timeStr;
	}

	if (date.empty() || time.empty())
	{
		msg = "Firmware date or time is not available.";
		return false;
	}

    std::string model = GetModelName();
    if (auto it = firmware_list.find(model); it != firmware_list.end())
    {
        if (it->second > date)
        {
            msg = "Firmware out of date";
            msgDetails = "Installed firmware (" + date + ") is out of date. Recommended firmware date is " + it->second + ". Please update your device.";
            return false;
        }
        return true;
    }
    // ? This returns the below message if a newer untested firmware is used. We should return this even if its newer, if not tested.
    // Ideally a config file should be used to get the tested firmware list.
    msg = "Not tested firmware";
    msgDetails = "Firmware (" + date + ") for device (" + model + ") has not been tested.";
    return false;
}

#define NOS_FOURCC NTV2_FOURCC('N', 'O', 'S', '.')

std::string FourCCToString(ULWord& curAppFourCC)
{
    char fourCCBuf[5];
#if defined(AJA_LITTLE_ENDIAN)
    fourCCBuf[0] = reinterpret_cast<const char*>(&curAppFourCC)[3];
    fourCCBuf[1] = reinterpret_cast<const char*>(&curAppFourCC)[2];
    fourCCBuf[2] = reinterpret_cast<const char*>(&curAppFourCC)[1];
    fourCCBuf[3] = reinterpret_cast<const char*>(&curAppFourCC)[0];
#else
    fourCCBuf[0] = reinterpret_cast<const char*>(&curAppFourCC)[0];
    fourCCBuf[1] = reinterpret_cast<const char*>(&curAppFourCC)[1];
    fourCCBuf[2] = reinterpret_cast<const char*>(&curAppFourCC)[2];
    fourCCBuf[3] = reinterpret_cast<const char*>(&curAppFourCC)[3];
#endif
    fourCCBuf[4] = '\0';
    return fourCCBuf;
}

bool AJADevice::AcquireDevice()
{
    int32_t nosPid = static_cast<int32_t>(AJAProcess::GetPid());

    ULWord   curAppFourCC(AJA_FOURCC('?', '?', '?', '?'));
    int32_t curPid{};
    if (GetStreamingApplication(curAppFourCC, curPid))
    {
        if (curPid == nosPid)
            return true;
		auto curApp = FourCCToString(curAppFourCC);
        if (curPid != 0)
		    nosEngine.LogW("Device %s is already acquired by application %s (PID %d). Trying to reclaim it.", GetDisplayName().c_str(), curApp.c_str(), curPid);
    }

    if (!AcquireStreamForApplication(NOS_FOURCC, nosPid))
    {
		nosEngine.LogE("Failed to acquire device %s for Nodos.", GetDisplayName().c_str());
		return false;
    }
    nosEngine.LogD("Device %s acquired by Nodos.", GetDisplayName().c_str());
    return true;
}


void AJADevice::ReleaseDevice()
{
    ULWord   curAppFourCC(AJA_FOURCC('?', '?', '?', '?'));
    int32_t curPid;

    if (!GetStreamingApplication(curAppFourCC, curPid))
    {
        nosEngine.LogE("Cannot acquire streaming application for device %s", GetDisplayName().c_str());
        return;
    }

	auto nosPid = static_cast<int32_t>(AJAProcess::GetPid());
    if (nosPid != curPid)
    {
        auto curApp = FourCCToString(curAppFourCC);
		nosEngine.LogD("Device %s is acquired by another application (%s, PID %d).", GetDisplayName().c_str(), curApp.c_str(), curPid);
		return;
    }

    if (ReleaseStreamForApplication(NOS_FOURCC, nosPid))
        nosEngine.LogD("Device %s released by Nodos", GetDisplayName().c_str());
    else
        nosEngine.LogE("Failed to release device %s", GetDisplayName().c_str());
}
