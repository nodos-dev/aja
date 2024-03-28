// Copyright Nodos AS. All Rights Reserved.

#include "AJADevice.h"
#include "nosDefines.h"
#include "ntv2enums.h"
#include "ntv2signalrouter.h"
#include "ntv2utils.h"

std::map<std::string, uint64_t> AJADevice::EnumerateDevices()
{
    CNTV2DeviceScanner scanner;
    std::map<std::string, uint64_t>  re;
    for(auto& dev: scanner.GetDeviceInfoList())
    {
        re[dev.deviceIdentifier] = dev.deviceSerialNumber;
    }
    return re;
}

std::unordered_map<std::string, std::set<NTV2VideoFormat>> AJADevice::StringToFormat()
{
    std::unordered_map<std::string, std::set<NTV2VideoFormat>> re;
    for (u32 i = 0; i < NTV2_MAX_NUM_VIDEO_FORMATS; ++i)
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
    for(auto& dev: scanner.GetDeviceInfoList())
        Devices[dev.deviceSerialNumber] = (std::make_shared<AJADevice>(dev.deviceSerialNumber)); // TODO: Error check on AJADevice ctor.
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

std::shared_ptr<AJADevice> AJADevice::GetDevice(std::string const& name)
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
}

u32 AJADevice::GetFBSize(NTV2Channel channel)
{
    NTV2Framesize fsz = NTV2_FRAMESIZE_INVALID;
    bool quad = false;
    GetFrameBufferSize(channel, fsz);
    GetQuadFrameEnable(quad, channel);
    return NTV2FramesizeToByteCount(fsz) * (quad ? 4 : 1);
}

u32 AJADevice::GetIntrinsicSize()
{
    u32 max = 0;
    for (auto c : Channels)
    {
        max = max(max, GetFBSize(c));
    }
    return max;
}

AJADevice::~AJADevice()
{
    ClearState();
    ReleaseStreamForApplication(NTV2_FOURCC('M', 'Z', 'M', 'Z'), AJAProcess::GetPid());
    Close();
}

AJADevice::AJADevice(uint64_t serial)
{
    AJAStatus	status	(AJA_STATUS_SUCCESS);

    //	Open the device...
    if (!CNTV2DeviceScanner::GetDeviceWithSerial (serial, *this))
    {
        std::cerr << "## ERROR:  Device '" << serial << "' not found\n";
        return;
    }

    if (!IsDeviceReady(false))
    {
        std::cerr << "## ERROR:  Device '" << serial << "' not ready\n";
        return;
    }

    ID =  GetDeviceID();

    if (!::NTV2DeviceCanDoCapture(ID))
    {
        std::cerr << "## ERROR:  Device '" << serial << "' cannot capture\n";
        return;
    }

    std::thread([this]{ AcquireStreamForApplicationWithReference(NTV2_FOURCC('M','Z','M','Z'), static_cast<int32_t>(AJAProcess::GetPid())); }).detach();
    
    AJA_ASSERT(SetEveryFrameServices(NTV2_OEM_TASKS));			//	Since this is an OEM demo, use the OEM service level
    AJA_ASSERT(SetMultiFormatMode(true));

    ClearState();
}

bool AJADevice::ChannelIsValid(NTV2Channel channel, bool isInput, NTV2VideoFormat fmt, Mode mode)
{
    if(
        !isInput && (NTV2_FRAMERATE_INVALID != FPSFamily) && 
        (GetFrameRateFamily(GetNTV2FrameRateFromVideoFormat(fmt)) != GetFrameRateFamily(FPSFamily))
        )
    {
        return false;
    }
    
    const bool validFmt = isInput || (NTV2DeviceCanDoVideoFormat(ID, fmt) && (SL == mode || NTV2_IS_QUAD_FRAME_FORMAT(fmt)));
    
    bool (AJADevice::*Arr[2][2])(NTV2Channel) = {
        {&AJADevice::ChannelCanOutput, &AJADevice::CanMakeQuadOutputFromChannel},
        {&AJADevice::ChannelCanInput,  &AJADevice::CanMakeQuadInputFromChannel},
    };
    
    return validFmt && (this->*Arr[isInput][SL != mode])(channel);
}

bool AJADevice::ChannelCanInput(NTV2Channel channel)
{
    if (Channels.contains(channel))
    {
        return false;
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
    if (Channels.contains(channel))
    {
        return false;
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
    {
        std::cerr << "Warning: Detected signal is " << (isTsi ? "TSI" : "Squares") << " but requested config is " << ((mode == TSI) ? "TSI" : "Squares") << "\n";
    }
    
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
    {
        for(auto c : channels)
        {
            Channels.insert(c);
        }
    }
    return re;
}

bool AJADevice::RouteQuadOutputSignal(NTV2Channel channel, NTV2VideoFormat fmt, Mode mode, NTV2FrameBufferFormat fbFmt)
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
    {
        for(auto c : channels)
        {
            Channels.insert(c);
        }
    }

    return re;
}


bool AJADevice::SetRef(NTV2Channel channel)
{
    return SetReference(ChannelToRefSrc(channel));
}

bool AJADevice::RouteSLInputSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, NTV2FrameBufferFormat fbFmt)
{
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
    re &= (SetVideoFormat(videoFmt, false, false, channel));
    re &= (SetFrameBufferFormat(channel, fbFmt));
    re &= (Connect(GetFrameBufferInputXptFromChannel(channel), GetInputSourceOutputXpt(src)));
    // re &= (SetReference(NTV2InputSourceToReferenceSource(src)));
    if (re) Channels.insert(channel);
    return re;
}

bool AJADevice::RouteSLOutputSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, NTV2FrameBufferFormat fbFmt)
{
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
    if(re) Channels.insert(channel);
    return re;
}	

void AJADevice::CloseChannel(NTV2Channel channel, bool isInput,  bool isQuad)
{
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
    if (IsQuad(mode) && !NTV2_IS_QUAD_FRAME_FORMAT(fmt))
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
            u32 w, h;
            GetExtent(channel, mode, w, h);
            videoFmt = GetFirstMatchingVideoFormat(GetNTV2FrameRateFromVideoFormat(videoFmt), h, w, false, false, false);
        }
    }

    if (isInput ? RouteInputSignal(channel, videoFmt, mode, fbFmt) : RouteOutputSignal(channel, videoFmt, mode, fbFmt))
    {
        if (NTV2_FRAMERATE_INVALID == FPSFamily)
        {
            FPSFamily = GetFrameRateFamily(GetNTV2FrameRateFromVideoFormat(videoFmt));
            SetMultiFormatMode(false);
            for (u32 i = 0; i < NTV2_MAX_NUM_CHANNELS; ++i)
            {
                SetVideoFormat(videoFmt);
            }
            SetMultiFormatMode(true);
        }
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