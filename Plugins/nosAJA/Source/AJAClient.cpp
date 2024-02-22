// Copyright MediaZ AS. All Rights Reserved.

#include "AJAClient.h"
#include "CopyThread.h"
#include "Ring.h"
#include "glm/glm.hpp"
#include <random>
#include <uuid.h>
#include <Nodos/PluginAPI.h>
#include <Nodos/Helpers.hpp>
#include <nosVulkanSubsystem/Helpers.hpp>

namespace nos
{

fb::UUID GenerateUUID()
{
    static std::mt19937 eng = std::mt19937(std::random_device()());
    static uuids::uuid_random_generator gen (&eng);
    return *(fb::UUID*)gen().as_bytes().data();
}

static NTV2Channel ParseChannel(std::string const &name)
{
    size_t idx = name.find("Link");
    return NTV2Channel(name[idx + sizeof("Link")] - '1');
}

std::vector<u8> StringValue(std::string const &str)
{
    return std::vector<u8>((u8 *)str.data(), (u8 *)str.data() + str.size() + 1);
}

std::string GetQuadName(NTV2Channel channel)
{
    const char *links[8] = {"1234", "5678"};
    return (std::string) "QuadLink " + links[channel / 4];
}

std::string GetChannelStr(NTV2Channel channel, AJADevice::Mode mode)
{
    switch (mode)
    {
    default:
        return GetQuadName(channel);
    case AJADevice::SL:
        return "SingleLink " + std::to_string(channel + 1);
    }
}

const u8 *AddIfNotFound(Name name, std::string tyName, std::vector<u8> val,
                        std::unordered_map<Name, const nos::fb::Pin *> &pins,
                        std::vector<flatbuffers::Offset<nos::fb::Pin>> &toAdd, 
                        std::vector<::flatbuffers::Offset<nos::PartialPinUpdate>>& toUpdate,
                        flatbuffers::FlatBufferBuilder &fbb,
                        nos::fb::ShowAs showAs, nos::fb::CanShowAs canShowAs, std::optional<nos::fb::TVisualizer> visualizer)
{
    if (auto pin = pins[name])
    {
        toUpdate.push_back(CreatePartialPinUpdateDirect(fbb, pin->id(), 0, nos::fb::CreateOrphanStateDirect(fbb, false)));
        return pin->data()->Data();
    }
    toAdd.push_back(
		nos::fb::CreatePinDirect(fbb, generator(), name.AsCStr(), tyName.c_str(), showAs, canShowAs, 0, visualizer ? nos::fb::Visualizer::Pack(fbb, &*visualizer) : 0, &val));
    return 0;
}

template <class K, class V> using SeqMap = std::vector<std::pair<K, V>>;

auto EnumerateFormats()
{
    struct FormatDescriptor
    {
        NTV2VideoFormat fmt;
        NTV2FrameRate fps;
        u32 w, h;
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
        bool i = !IsProgressiveTransport(fmt);
        NTV2FrameRate fps = GetNTV2FrameRateFromVideoFormat(fmt);
        auto desc = FormatDescriptor{
            .fmt = fmt,
            .fps = fps,
            .w = w,
            .h = h,
            .Interlaced = !IsProgressiveTransport(fmt),
            .ALevel = NTV2_VIDEO_FORMAT_IS_A(fmt),
            .BLevel = NTV2_VIDEO_FORMAT_IS_B(fmt),
        };

        u64 extent = ((u64(w) << u64(32)) | u64(h));
        re[extent][fps].push_back(desc);
    }

    SeqMap<glm::uvec2, SeqMap<f64, std::vector<FormatDescriptor>>> re2;

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
        return std::pair(glm::uvec2((extent >> 32) & 0xFFFFFFFF, extent & 0xFFFFFFFF), std::move(XX));
    });

    std::sort(re2.begin(), re2.end(), [](auto &a, auto &b) {
        if (a.first.x == 1920)
            return true;
        if (b.first.x == 1920)
            return false;
        return a.first.x > b.first.x;
    });
    return re2;
}

AJAClient::AJAClient(bool input, AJADevice *device) : Input(input), Device(device)
{
    Ctx.Add(this);
}

AJAClient::~AJAClient()
{
    Ctx.Remove(this); 
}

