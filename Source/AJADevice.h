/*
 * Copyright MediaZ Teknoloji A.S. All Rights Reserved.
 */

#pragma once

#include <ajabase/common/types.h>
#include <ajabase/common/timecodeburn.h>

#include <ajantv2/includes/ntv2card.h>
#include <ajantv2/includes/ntv2enums.h>
#include <ajantv2/includes/ntv2rp188.h>
#include <ajantv2/includes/ntv2utils.h>
#include <ajantv2/includes/ntv2signalrouter.h>

#include "ntv2publicinterface.h"
#include "ntv2vpid.h"

// stl
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <Nodos/PluginHelpers.hpp>

#define AJA_ASSERT(x) { if(!(x)) { printf("%s:%d\n", __FILE__, __LINE__); abort();} }

struct RestartParams {
    enum Flags : uint32_t {
        UpdateRingSize = 1 << 0,
    };
    uint32_t UpdateFlags;
    uint32_t RingSize;
};

struct AJADevice : CNTV2Card
{
    enum Mode: uint32_t
    {
        TSI,
        SQD,
        AUTO,
        SL,
    };

    static bool IsQuad(Mode mode) 
    {
        switch(mode)
        {
            case SL: return false;
            default: return true;
        }
    }
    
    inline static std::unordered_map<uint64_t, std::shared_ptr<AJADevice>> Devices;
   

    NTV2FrameRate FPSFamily = NTV2_FRAMERATE_INVALID;

    NTV2DeviceID ID;

    std::shared_mutex ChannelsMutex;
    //Channel To IsInput
    std::unordered_map<NTV2Channel, bool> Channels;

	// TODO: Remove when Legacy nodes are removed
    std::atomic_bool HasInput = false;
    std::atomic_bool HasOutput = false;

    static std::map<std::string, uint64_t>  EnumerateDevices();
    static std::unordered_map<std::string, std::set<NTV2VideoFormat>> StringToFormat();

    static std::map<std::string, uint64_t> AvailableDevices;

    inline static NTV2VideoFormat GetMatchingFormat(std::string const& fmt, bool multiLink)
    {
        static const auto Formats = StringToFormat();
        if(auto it = Formats.find(fmt); it != Formats.end())
            for (auto fmt : it->second)
                if (multiLink == NTV2_IS_SQUARE_DIVISION_FORMAT(fmt)) 
                    return fmt;
        return NTV2_FORMAT_UNKNOWN;
    }

    static uint64_t FindDeviceSerial(const char* ident);
    static bool DeviceAvailable(const char* ident, bool input);
    static bool GetAvailableDevice(bool input, AJADevice** = 0);
    static void Init();
    static void Deinit();
    static std::shared_ptr<AJADevice> GetDevice(std::string const& name);
    static std::shared_ptr<AJADevice> GetDevice(uint32_t index);
	static std::shared_ptr<AJADevice> GetDeviceBySerialNumber(uint64_t serial);

    static NTV2ReferenceSource ChannelToRefSrc(NTV2Channel channel)
    {
        return NTV2InputSourceToReferenceSource(NTV2ChannelToInputSource(channel, NTV2_INPUTSOURCES_SDI));
    }
    
    CNTV2VPID GetVPID(NTV2Channel channel, CNTV2VPID* B = 0);

    NTV2VideoFormat GetInputVideoFormat(NTV2Channel channel);
    NTV2VideoFormat ForceInterlace(NTV2VideoFormat channel);
    bool IsTSI(NTV2Channel channel);
    Mode GetMode(NTV2Channel channel);
    ~AJADevice();
    AJADevice(uint64_t serial);
    bool ChannelIsValid(NTV2Channel channel, bool isInput, NTV2VideoFormat fmt, Mode mode);

    bool CanChannelDoFormat(NTV2Channel channel, bool isInput, NTV2VideoFormat fmt, Mode mode);

    bool ChannelCanInput(NTV2Channel channel);
	bool ChannelCanOutput(NTV2Channel channel);

