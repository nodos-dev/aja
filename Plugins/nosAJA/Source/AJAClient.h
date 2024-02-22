/*
 * Copyright MediaZ AS. All Rights Reserved.
 */

#pragma once

#include "AJAMain.h"
#include "AJADevice.h"
#include "AJA_generated.h"

namespace nos
{
struct UUID
{
    fb::UUID id;
    UUID(fb::UUID id):id(id) {}
    bool operator ==(UUID const& rhs) const { return 0 == memcmp(id.bytes(), rhs.id.bytes(), 16); }
    operator fb::UUID() const { return id; }
    fb::UUID* operator ->() { return &id; }
    const fb::UUID* operator ->() const { return &id; }
    fb::UUID* operator &() { return &id; }
    const fb::UUID* operator &() const { return &id; }
};
}

template<> struct std::hash<nos::UUID>{ size_t operator()(nos::UUID const& val) const { return nos::UUIDHash(val); } };

namespace nos
{

std::vector<u8> StringValue(std::string const& str);
std::string GetQuadName(NTV2Channel channel);
std::string GetChannelStr(NTV2Channel channel, AJADevice::Mode mode);
const u8 *AddIfNotFound(Name name, std::string tyName, std::vector<u8> val,
                               std::unordered_map<Name, const nos::fb::Pin *> &pins,
                               std::vector<flatbuffers::Offset<nos::fb::Pin>> &toAdd,
                               std::vector<::flatbuffers::Offset<nos::PartialPinUpdate>>& toUpdate,
                               flatbuffers::FlatBufferBuilder &fbb, nos::fb::ShowAs showAs = nos::fb::ShowAs::PROPERTY,
                               nos::fb::CanShowAs canShowAs = nos::fb::CanShowAs::INPUT_PIN_OR_PROPERTY, 
                               std::optional<nos::fb::TVisualizer> visualizer = std::nullopt);

nos::fb::UUID GenerateUUID();

inline auto generator() 
{
    struct {
        nos::fb::UUID id; operator nos::fb::UUID *() { return &id;}
    } re {GenerateUUID()}; 
    return re;
}

inline NTV2FieldID GetAJAFieldID(nosTextureFieldType type)
{
	return (type == NOS_TEXTURE_FIELD_TYPE_PROGRESSIVE || type == NOS_TEXTURE_FIELD_TYPE_UNKNOWN)
			   ? NTV2_FIELD_INVALID
			   : (type == NOS_TEXTURE_FIELD_TYPE_EVEN ? NTV2_FIELD0 : NTV2_FIELD1);
}

struct AjaAction
{
    enum
    {
        INVALID = 0,
        ADD_CHANNEL = 1,
        ADD_QUAD_CHANNEL = 2,
        DELETE_CHANNEL = 3,
        SELECT_DEVICE = 4,
    } Action:4;
    u32 DeviceIndex:4;
    NTV2Channel Channel:5;
    NTV2VideoFormat Format:12;
    static void TestAjaAction()
    {
        static_assert(sizeof(AjaAction) == sizeof(u32));
    }

    operator u32() const { return *(u32*)this; }
};

struct NOSAPI_ATTR AJAClient
{
    inline static struct
    {
        std::mutex Mutex;
        std::set<AJAClient *> Clients;
        std::thread ControlThread;

        void ControlThreadProc()
        {
            while (!Clients.empty())
            {
                {
                    std::lock_guard lock(Mutex);
                    for (auto c : Clients)
                    {
                        c->UpdateDeviceStatus();
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }

        void Add(AJAClient *client)
        {
            {
                std::lock_guard lock(Mutex);
                Clients.insert(client);
            }
            if (Clients.size() == 1)
                ControlThread = std::thread([this] { ControlThreadProc(); });
        }

        void Remove(AJAClient *client)
        {
            {
                std::lock_guard lock(Mutex);
                Clients.erase(client);
            }
            if (Clients.empty())
            {
                ControlThread.join();
                AJADevice::Deinit();
            }
        }

    } Ctx;

    PinMapping Mapping;

    bool Input = false;
    std::atomic<aja::Shader> Shader = aja::Shader::Comp8;
    std::atomic_uint DispatchSizeX = 80, DispatchSizeY = 135;
    std::atomic_uint Debug = 0;

    std::unordered_map<nos::Name, rc<struct CopyThread>> Pins;
	std::unordered_map<nos::Name, AJADevice::Mode> OrphanPins;
    AJADevice *Device = 0;

    std::optional<nos::Table<nos::fb::Node>> NodeFb = std::nullopt;

    NTV2ReferenceSource Ref = NTV2_REFERENCE_EXTERNAL;
    NTV2FrameRate FR = NTV2_FRAMERATE_5994;

    AJAClient(bool input, AJADevice *device);
    ~AJAClient();

    void Init(nos::fb::Node const& node, AJADevice* dev);

    u32 BitWidth() const;

    PinMapping *operator->();
    fb::UUID GetPinId(Name pinName) const;

    void GeneratePinIDSet(Name pinName, AJADevice::Mode mode, std::vector<nos::fb::UUID> &ids);
    
    std::vector<nos::fb::UUID> GeneratePinIDSet(Name pinName, AJADevice::Mode mode);
    std::shared_ptr<CopyThread> FindChannel(NTV2Channel channel);
    NTV2FrameBufferFormat FBFmt() const;
    void StopAll();
    void StartAll();
    void UpdateDeviceStatus();
    void UpdateDeviceValue();
    void UpdateReferenceValue();
    void UpdateStatus();
    void Refresh();
    void UpdateStatus(flatbuffers::FlatBufferBuilder &fbb,
                      std::vector<flatbuffers::Offset<nos::fb::NodeStatusMessage>> &msg);
    void SetReference(std::string const &val);
    void OnNodeUpdate(nos::fb::Node const &event);
    void OnNodeUpdate(PinMapping &&newMapping, std::unordered_map<Name, const nos::fb::Pin *> &tmpPins,
                      std::vector<nos::fb::UUID> &pinsToOrphan);
    void OnPinMenuFired(nosContextMenuRequest const &request);
    
    bool CanRemoveOrphanPin(nos::Name pinName, nosUUID pinId);
    bool OnOrphanPinRemoved(nos::Name pinName, nosUUID pinId);

    void OnMenuFired(nosContextMenuRequest const &request);
    void OnCommandFired(u32 cmd);

    void OnNodeRemoved();

    void OnPathCommand(const nosPathCommand* cmd);
    void OnPinValueChanged(nos::Name pinName, void* value);
    void OnExecute();

    bool CopyFrom(nosCopyInfo &cpy);
    bool CopyTo(nosCopyInfo &cpy);

    void AddTexturePin(const nos::fb::Pin* pin, u32 ringSize, NTV2Channel channel,
                       const sys::vulkan::Texture* tex, NTV2VideoFormat fmt, AJADevice::Mode mode,
                       aja::Colorspace cs, aja::GammaCurve gc, bool range, unsigned spareCount);
    void DeleteTexturePin(rc<CopyThread> const& c);

    bool HasDevice() const { return Device != nullptr; }
    AJADevice* TryGetAvailableDevice() 
    { 
        AJADevice* dev = nullptr;
		AJADevice::GetAvailableDevice(Input, &dev);
		return dev;
    }

};

} // namespace nos