void AJAClient::Init(nos::fb::Node const& node, AJADevice* dev)
{
	flatbuffers::FlatBufferBuilder fbb;
	std::vector<flatbuffers::Offset<nos::fb::Pin>> pinsToAdd;
	std::vector<::flatbuffers::Offset<nos::PartialPinUpdate>> pinsToUpdate;
	flatbuffers::Offset<nos::fb::OrphanState> orphanState;
	PinMapping mapping;
	auto loadedPins = mapping.Load(node);

    if (HasDevice())
	{
		(Input ? Device->HasInput : Device->HasOutput) = true;
		Device->SetReference(Ref);
		Device->GetReferenceAndFrameRate(Ref, FR);
		orphanState = nos::fb::CreateOrphanStateDirect(fbb, false);
	    std::string refSrc =
		    "Ref : " + NTV2ReferenceSourceToString(Ref, true) + " - " + NTV2FrameRateToString(FR, true);

	    using nos::fb::CanShowAs;
	    using nos::fb::ShowAs;

	    AddIfNotFound(NSN_Device,
				      "string",
				      StringValue(dev->GetDisplayName()),
				      loadedPins,
				      pinsToAdd,
				      pinsToUpdate,
				      fbb,
				      ShowAs::PROPERTY,
				      CanShowAs::PROPERTY_ONLY);
	    if (auto val = AddIfNotFound(NSN_Dispatch_Size,
								     "nos.fb.vec2u",
								     nos::Buffer::From(nos::fb::vec2u(DispatchSizeX, DispatchSizeY)),
								     loadedPins,
								     pinsToAdd,
								     pinsToUpdate,
								     fbb))
	    {
		    DispatchSizeX = ((glm::uvec2*)val)->x;
		    DispatchSizeY = ((glm::uvec2*)val)->y;
	    }

	    if (auto val = AddIfNotFound(NSN_Shader_Type,
								     "nos.aja.Shader",
								     nos::Buffer::From(Shader.load()),
								     loadedPins,
								     pinsToAdd,
								     pinsToUpdate,
								     fbb))
	    {
		    Shader = *((aja::Shader*)val);
	    }

	    if (auto val = AddIfNotFound(
			    NSN_Debug, "uint", nos::Buffer::From(u32(Debug)), loadedPins, pinsToAdd, pinsToUpdate, fbb))
	    {
		    Debug = *((u32*)val);
	    }

	    if (!Input)
	    {
		    nos::fb::TVisualizer vis = {.type = nos::fb::VisualizerType::COMBO_BOX,
									    .name = dev->GetDisplayName() + "-AJAOut-Reference-Source"};
		    if (auto ref = AddIfNotFound(NSN_ReferenceSource,
									     "string",
									     StringValue(refSrc),
									     loadedPins,
									     pinsToAdd,
									     pinsToUpdate,
									     fbb,
									     ShowAs::PROPERTY,
									     CanShowAs::PROPERTY_ONLY,
									     vis))
		    {
			    refSrc = (char*)ref;
		    }
	    }
	    SetReference(refSrc);
    }
	else
	{
		NodeFb = nos::Buffer::From(node);
		orphanState = nos::fb::CreateOrphanStateDirect(fbb, true, "No suitable device selected.");
    }
	std::vector<flatbuffers::Offset<nos::fb::NodeStatusMessage>> msg;
	std::vector<nos::fb::UUID> pinsToOrphan;

	OnNodeUpdate(std::move(mapping), loadedPins, pinsToOrphan);
	UpdateStatus(fbb, msg);

    for (auto const& pinToOrphan : pinsToOrphan)
    {
		pinsToUpdate.push_back(nos::CreatePartialPinUpdateDirect(
			fbb, &pinToOrphan, 0, nos::fb::CreateOrphanStateDirect(fbb, true, "No suitable channel found.")));
    }

	HandleEvent(CreateAppEvent(
		fbb,
		nos::CreatePartialNodeUpdateDirect(
			fbb, &Mapping.NodeId, ClearFlags::NONE, 0, &pinsToAdd, 0, 0, 0, 0, &msg, &pinsToUpdate, 0, orphanState)));
}

u32 AJAClient::BitWidth() const
{
    switch (Shader)
    {
	case aja::Shader::Comp10:
        return 10;
    default:
        return 8;
    }
}

PinMapping *AJAClient::operator->()
{
    return &Mapping;
}

fb::UUID AJAClient::GetPinId(Name pinName) const
{
    return *Mapping.GetPinId(pinName);
}

void AJAClient::GeneratePinIDSet(Name pinName, AJADevice::Mode mode, std::vector<nos::fb::UUID> &ids)
{
	auto pinStr = pinName.AsString();

    ids.push_back(GetPinId(pinName));
	ids.push_back(GetPinId(Name(pinStr + " Ring Size")));
    ids.push_back(GetPinId(Name(pinStr + " Ring Spare Count")));
    ids.push_back(GetPinId(Name(pinStr + " Video Format")));
    ids.push_back(GetPinId(Name(pinStr + " Colorspace")));
    ids.push_back(GetPinId(Name(pinStr + " Gamma Curve")));
    ids.push_back(GetPinId(Name(pinStr + " Narrow Range")));

    if (AJADevice::IsQuad(mode))
        ids.push_back(GetPinId(Name(pinStr + " Mode")));
}

std::vector<nos::fb::UUID> AJAClient::GeneratePinIDSet(Name pinName, AJADevice::Mode mode)
{
    std::vector<nos::fb::UUID> ids;
    GeneratePinIDSet(pinName, mode, ids);
    return ids;
}

std::shared_ptr<CopyThread> AJAClient::FindChannel(NTV2Channel channel)
{
    for (auto &[_,p] : Pins)
    {
        if (p->Channel == channel)
        {
            return p;
        }
    }
    return 0;
}

NTV2FrameBufferFormat AJAClient::FBFmt() const
{
    return Shader == aja::Shader::Comp10 ? NTV2_FBF_10BIT_YCBCR : NTV2_FBF_8BIT_YCBCR;
}

void AJAClient::StopAll()
{
    for (auto& [_,th] : Pins)
        th->Stop();
}

void AJAClient::StartAll()
{
    for (auto& [_,th] : Pins)
        th->StartThread();
}

void AJAClient::UpdateDeviceStatus()
{
    if (!HasDevice())
    {
        if (auto* dev = TryGetAvailableDevice())
		{ 
            Device = dev;
			nosEngine.LogI("AAA: %s", (*NodeFb)->name()->c_str());
            Init(*NodeFb, dev);
        }
        return;
    }
    UpdateReferenceValue();
    if (!Input)
        return;

    bool mfmt = false;
    if (Device->GetMultiFormatMode(mfmt))
    {
        if (!mfmt)
        {
            Device->SetMultiFormatMode(true);
        }
    }
}