	bool CanMakeQuadInputFromChannel(NTV2Channel channel);
	bool CanMakeQuadOutputFromChannel(NTV2Channel channel);

    uint64_t GetLastInputVerticalInterruptTimestamp(NTV2Channel channel);
    
    bool RouteSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, bool isInput, Mode mode, NTV2FrameBufferFormat fbFmt);

    void CloseChannel(NTV2Channel channel, bool isInput, bool isQuad);

    void ClearState();

    uint32_t GetFBSize(NTV2Channel channel);

    bool GetExtent(NTV2Channel channel, Mode mode, uint32_t& width, uint32_t& height);
    bool GetExtent(NTV2VideoFormat fmt, Mode mode, uint32_t& width, uint32_t& height);

    void GetReferenceAndFrameRate(NTV2ReferenceSource& reference, NTV2FrameRate& framerate);

    uint32_t AddReferenceSourceListener(std::function<void(NTV2ReferenceSource)> listener);
    void RemoveReferenceSourceListener(uint32_t id);

    void RegisterNode(nosUUID id);
    void UnregisterNode(nosUUID id);

    bool SetReference (const NTV2ReferenceSource inRefSource, const bool inKeepFramePulseSelect = false) override;

    std::unordered_set<NTV2Channel> GetFilteredChannels(bool isInput);
    bool WaitVBL(NTV2Channel, bool isInput, NTV2FieldID fieldId);
    bool CheckFirmware(std::string& msg);
private:
    bool RouteSLInputSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, NTV2FrameBufferFormat fbFmt);
    bool RouteSLOutputSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, NTV2FrameBufferFormat fbFmt);

    bool RouteQuadInputSignal (NTV2Channel channel, NTV2VideoFormat videoFmt, Mode mode, NTV2FrameBufferFormat fbFmt);
    bool RouteQuadOutputSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, Mode mode, NTV2FrameBufferFormat fbFmt);

    bool RouteInputSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, Mode mode, NTV2FrameBufferFormat fbFmt)
    {
        return (mode != SL) ? RouteQuadInputSignal(channel, videoFmt, mode, fbFmt) : RouteSLInputSignal(channel, videoFmt, fbFmt);
    }

    bool RouteOutputSignal(NTV2Channel channel, NTV2VideoFormat videoFmt, Mode mode, NTV2FrameBufferFormat fbFmt)
    {
        return (mode != SL) ? RouteQuadOutputSignal(channel, videoFmt, mode, fbFmt) : RouteSLOutputSignal(channel, videoFmt, fbFmt);
    }

    void CloseSLChannel(NTV2Channel channel, bool isInput);
    void CloseQLChannel(NTV2Channel channel, bool isInput);

    void SendCheckConfigurationToNodes();

    struct {
        std::unordered_map<uint32_t, std::function<void(NTV2ReferenceSource)>> Map;
        uint32_t NextID = 0;
        std::mutex Mutex;
    } ReferenceListeners;

    std::mutex RegisteredNodesMutex;
    std::unordered_set<nosUUID> RegisteredNodes;
};

inline NTV2Channel ParseChannel(std::string_view const &name)
{
	size_t idx = name.find("Link");
	return NTV2Channel(name[idx + sizeof("Link")] - '1');
}

inline std::vector<uint8_t> StringValue(std::string const &str)
{
	return std::vector<uint8_t>((uint8_t *)str.data(), (uint8_t *)str.data() + str.size() + 1);
}

inline std::string GetQuadName(NTV2Channel channel)
{
	const char *links[8] = {"1234", "5678"};
	return (std::string) "QuadLink " + links[channel / 4];
}

inline std::string GetChannelName(NTV2Channel channel, AJADevice::Mode mode)
{
	switch (mode)
	{
	default:
		return GetQuadName(channel);
	case AJADevice::SL:
		return "SingleLink " + std::to_string(channel + 1);
	}
}
