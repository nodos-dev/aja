// Copyright Nodos AS. All Rights Reserved.

#include "AJAMain.h"
#include "CopyThread.h"
#include "AJAClient.h"

#include "../Shaders/RGB2YCbCr.comp.spv.dat"
#include "../Shaders/RGB2YCbCr.frag.spv.dat"
#include "../Shaders/YCbCr2RGB.comp.spv.dat"
#include "../Shaders/YCbCr2RGB.frag.spv.dat"

#include <Nodos/PluginAPI.h>

using namespace nos;

NOS_INIT();

namespace nos
{

static nosBuffer Blob2Buf(std::vector<u8> const& v) 
{ 
    return { (void*)v.data(), v.size() }; 
};

static std::vector<std::pair<Name, std::tuple<nosShaderStage, const char*, std::vector<u8>>>> GShaders;

struct AJA
{
    static nosResult GetShaders(size_t* outCount, nosShaderInfo* outShaders)
    {
        *outCount = GShaders.size();
        if(!outShaders) 
            return NOS_RESULT_SUCCESS;

        for (auto& [name, data] : GShaders)
        {
            auto& [stage, path, spirv] = data;
            *outShaders++ = {
                .Key = name,
                .Source = { 
                    .Stage = stage,
                    .GLSLPath = path,
                    .SpirvBlob = { spirv.data(), spirv.size() } 
                },
            };
        }

        return NOS_RESULT_SUCCESS;
    };

    static nosResult GetPasses(size_t* outCount, nosPassInfo* outNosPassInfos)
    {
        const std::vector<nosPassInfo> passes =
        {
			{.Key = NSN_AJA_RGB2YCbCr_Compute_Pass, .Shader = NSN_AJA_RGB2YCbCr_Compute_Shader, .MultiSample = 1},
			{.Key = NSN_AJA_YCbCr2RGB_Compute_Pass, .Shader = NSN_AJA_YCbCr2RGB_Compute_Shader, .MultiSample = 1},
			{.Key = NSN_AJA_YCbCr2RGB_Pass, .Shader = NSN_AJA_YCbCr2RGB_Shader, .MultiSample = 1},
			{.Key = NSN_AJA_RGB2YCbCr_Pass, .Shader = NSN_AJA_RGB2YCbCr_Shader, .MultiSample = 1}
        };

        *outCount = passes.size();

        if (!outNosPassInfos)
            return NOS_RESULT_SUCCESS;
        for (auto& pass : passes)
        {
            *outNosPassInfos++ = pass;
        }

        return NOS_RESULT_SUCCESS;
    }

    static nosResult GetShaderSource(nosShaderSource* outSpirvBuf)
    { 
        return NOS_RESULT_SUCCESS;
    }

    static nosResult CanCreateNode(const nosFbNode * node) 
    { 
        for (auto pin : *node->pins())
        {
            if (pin->name()->str() == "Device")
            {
                if (flatbuffers::IsFieldPresent(pin, nos::fb::Pin::VT_DATA))
                    return AJADevice::DeviceAvailable((char *)pin->data()->Data(),
                                                      node->class_name()->str() == "AJA.AJAIn") ? NOS_RESULT_SUCCESS : NOS_RESULT_FAILED;
                break;
            }
        }
		if (AJADevice::GetAvailableDevice(node->class_name()->str() == "AJA.AJAIn"))
        {
            return NOS_RESULT_SUCCESS;
        }
        return NOS_RESULT_FAILED;
    }
    