void AJAClient::UpdateDeviceValue()
{
    flatbuffers::FlatBufferBuilder fbb;
    auto pinId = GetPinId(NSN_Device);
    std::vector<u8> value = StringValue(Device->GetDisplayName());
	nosEngine.SetPinValue(pinId, {value.data(), value.size()});
    UpdateReferenceValue();
}

void AJAClient::UpdateReferenceValue()
{
    if (Input)
        return;
    auto oldRef = Ref;
    auto oldFr = FR;
    Device->GetReferenceAndFrameRate(Ref, FR);
    if (oldRef == Ref && oldFr == FR)
        return;

    flatbuffers::FlatBufferBuilder fbb;
    nos::fb::UUID pinId;
    if (auto id = Mapping.GetPinId(NSN_ReferenceSource))
        pinId = *id;
    else
        return;

    std::vector<u8> value = StringValue(NTV2ReferenceSourceToString(Ref, true));
	nosEngine.SetPinValue(pinId, {value.data(), value.size()});
    UpdateStatus();
}

void AJAClient::UpdateStatus()
{
    std::vector<flatbuffers::Offset<nos::fb::NodeStatusMessage>> msg;
    flatbuffers::FlatBufferBuilder fbb;
    UpdateStatus(fbb, msg);
    HandleEvent(CreateAppEvent(
        fbb, nos::CreatePartialNodeUpdateDirect(fbb, &Mapping.NodeId, ClearFlags::NONE, 0, 0, 0, 0, 0, 0, &msg)));
}

void AJAClient::UpdateStatus(flatbuffers::FlatBufferBuilder &fbb,
                             std::vector<flatbuffers::Offset<nos::fb::NodeStatusMessage>> &msg)
{
    msg.push_back(fb::CreateNodeStatusMessageDirect(
		fbb, HasDevice() ? Device->GetDisplayName().c_str() : "No device", fb::NodeStatusMessageType::INFO));

    if (HasDevice() && !Input)
    {
        Device->GetReferenceAndFrameRate(Ref, FR);
        msg.push_back(fb::CreateNodeStatusMessageDirect(
            fbb, ("Ref : " + NTV2ReferenceSourceToString(Ref, true) + " - " + NTV2FrameRateToString(FR, true)).c_str(),
            fb::NodeStatusMessageType::INFO));
    }

    // services.HandleEvent(CreateAppEvent(fbb, nos::app::CreatePartialNodeUpdateDirect(fbb, &mapping.NodeId,
    // ClearFlags::NONE, 0, 0, 0, 0, 0, 0, &msg)));
}

void AJAClient::SetReference(std::string const &val)
{
    auto src = NTV2_REFERENCE_INVALID;
    if (val.empty()) {
        nosEngine.LogE("Empty value received for reference pin!");
    }
    else if (std::string::npos != val.find("Reference In"))
    {
        src = NTV2_REFERENCE_EXTERNAL;
    }
    else if (std::string::npos != val.find("Free Run"))
    {
        src = NTV2_REFERENCE_FREERUN;
    }
    else if(auto pos = val.find("SDI In"); std::string::npos != pos)
    {
        src = AJADevice::ChannelToRefSrc(NTV2Channel(val[pos+7] - '1'));
    }
    
    if (src != NTV2_REFERENCE_INVALID)
    {
        Device->SetReference(src);
    }
    this->Ref = src;
}

void AJAClient::OnNodeUpdate(nos::fb::Node const &event)
{
    PinMapping mapping;
    auto pins = mapping.Load(event);
    if (!HasDevice())
    {
		NodeFb = nos::Buffer::From(event);
		return;
    }
    std::vector<nos::fb::UUID> pinsToOrphan;
    OnNodeUpdate(std::move(mapping), pins, pinsToOrphan);
    if (!pinsToOrphan.empty())
    {
		std::vector<flatbuffers::Offset<nos::PartialPinUpdate>> pinsToUpdate;
        flatbuffers::FlatBufferBuilder fbb;
		for (auto const& pinToOrphan : pinsToOrphan)
		{
			pinsToUpdate.push_back(nos::CreatePartialPinUpdateDirect(
				fbb, &pinToOrphan, 0, nos::fb::CreateOrphanStateDirect(fbb, true, "No suitable channel found.")));
		}
        HandleEvent(
            CreateAppEvent(fbb, nos::CreatePartialNodeUpdateDirect(fbb, &mapping.NodeId, ClearFlags::NONE, 0,
                                                                  0, 0, 0, 0, 0, 0, &pinsToUpdate)));
    }
}

void AJAClient::OnNodeUpdate(PinMapping &&newMapping, std::unordered_map<Name, const nos::fb::Pin *> &tmpPins,
                             std::vector<nos::fb::UUID> &pinsToOrphan)
{
    Mapping = std::move(newMapping);
	if (!HasDevice())
		return;
    struct StreamData
    {
        const nos::fb::Pin *pin = 0;
        const nos::fb::Pin *size = 0;
        const nos::fb::Pin *spare_count = 0;
        const nos::fb::Pin *frame_rate = 0;
        const nos::fb::Pin *quad_mode = 0;
        const nos::fb::Pin *colorspace = 0;
        const nos::fb::Pin *curve = 0;
        const nos::fb::Pin *narrow_range = 0;
    };

    std::map<NTV2Channel, StreamData> prs;

    for (auto &[name, pin] : tmpPins)
    {
        if (!pin)
            continue;
        const auto tyname = pin->type_name()->str();
		std::string str = name.AsString();
		NTV2Channel channel = ParseChannel(str);
		if (str.ends_with("Ring Size"))
        {
            prs[channel].size = pin;
        }
        else if (str.ends_with("Video Format"))
        {
            prs[channel].frame_rate = pin;
        }
		else if (str.ends_with("Ring Spare Count"))
        {
            prs[channel].spare_count = pin;
        }
        else if (tyname == "nos.sys.vulkan.Texture")
        {
            prs[channel].pin = pin;
        }
        else if (tyname == "nos.aja.QuadLinkMode" || tyname == "nos.aja.QuadLinkInputMode")
        {
            prs[channel].quad_mode = pin;
        }
        else if (tyname == "nos.aja.Colorspace")
        {
            prs[channel].colorspace = pin;
        }
        else if (tyname == "nos.aja.GammaCurve")
        {
            prs[channel].curve = pin;
        }
		else if (str.ends_with("Narrow Range"))
        {
            prs[channel].narrow_range = pin;
        }
    }

    {
        auto tmp = Pins;
        for (auto &[_,th] : tmp)
        {
            if (!Mapping.GetPinId(th->PinName))
            {
                DeleteTexturePin(th);
            }
        }
    }

    for (auto [channel, pr] : prs)
    {
        auto pin = pr.pin;
		if (!pin)
			continue;
        nos::Name name(pin->name()->c_str());
        auto tex = flatbuffers::GetRoot<sys::vulkan::Texture>(pin->data()->Data());
        auto id = *pin->id();
        auto it = Pins.find(name);

        if (it != Pins.end())
        {
            // it->second->Restart(tex);
        }
        else
        {
            auto mode = pr.quad_mode ? *(AJADevice::Mode*)pr.quad_mode->data()->Data() : AJADevice::SL;
            NTV2VideoFormat fmt = NTV2_FORMAT_UNKNOWN;
            switch (pin->show_as())
            {
            case nos::fb::ShowAs::INPUT_PIN: {
                if (pr.frame_rate && flatbuffers::IsFieldPresent(pr.frame_rate, nos::fb::Pin::VT_DATA))
                {
                    fmt = AJADevice::GetMatchingFormat((const char*)(pr.frame_rate->data()->Data()), AJADevice::IsQuad(mode));
                    nosEngine.LogI("Route output %s with framerate %s", NTV2ChannelToString(channel, true).c_str(),
								  NTV2VideoFormatToString(fmt, true).c_str());
                }
                break;
            }
            case nos::fb::ShowAs::OUTPUT_PIN:
				nosEngine.LogI("Route input %s", NTV2ChannelToString(channel, true).c_str());
                break;
            }
          
            if (Device->RouteSignal(channel, fmt, Input, mode, FBFmt()))
            {
				fmt = Device->GetInputVideoFormat(channel);
                auto cs = *(aja::Colorspace *)pr.colorspace->data()->Data();
                auto gc = *(aja::GammaCurve *)pr.curve->data()->Data();
                auto range = *(bool *)pr.narrow_range->data()->Data();
                auto spareCount = pr.spare_count ? *(u32*)pr.spare_count->data()->Data() : 0;

                AddTexturePin(pin, *(u32*)pr.size->data()->Data(), channel, tex, fmt, mode, cs, gc, range, spareCount);
			}
            else
            {
				if (!OrphanPins.contains(name))
				{
					GeneratePinIDSet(name, mode, pinsToOrphan);
					OrphanPins[name] = mode;
                }
                continue;
            }
        }
    }
}

void AJAClient::OnPinMenuFired(nosContextMenuRequest const &request)
{
    flatbuffers::FlatBufferBuilder fbb;
    auto name = *Mapping.GetPinName(*request.item_id());
    if (auto pin = FindChannel(ParseChannel(name.AsString())))
    {
        if (pin->IsOrphan)
            return;
        std::vector<flatbuffers::Offset<nos::ContextMenuItem>> remove = {
            nos::CreateContextMenuItemDirect(fbb, "Remove",
                                            AjaAction{
                                                .Action = AjaAction::DELETE_CHANNEL,
                                                .DeviceIndex = Device->GetIndexNumber(),
                                                .Channel = pin->Channel,
                                            })};

        HandleEvent(CreateAppEvent(
            fbb, nos::app::CreateAppContextMenuUpdateDirect(fbb, request.item_id(), request.pos(), request.instigator(), &remove)));
    }
}

bool AJAClient::CanRemoveOrphanPin(nos::Name pinName, nosUUID pinId)
{
    auto pin = FindChannel(ParseChannel(pinName.AsString()));
    return pin != nullptr || OrphanPins.contains(pinName);
}

bool AJAClient::OnOrphanPinRemoved(nos::Name pinName, nosUUID pinId)
{
    auto pin = FindChannel(ParseChannel(pinName.AsString()));
    if (!pin)
    {
        if (auto it = OrphanPins.find(pinName); it != OrphanPins.end())
        {
			flatbuffers::FlatBufferBuilder fbb;
			auto ids = GeneratePinIDSet(pinName, it->second);
			HandleEvent(CreateAppEvent(
				fbb, nos::CreatePartialNodeUpdateDirect(fbb, &Mapping.NodeId, ClearFlags::NONE, &ids)));
			OrphanPins.erase(it);
            return true;
        }
        return false;
    }
    pin->SendDeleteRequest();
    return true;
}