    static void OnNodeCreated(const nosFbNode * inNode, void** ctx) 
    { 
        auto& node = *inNode;
        AJADevice::Init();
        const bool isIn = node.class_name()->str() == "AJA.AJAIn";
        AJADevice *dev = 0;

        if (auto devpin = std::find_if(node.pins()->begin(), node.pins()->end(),
                                       [](auto pin) {
                                           return pin->name()->str() == "Device" &&
                                                  flatbuffers::IsFieldPresent(pin, fb::Pin::VT_DATA);
                                       });
            devpin != node.pins()->end())
        {
            dev = AJADevice::GetDevice((char *)devpin->data()->Data()).get();
        }
        else
            AJADevice::GetAvailableDevice(isIn, &dev);

        if (!dev)
        {
            // This case shouldn't happen with canCreate
            AJADevice::Deinit();
            return;
        }

        for (auto dev : AJADevice::Devices)
        {
            const auto str = dev->GetDisplayName() + "-AJAOut-Reference-Source";
            flatbuffers::FlatBufferBuilder fbb;
            std::vector<std::string> list{"Reference In", "Free Run"};
            for (int i = 1; i <= NTV2DeviceGetNumVideoInputs(dev->ID); ++i)
            {
                list.push_back("SDI In " + std::to_string(i));
            }
            
            HandleEvent(CreateAppEvent(
                fbb, nos::app::CreateUpdateStringList(fbb, nos::fb::CreateStringList(fbb, fbb.CreateString(str), fbb.CreateVectorOfStrings(list)))));
        }

        AJAClient *c = new AJAClient(isIn, dev);
        *ctx = c;

        std::string refSrc = "Ref : " + NTV2ReferenceSourceToString(c->Ref, true) + " - " + NTV2FrameRateToString(c->FR, true);
        flatbuffers::FlatBufferBuilder fbb;

        std::vector<flatbuffers::Offset<nos::fb::Pin>> pinsToAdd;
        std::vector<::flatbuffers::Offset<nos::PartialPinUpdate>> pinsToUpdate;
        using nos::fb::ShowAs;
        using nos::fb::CanShowAs;

        PinMapping mapping;
        auto loadedPins = mapping.Load(node);

        AddIfNotFound(NSN_Device,
					  "string",
					  StringValue(dev->GetDisplayName()),
					  loadedPins,
					  pinsToAdd, pinsToUpdate,
					  fbb,
                      ShowAs::PROPERTY, CanShowAs::PROPERTY_ONLY);
		if (auto val = AddIfNotFound(NSN_Dispatch_Size,
									 "nos.fb.vec2u",
									 nos::Buffer::From(nos::fb::vec2u(c->DispatchSizeX, c->DispatchSizeY)),
                                     loadedPins, pinsToAdd, pinsToUpdate, fbb))
        {
            c->DispatchSizeX = ((glm::uvec2 *)val)->x;
            c->DispatchSizeY = ((glm::uvec2 *)val)->y;
        }

        if (auto val = AddIfNotFound(
				NSN_Shader_Type, "AJA.Shader", nos::Buffer::From(ShaderType(c->Shader)), loadedPins,
                                     pinsToAdd, pinsToUpdate, fbb))
        {
            c->Shader = *((ShaderType *)val);
        }

        if (auto val =
				AddIfNotFound(NSN_Debug, "uint", nos::Buffer::From(u32(c->Debug)), loadedPins, pinsToAdd, pinsToUpdate, fbb))
        {
            c->Debug = *((u32 *)val);
        }

        if (!isIn)
        {
            nos::fb::TVisualizer vis = { .type = nos::fb::VisualizerType::COMBO_BOX, .name = dev->GetDisplayName() + "-AJAOut-Reference-Source" };
            if (auto ref = AddIfNotFound(NSN_ReferenceSource,
                "string",
                StringValue(refSrc),
                loadedPins,
                pinsToAdd, pinsToUpdate,
                fbb,
                ShowAs::PROPERTY, CanShowAs::PROPERTY_ONLY, vis))
            {
                refSrc = (char *)ref;
            }
        }
        c->SetReference(refSrc);

        std::vector<flatbuffers::Offset<nos::fb::NodeStatusMessage>> msg;
        std::vector<nos::fb::UUID> pinsToDel;
        c->OnNodeUpdate(std::move(mapping), loadedPins, pinsToDel);
        c->UpdateStatus(fbb, msg);
        HandleEvent(
            CreateAppEvent(fbb, nos::CreatePartialNodeUpdateDirect(fbb, &c->Mapping.NodeId, ClearFlags::NONE, &pinsToDel,
                                                                  &pinsToAdd, 0, 0, 0, 0, &msg, &pinsToUpdate)));
    }


    static void OnNodeUpdated(void* ctx, const nosFbNode * node) 
    {
        ((AJAClient *)ctx)->OnNodeUpdate(*node);
    }
    
    static void OnNodeDeleted(void* ctx, const nosUUID nodeId) 
    { 
        auto c = ((AJAClient *)ctx);
        c->OnNodeRemoved();
        delete c;
    }
    static void OnPinValueChanged(void* ctx, const nosName pinName, const nosUUID pinId, nosBuffer * value)
    { 
        return ((AJAClient *)ctx)->OnPinValueChanged(pinName, value->Data);
    }
    static void OnPathCommand(void* ctx, const nosPathCommand* cmd)
    { 
        auto aja = ((AJAClient *)ctx);
        aja->OnPathCommand(cmd);
    }
    static void OnPinConnected(void* ctx, const nosName pinName, nosUUID connectedPin, nosUUID nodeid)
    {
        ((AJAClient*)ctx)->OnPinConnected(pinName);
    }
    static void OnPinDisconnected(void* ctx, const nosName pinName)
    {
        ((AJAClient*)ctx)->OnPinDisconnected(pinName);
    }
    
    static nosResult CanRemoveOrphanPin(void* ctx, nosName pinName, nosUUID pinId)
    {
        return ((AJAClient*)ctx)->CanRemoveOrphanPin(pinName, pinId) ? NOS_RESULT_SUCCESS : NOS_RESULT_FAILED;
    }

    static nosResult OnOrphanPinRemoved(void* ctx, nosName pinName, nosUUID pinId)
    {
        return ((AJAClient*)ctx)->OnOrphanPinRemoved(pinName, pinId) ? NOS_RESULT_SUCCESS : NOS_RESULT_FAILED;
    }

    static void PathRestart(void* ctx, const nosNodeExecuteArgs* nodeArgs, const nosNodeExecuteArgs* functionArgs)
    {
        auto aja = ((AJAClient *)ctx);
        for (auto& [_,th] : aja->Pins)
            th->NotifyRestart({});
    }

    static nosResult GetFunctions(size_t * outCount, nosName* outName, nosPfnNodeFunctionExecute* outFunction) 
    {
        *outCount = 1;
        if(!outName || !outFunction)
            return NOS_RESULT_SUCCESS;
        outFunction[0] = PathRestart;
        outName[0] = NOS_NAME_STATIC("PathRestart");
        return NOS_RESULT_SUCCESS;
    }
    static nosResult  ExecuteNode(void* ctx, const nosNodeExecuteArgs * args) { return NOS_RESULT_SUCCESS; }
    static nosResult  CanCopy(void* ctx, nosCopyInfo * copyInfo)
    { 
        return NOS_RESULT_SUCCESS;
    }

    static nosResult  BeginCopyFrom(void* ctx, nosCopyInfo * cpy)
    { 
        if (((AJAClient*)ctx)->BeginCopyFrom(*cpy))
        {
            return NOS_RESULT_SUCCESS;
        }
        return NOS_RESULT_FAILED;
    }

    static nosResult  BeginCopyTo(void* ctx, nosCopyInfo * cpy)
    { 
        if (((AJAClient*)ctx)->BeginCopyTo(*cpy))
        {
            return NOS_RESULT_SUCCESS;
        }
        return NOS_RESULT_FAILED;
    }

    static void  EndCopyFrom(void* ctx, nosCopyInfo * cpy)
    { 
        return ((AJAClient *)ctx)->EndCopyFrom(*cpy);
    }

    static void  EndCopyTo(void* ctx, nosCopyInfo * cpy)
    { 
        return ((AJAClient *)ctx)->EndCopyTo(*cpy);
    }

    static void OnMenuRequested(void* ctx, const nosContextMenuRequest * request) 
    { 
        ((AJAClient *)ctx)->OnMenuFired(*request);
    }

    static void OnMenuCommand(void* ctx,  nosUUID itemID, uint32_t cmd) 
    { 
        ((AJAClient *)ctx)->OnCommandFired(cmd);
    }