void AJAClient::OnMenuFired(nosContextMenuRequest const&request)
{
    if (0 != memcmp(request.item_id(), &Mapping.NodeId, 16))
    {
        return OnPinMenuFired(request);
    }

    flatbuffers::FlatBufferBuilder fbb;

    std::vector<flatbuffers::Offset<nos::ContextMenuItem>> items;
    std::vector<flatbuffers::Offset<nos::ContextMenuItem>> devices;

    for (auto &d : AJADevice::Devices)
    {
        if (d.get() != Device && ((Input && !d->HasInput) || (!Input && !d->HasOutput)))
        {
            devices.push_back(nos::CreateContextMenuItemDirect(fbb, d->GetDisplayName().c_str(),
                                                              AjaAction{
                                                                  .Action = AjaAction::SELECT_DEVICE,
                                                                  .DeviceIndex = d->GetIndexNumber(),
                                                              }));
        }
    }

    if (HasDevice())
    {
        static auto Descriptors = EnumerateFormats();

        for (u32 i = NTV2_CHANNEL1; i < NTV2_MAX_NUM_CHANNELS; ++i)
        {

            AJADevice::Mode modes[2] = {AJADevice::SL, AJADevice::AUTO};
            for (auto mode : modes)
            {
                NTV2Channel channel = NTV2Channel(AJADevice::SL == mode ? i : NTV2_MAX_NUM_CHANNELS - i - 1);
                AjaAction action = {
                    .Action = (AJADevice::SL == mode) ? AjaAction::ADD_CHANNEL : AjaAction::ADD_QUAD_CHANNEL,
                    .DeviceIndex = Device->GetIndexNumber(),
                    .Channel = channel,
                };
                auto it = AJADevice::SL != mode ? items.begin() : items.end();
                auto channelStr = GetChannelStr(channel, mode);
                if (Input)
                {
                    if (Device->ChannelIsValid(channel, Input, NTV2_FORMAT_UNKNOWN, mode))
                    {
                        items.insert(it, nos::CreateContextMenuItemDirect(fbb, channelStr.c_str(), action));
                    }
                }
                else
                {
                    std::vector<flatbuffers::Offset<nos::ContextMenuItem>> extents;
                    for (auto &[extent, Container0] : Descriptors)
                    {
                        std::vector<flatbuffers::Offset<nos::ContextMenuItem>> frameRates;
                        for (auto &[fps, Container1] : Container0)
                        {
                            std::vector<flatbuffers::Offset<nos::ContextMenuItem>> formats;
                            for (auto &desc : Container1)
                            {
                                if (Device->ChannelIsValid(channel, false, desc.fmt, mode))
                                {
                                    action.Format = desc.fmt;
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
                                GetNTV2FrameRateFromNumeratorDenominator(0, 0);
                                char buf[16] = {};
                                std::sprintf(buf, "%.2f", fps);
                                frameRates.push_back(nos::CreateContextMenuItemDirect(fbb, buf, 0, &formats));
                            }
                        }

                        if (!frameRates.empty())
                        {
                            char buf[32] = {};
                            std::sprintf(buf, "%dx%d", extent.x, extent.y);
                            extents.push_back(nos::CreateContextMenuItemDirect(fbb, buf, 0, &frameRates));
                        }
                    }

                    if (!extents.empty())
                    {
                        items.insert(it, nos::CreateContextMenuItemDirect(fbb, channelStr.c_str(), 0, &extents));
                    }
                }
            }
        }
    }

    if (!devices.empty())
    {
        items.push_back(nos::CreateContextMenuItemDirect(fbb, "Select Device", 0, &devices));
    }

    if (items.empty())
    {
        return;
    }

    HandleEvent(CreateAppEvent(
        fbb, nos::app::CreateAppContextMenuUpdateDirect(fbb, &Mapping.NodeId, request.pos(), request.instigator(), &items)));
}

void AJAClient::OnCommandFired(u32 cmd)
{
    if (!cmd)
    {
        return;
    }
    AjaAction action = (AjaAction &)cmd;
    auto channel = action.Channel;

    switch (action.Action)
    {
    case AjaAction::SELECT_DEVICE: {
		for (auto& [_, pin] : Pins)
		{
			Device->CloseChannel(pin->Channel, pin->IsInput(), pin->IsQuad());
			pin->Stop();
			pin->SendDeleteRequest();
		}

        auto newDev = AJADevice::GetDevice(action.DeviceIndex).get();
        ;
        (Input ? Device->HasInput : Device->HasOutput) = false;
        Device = newDev;
        (Input ? Device->HasInput : Device->HasOutput) = true;
        if (!HasDevice())
        {
			Init(*NodeFb, newDev);
        }
        UpdateDeviceValue();
        UpdateStatus();
        break;
    }
    case AjaAction::DELETE_CHANNEL:
        for (auto &[_,pin] : Pins)
        {
            if (pin->Channel == channel)
            {
                pin->SendDeleteRequest();
                break;
            }
        }
        break;

    case AjaAction::ADD_QUAD_CHANNEL:
    case AjaAction::ADD_CHANNEL: {
        const bool isQuad = action.Action == AjaAction::ADD_QUAD_CHANNEL;
        const AJADevice::Mode mode = isQuad ? (Input ? AJADevice::AUTO : AJADevice::TSI) : AJADevice::SL;
        NTV2VideoFormat format = action.Format;
        if (Input)
        {
            format = Device->GetInputVideoFormat(action.Channel);
        }

        u32 width = 1920 * (1 + isQuad);
        u32 height = 1080 * (1 + isQuad);
        Device->GetExtent(format, mode, width, height);
        bool validates = !IsProgressiveTransport(format); // interlaced input and output both validate

        sys::vulkan::TTexture tex;
        tex.resolution = sys::vulkan::SizePreset::CUSTOM;
        tex.width = width;
        tex.height = height;
        tex.unscaled = true;    // Prevent auto-scaling when an output pin is connected to this pin.
        tex.unmanaged = !Input; // do not create resource for this pin, do not assign test signal as well
        tex.format = sys::vulkan::Format::R16G16B16A16_UNORM;
        nos::fb::ShowAs showAs = Input ? nos::fb::ShowAs::OUTPUT_PIN : nos::fb::ShowAs::INPUT_PIN;
        nos::fb::CanShowAs canShowAs = Input ? nos::fb::CanShowAs::OUTPUT_PIN_ONLY : nos::fb::CanShowAs::INPUT_PIN_ONLY;
        std::string pinName = (isQuad ? GetQuadName(channel) : ("SingleLink " + std::to_string(channel + 1)));
        std::vector<u8> data = nos::Buffer::From(tex);
        std::vector<u8> ringData = nos::Buffer::From(2);
        std::vector<u8> spareCountData = nos::Buffer::From(0);
        std::vector<u8> ringDataMin = nos::Buffer::From(1);
        std::vector<u8> ringDataMax = nos::Buffer::From(AJA_MAX_RING_SIZE);
        flatbuffers::FlatBufferBuilder fbb;
        std::string fmtString = NTV2VideoFormatToString(format, true);
        std::vector<u8> fmtData(fmtString.data(), fmtString.data() + fmtString.size() + 1);
        std::vector<u8> colorspaceData = nos::Buffer::From(aja::Colorspace::REC709);
        std::vector<u8> curveData = nos::Buffer::From(aja::GammaCurve::REC709);
        std::vector<u8> narrowRangeData = nos::Buffer::From(true);

        std::vector<flatbuffers::Offset<nos::fb::Pin>> pins = {
            nos::fb::CreatePinDirect(fbb, generator(), pinName.c_str(), "nos.sys.vulkan.Texture", showAs, canShowAs, 0, 0, &data,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, Input, fb::PinContents::NONE, 0, 0, validates),
            nos::fb::CreatePinDirect(fbb, generator(), (pinName + " Ring Size").c_str(), "uint",
                                    nos::fb::ShowAs::PROPERTY, nos::fb::CanShowAs::OUTPUT_PIN_OR_PROPERTY, 0, 0,
                                    &ringData, 0, &ringDataMin, &ringDataMax, nullptr, .0f, Input),
            nos::fb::CreatePinDirect(fbb, generator(), (pinName + " Ring Spare Count").c_str(), "uint",
                                    nos::fb::ShowAs::PROPERTY, nos::fb::CanShowAs::OUTPUT_PIN_OR_PROPERTY, 0, 0,
                                    &spareCountData, 0, &spareCountData, &ringDataMax, nullptr, .0f),
            nos::fb::CreatePinDirect(fbb, generator(), (pinName + " Video Format").c_str(), "string",
                                    nos::fb::ShowAs::PROPERTY, nos::fb::CanShowAs::OUTPUT_PIN_OR_PROPERTY, 0, 0, &fmtData,
                                    0, 0, 0, 0, 0, true),
            nos::fb::CreatePinDirect(fbb, generator(), (pinName + " Colorspace").c_str(), "nos.aja.Colorspace",
                                    nos::fb::ShowAs::PROPERTY, nos::fb::CanShowAs::INPUT_OUTPUT_PROPERTY, 0, 0,
                                    &colorspaceData, 0, 0, 0, 0, 0, false),
            nos::fb::CreatePinDirect(fbb, generator(), (pinName + " Gamma Curve").c_str(), "nos.aja.GammaCurve",
                                    nos::fb::ShowAs::PROPERTY, nos::fb::CanShowAs::INPUT_OUTPUT_PROPERTY, 0, 0,
                                    &curveData, 0, 0, 0, 0, 0, false),
            nos::fb::CreatePinDirect(fbb, generator(), (pinName + " Narrow Range").c_str(), "bool",
                                    nos::fb::ShowAs::PROPERTY, nos::fb::CanShowAs::INPUT_OUTPUT_PROPERTY, 0, 0,
                                    &narrowRangeData, 0, 0, 0, 0, 0, false),
        };

        if (isQuad)
        {
            std::vector<u8> data = nos::Buffer::From(mode);
            pins.push_back(nos::fb::CreatePinDirect(
                fbb, generator(), (pinName + " Mode").c_str(), Input ? "nos.aja.QuadLinkInputMode" : "nos.aja.QuadLinkMode",
                nos::fb::ShowAs::PROPERTY, nos::fb::CanShowAs::PROPERTY_ONLY, 0, 0, &data));
        }

        HandleEvent(
            CreateAppEvent(fbb, nos::CreatePartialNodeUpdateDirect(fbb, &Mapping.NodeId, ClearFlags::NONE, 0, &pins)));
        break;
    }
    }
}

void AJAClient::OnNodeRemoved()
{
    for (auto& [_,th] : Pins)
        // Because there can be references to the CopyThread shared pointers in 'pins' list,
        // CopyThread destructor will not be called in the subsequent pins.clear call. So we need to stop the thread
        // here to avoid accessing the device (in CopyThread) after it has been destroyed.
        th->Stop();
    Pins.clear();
	if (HasDevice())
        (Input ? Device->HasInput : Device->HasOutput) = false;
}

void AJAClient::OnPathCommand(const nosPathCommand* cmd)
{
    auto pinId = cmd->PinId;
    auto pinNameOpt = Mapping.GetPinName(pinId);
    if (!pinNameOpt)
    {
        nosEngine.LogD("Path command on unknown pin: %s", UUID2STR(pinId).c_str());
        return;
    }
    auto pinName = *pinNameOpt;
	auto result = Pins.find(pinName);
	if (result == Pins.end())
	{
        nosEngine.LogD("Path command on unknown pin: %s", pinName.AsCStr());
		return;
	}
	auto copyThread = result->second;
	copyThread->Restart(cmd->Event == NOS_RING_SIZE_CHANGE ? cmd->RingSize : 0);
}

void AJAClient::Refresh()
{
    StopAll();
    for (auto& [_,th] : Pins)
    {
        th->Refresh();
        th->UpdateCurve(th->GammaCurve);
    }
    StartAll();
    for (auto& [_,th] : Pins)
        th->NotifyRestart({});
    return;
}

void AJAClient::OnPinValueChanged(nos::Name pinName, void *value)
{
    if (!value)
        return;

    std::string pinNameStr = pinName.AsString();
    
    if (pinNameStr == "Shader Type")
    {
        StopAll();
        Shader = *(aja::Shader *)value;
        Refresh();
        return;
    }

    if (pinNameStr == "Dispatch Size")
    {
        DispatchSizeX = ((nos::fb::vec2u *)value)->x();
        DispatchSizeY = ((nos::fb::vec2u *)value)->y();
        return;
    }

    if (pinNameStr == "Debug")
    {
        Debug = *(u32 *)value;
        return;
    }

    if (!Input && pinNameStr == "ReferenceSource")
    {
        SetReference((char *)value);
        UpdateReferenceValue();
        return;
    }

    if (pinNameStr.ends_with("Narrow Range"))
    {
		FindChannel(ParseChannel(pinNameStr))->NarrowRange = *(bool*)value;
        return;
    }

    if (pinNameStr.ends_with("Colorspace"))
    {
		FindChannel(ParseChannel(pinNameStr))->Colorspace = *(aja::Colorspace*)value;
        return;
    }

    if (pinNameStr.ends_with("Gamma Curve"))
    {
		FindChannel(ParseChannel(pinNameStr))->UpdateCurve(*(aja::GammaCurve*)value);
        return;
    }

    if (pinNameStr.ends_with("Mode"))
    {
		auto pin = FindChannel(ParseChannel(pinNameStr));
        auto mode = *(AJADevice::Mode*)value;
        if (mode != pin->Mode)
        {
            pin->Stop();
            pin->Mode = mode;
            pin->Refresh();
            pin->StartThread();
        }
        return;
    }

    if (pinNameStr.ends_with("Ring Size"))
    {
        auto pin = FindChannel(ParseChannel(pinNameStr));
		if (pin && pin->RingSize != *(u32*)value)
            pin->NotifyRestart(*(u32*)value);
    }
	if (pinNameStr.ends_with("Ring Spare Count"))
    {
		auto channel = ParseChannel(pinNameStr);
		auto pin = FindChannel(channel);
        pin->SpareCount = *(u32*)value;
		if (pin->SpareCount >= pin->RingSize)
		{
			uint32_t newSpareCount = pin->RingSize - 1; 
			pin->SpareCount = newSpareCount;
			nosEngine.LogW("Spare count must be less than ring size! Capping spare count at %u.", newSpareCount);
			nosEngine.SetPinValueByName(Mapping.NodeId, pinName, nosBuffer{.Data = &newSpareCount, .Size = sizeof(newSpareCount)});
		}
    }
}

void AJAClient::OnExecute()
{
}

bool AJAClient::CopyFrom(nosCopyInfo &cpy)
{
    auto it = Pins.find(cpy.Name); 
    if (it == Pins.end()) 
    	return false;
	auto th = it->second;
	CPURing::Resource* slot = nullptr;
	auto effectiveSpareCount = th->SpareCount * (1 + u32(th->Interlaced()));

	if (!(slot = th->Ring->TryPop(cpy.FrameNumber, effectiveSpareCount)))
		return false;

	auto& params = slot->Params;
	nos::sys::vulkan::TTexture outTex;
	flatbuffers::GetRoot<nos::sys::vulkan::Texture>(cpy.PinData->Data)->UnPackTo(&outTex);
	outTex.field_type = static_cast<nos::sys::vulkan::FieldType>(params.FieldType);
	nosEngine.SetPinValue(cpy.ID, nos::Buffer::From(outTex));

	std::vector<nosShaderBinding> inputs;
	uint32_t iFlags = (vkss::IsTextureFieldTypeInterlaced(params.FieldType) ? u32(params.FieldType) : 0) | (Shader == aja::Shader::Comp10) << 2;

	inputs.emplace_back(vkss::ShaderBinding(NSN_Colorspace, params.ColorspaceMatrix));
	inputs.emplace_back(vkss::ShaderBinding(NSN_Source, &th->ConversionIntermediateTex->Res));
	inputs.emplace_back(vkss::ShaderBinding(NSN_Interlaced, iFlags));
	inputs.emplace_back(vkss::ShaderBinding(NSN_ssbo, th->SSBO->Res));

	auto MsgKey = "Input " + th->Name().AsString() + " DMA";

	nosCmd cmd;
	nosVulkan->Begin("AJA Input YUV Conversion", &cmd);
	nosVulkan->Copy(cmd,
					&slot->Res,
					&th->ConversionIntermediateTex->Res,
					Debug ? ("(GPUTransfer)" + MsgKey + ":" + std::to_string(Debug)).c_str() : 0);

	auto outPinResource = vkss::DeserializeTextureInfo(cpy.PinData->Data);
	inputs.emplace_back(vkss::ShaderBinding(NSN_Output, outPinResource));
	nosRunComputePassParams pass = {};
	pass.Key = NSN_AJA_YCbCr2RGB_Compute_Pass;
	pass.DispatchSize = th->GetSuitableDispatchSize();
	pass.Bindings = inputs.data();
	pass.BindingCount = inputs.size();
	pass.Benchmark = Debug;
	nosVulkan->RunComputePass(cmd, &pass);

	nosGPUEvent event{};
    nosCmdEndParams endParams = {.OutGPUEventHandle = &event};
    nosVulkan->End(cmd, &endParams);
    slot->Params.WaitEvent = event;

	cpy.FrameNumber = slot->FrameNumber;
    cpy.CopyFromOptions.ShouldSetSourceFrameNumber = true;
    th->Ring->EndPop(slot);
	return true;
}

bool AJAClient::CopyTo(nosCopyInfo &cpy)
{
	auto it = Pins.find(cpy.Name); 
    if (it == Pins.end()) 
        return true;

    auto th = it->second;

    if (!th->Ring->CanPush())
    {
		nosEngine.LogI("%s: Trying to copy while ring full.", th->Name().AsCStr());
    }
	auto outgoing = th->Ring->TryPush();
	if (!outgoing)
	{
		return false;
	}
	outgoing->FrameNumber = cpy.FrameNumber;
 	auto wantedField = th->OutFieldType;
	auto outInterlaced = vkss::IsTextureFieldTypeInterlaced(wantedField);
	auto incomingTextureInfo = vkss::DeserializeTextureInfo(cpy.CopyToOptions.IncomingPinData->Data);
	auto incomingField = incomingTextureInfo.Info.Texture.FieldType;
	auto inInterlaced = vkss::IsTextureFieldTypeInterlaced(incomingField);
	if ((inInterlaced && outInterlaced) && incomingField != wantedField)
	{
		nosEngine.LogW("%s: Field mismatch. Waiting for a new frame.", th->PinName.AsCStr());
		th->Ring->CancelPush(outgoing);
		return false;
	}
	outgoing->Params.FieldType = wantedField;

    glm::mat4 colorspaceMatrix = th->GetMatrix<f64>();
	// 0th bit: is out even
	// 1st bit: is out odd
	// 2nd bit: is input even
	// 3rd bit: is input odd
	// 4th bit: comp10
	uint32_t iFlags = ((Shader == aja::Shader::Comp10) << 4);
	if (outInterlaced)
		iFlags |= u32(wantedField);
	if (inInterlaced)
		iFlags |= (u32(incomingField) << 2);

	nosCmd cmd;
	nosVulkan->Begin("AJA Output YUV Conversion", &cmd);
	std::vector<nosShaderBinding> inputs;
	inputs.emplace_back(vkss::ShaderBinding(NSN_Colorspace, colorspaceMatrix));
	inputs.emplace_back(vkss::ShaderBinding(NSN_Source, incomingTextureInfo));
	inputs.emplace_back(vkss::ShaderBinding(NSN_Interlaced, iFlags));
	inputs.emplace_back(vkss::ShaderBinding(NSN_ssbo, th->SSBO->Res));
	// watch out for th members, they are not synced
	inputs.emplace_back(vkss::ShaderBinding(NSN_Output, th->ConversionIntermediateTex->Res));
	nosRunComputePassParams pass = {};
	pass.Key = NSN_AJA_RGB2YCbCr_Compute_Pass;
	pass.DispatchSize = th->GetSuitableDispatchSize();
	pass.Bindings = inputs.data();
	pass.BindingCount = inputs.size();
	pass.Benchmark = Debug;
	nosVulkan->RunComputePass(cmd, &pass);
	nosCmdEndParams endParams{.ForceSubmit = true};
	nosVulkan->End(cmd, &endParams);

	nosVulkan->Begin("AJA Output Ring Copy", &cmd);
	nosVulkan->Copy(cmd, &th->ConversionIntermediateTex->Res, &outgoing->Res, 0);
	endParams.OutGPUEventHandle = &outgoing->Params.WaitEvent;
    nosVulkan->End(cmd, &endParams); // Wait in DMA thread.

	th->Ring->EndPush(outgoing);
	th->OutFieldType = vkss::FlippedField(th->OutFieldType);
	return true;
}

void AJAClient::AddTexturePin(const nos::fb::Pin* pin, u32 ringSize, NTV2Channel channel,
    const sys::vulkan::Texture* tex, NTV2VideoFormat fmt, AJADevice::Mode mode, aja::Colorspace cs, aja::GammaCurve gc, bool range, unsigned spareCount)
{
    auto th = MakeShared<CopyThread>(this, ringSize, spareCount, 
                                     pin->show_as(), channel, fmt, mode, cs, gc, range, tex);
    Pins.insert({th->Name(), std::move(th)});
	OrphanPins.erase(nos::Name(pin->name()->c_str()));
}

void AJAClient::DeleteTexturePin(rc<CopyThread> const& c)
{
    c->Stop();
    Pins.erase(c->Name());
}

} // namespace nos