    static void OnKeyEvent(void* ctx, const nosKeyEvent * keyEvent) { }
};

extern "C"
{

NOSAPI_ATTR nosResult NOSAPI_CALL nosExportNodeFunctions(size_t* outSize, nosNodeFunctions** outList)
{
    *outSize = 2;
    if (!outList)
        return NOS_RESULT_SUCCESS;
    auto* ajaIn = outList[0];
    auto* ajaOut = outList[1];
    ajaIn->TypeName = NSN_AJA_AJAIn;
    ajaOut->TypeName = NSN_AJA_AJAOut;
    ajaIn->CanCreateNode = ajaOut->CanCreateNode = AJA::CanCreateNode;
    ajaIn->OnNodeCreated = ajaOut->OnNodeCreated = AJA::OnNodeCreated;
    ajaIn->OnNodeUpdated = ajaOut->OnNodeUpdated = AJA::OnNodeUpdated;
    ajaIn->OnNodeDeleted = ajaOut->OnNodeDeleted = AJA::OnNodeDeleted;
    ajaIn->OnPinConnected = ajaOut->OnPinConnected = AJA::OnPinConnected;
    ajaIn->OnPinDisconnected = ajaOut->OnPinDisconnected = AJA::OnPinDisconnected;
    ajaIn->OnPinValueChanged = ajaOut->OnPinValueChanged = AJA::OnPinValueChanged;
    ajaIn->OnPathCommand = ajaOut->OnPathCommand = AJA::OnPathCommand;
    ajaIn->GetFunctions = ajaOut->GetFunctions = AJA::GetFunctions;
    ajaIn->ExecuteNode = ajaOut->ExecuteNode = AJA::ExecuteNode;
    ajaIn->CanCopy = ajaOut->CanCopy = AJA::CanCopy;
    ajaIn->BeginCopyFrom = ajaOut->BeginCopyFrom = AJA::BeginCopyFrom;
    ajaIn->BeginCopyTo = ajaOut->BeginCopyTo = AJA::BeginCopyTo;
    ajaIn->EndCopyFrom = ajaOut->EndCopyFrom = AJA::EndCopyFrom;
    ajaIn->EndCopyTo = ajaOut->EndCopyTo = AJA::EndCopyTo;
    ajaIn->GetShaderSource = ajaOut->GetShaderSource = AJA::GetShaderSource;
    ajaIn->GetShaders = ajaOut->GetShaders = AJA::GetShaders;
    ajaIn->GetPasses = ajaOut->GetPasses = AJA::GetPasses;
    ajaIn->OnMenuRequested = ajaOut->OnMenuRequested = AJA::OnMenuRequested;
    ajaIn->OnMenuCommand = ajaOut->OnMenuCommand = AJA::OnMenuCommand;
    ajaIn->OnKeyEvent = ajaOut->OnKeyEvent = AJA::OnKeyEvent;
    ajaIn->CanRemoveOrphanPin = ajaOut->CanRemoveOrphanPin= AJA::CanRemoveOrphanPin;
    ajaIn->OnOrphanPinRemoved = ajaOut->OnOrphanPinRemoved = AJA::OnOrphanPinRemoved;

    GShaders = {
		{NSN_AJA_RGB2YCbCr_Compute_Shader, { NOS_SHADER_STAGE_COMP, "RGB2YCbCr.comp", {std::begin(RGB2YCbCr_comp_spv), std::end(RGB2YCbCr_comp_spv)}}},
        {NSN_AJA_YCbCr2RGB_Compute_Shader, { NOS_SHADER_STAGE_COMP, "YCbCr2RGB.comp", {std::begin(YCbCr2RGB_comp_spv), std::end(YCbCr2RGB_comp_spv)}}},
        {NSN_AJA_RGB2YCbCr_Shader,         { NOS_SHADER_STAGE_FRAG, "RGB2YCbCr.frag", {std::begin(RGB2YCbCr_frag_spv), std::end(RGB2YCbCr_frag_spv)}}},
        {NSN_AJA_YCbCr2RGB_Shader,         { NOS_SHADER_STAGE_FRAG, "YCbCr2RGB.frag", {std::begin(YCbCr2RGB_frag_spv), std::end(YCbCr2RGB_frag_spv)}}},
	};

    return NOS_RESULT_SUCCESS;
}

}

} // namespace nos
