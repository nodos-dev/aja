// Copyright Nodos AS. All Rights Reserved.

#include "AJAMain.h"
#include "CopyThread.h"
#include "AJAClient.h"

#include "../Shaders/RGB2YCbCr.comp.spv.dat"
#include "../Shaders/YCbCr2RGB.comp.spv.dat"
// #include "../Shaders/RGB2YCbCr.frag.spv.dat"
// #include "../Shaders/YCbCr2RGB.frag.spv.dat"

#include <Nodos/PluginAPI.h>

using namespace nos;

nosVulkanSubsystem* nosVulkan = nullptr;

NOS_INIT();

NOS_REGISTER_NAME(Device);
NOS_REGISTER_NAME(ReferenceSource);
NOS_REGISTER_NAME(Debug);
NOS_REGISTER_NAME_SPACED(Dispatch_Size, "Dispatch Size");
NOS_REGISTER_NAME_SPACED(Shader_Type, "Shader Type");

NOS_REGISTER_NAME(AJA_RGB2YCbCr_Compute_Shader);
NOS_REGISTER_NAME(AJA_YCbCr2RGB_Compute_Shader);
NOS_REGISTER_NAME(AJA_RGB2YCbCr_Compute_Pass);
NOS_REGISTER_NAME(AJA_YCbCr2RGB_Compute_Pass);

NOS_REGISTER_NAME(Colorspace);
NOS_REGISTER_NAME(Source);
NOS_REGISTER_NAME(Interlaced);
NOS_REGISTER_NAME(ssbo);
NOS_REGISTER_NAME(Output);

NOS_REGISTER_NAME_SPACED(AJA_AJAIn, "nos.aja.AJAIn");
NOS_REGISTER_NAME_SPACED(AJA_AJAOut, "nos.aja.AJAOut");

namespace nos
{

struct AJA
{
    template<bool input>
    static nosResult CanCreateNode(const nosFbNode * node) 
    { 
        for (auto pin : *node->pins())
        {
            if (pin->name()->str() == "Device")
            {
                if (flatbuffers::IsFieldPresent(pin, nos::fb::Pin::VT_DATA))
                    return AJADevice::DeviceAvailable((char *)pin->data()->Data(),
                                                      input) ? NOS_RESULT_SUCCESS : NOS_RESULT_FAILED;
                break;
            }
        }
		if (AJADevice::GetAvailableDevice(input))
        {
            return NOS_RESULT_SUCCESS;
        }
        return NOS_RESULT_FAILED;
    }
    
    template<bool input>
    static void OnNodeCreated(const nosFbNode * inNode, void** ctx) 
    { 
        auto& node = *inNode;
        AJADevice::Init();
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
            AJADevice::GetAvailableDevice(input, &dev);

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

        AJAClient *c = new AJAClient(input, dev);
        *ctx = c;

        std::string refSrc = "Ref : " + NTV2ReferenceSourceToString(c->Ref, true) + " - " + NTV2FrameRateToString(c->FR, true);
        flatbuffers::FlatBufferBuilder fbb;

        std::vector<flatbuffers::Offset<nos::fb::Pin>> pinsToAdd;
        std::vector<::flatbuffers::Offset<nos::app::PartialPinUpdate>> pinsToUpdate;
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
				NSN_Shader_Type, "nos.aja.Shader", nos::Buffer::From(ShaderType(c->Shader)), loadedPins,
                                     pinsToAdd, pinsToUpdate, fbb))
        {
            c->Shader = *((ShaderType *)val);
        }

        if (auto val =
				AddIfNotFound(NSN_Debug, "uint", nos::Buffer::From(u32(c->Debug)), loadedPins, pinsToAdd, pinsToUpdate, fbb))
        {
            c->Debug = *((u32 *)val);
        }

        if (!input)
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
            CreateAppEvent(fbb, nos::app::CreatePartialNodeUpdateDirect(fbb, &c->Mapping.NodeId, app::ClearFlags::NONE, &pinsToDel,
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
    static void OnPinValueChanged(void* ctx, const nosName pinName, const nosUUID pinId, nosBuffer value)
    { 
        return ((AJAClient *)ctx)->OnPinValueChanged(pinName, value.Data);
    }
    static void OnPathCommand(void* ctx, const nosPathCommand* cmd)
    { 
        auto aja = ((AJAClient *)ctx);
        aja->OnPathCommand(cmd);
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

    static nosResult  CopyFrom(void* ctx, nosCopyInfo * cpy)
    { 
        if (((AJAClient*)ctx)->CopyFrom(*cpy))
        {
            return NOS_RESULT_SUCCESS;
        }
        return NOS_RESULT_FAILED;
    }

    static nosResult  CopyTo(void* ctx, nosCopyInfo * cpy)
    { 
        if (((AJAClient*)ctx)->CopyTo(*cpy))
        {
            return NOS_RESULT_SUCCESS;
        }
        return NOS_RESULT_FAILED;
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
    ajaIn->ClassName = NSN_AJA_AJAIn;
    ajaOut->ClassName = NSN_AJA_AJAOut;
	ajaIn->CanCreateNode = AJA::CanCreateNode<true>;
	ajaOut->CanCreateNode = AJA::CanCreateNode<false>;
	ajaIn->OnNodeCreated = AJA::OnNodeCreated<true>;
	ajaOut->OnNodeCreated = AJA::OnNodeCreated<false>;
    ajaIn->OnNodeUpdated = ajaOut->OnNodeUpdated = AJA::OnNodeUpdated;
    ajaIn->OnNodeDeleted = ajaOut->OnNodeDeleted = AJA::OnNodeDeleted;
    ajaIn->OnPinValueChanged = ajaOut->OnPinValueChanged = AJA::OnPinValueChanged;
    ajaIn->OnPathCommand = ajaOut->OnPathCommand = AJA::OnPathCommand;
    ajaIn->GetFunctions = ajaOut->GetFunctions = AJA::GetFunctions;
    ajaIn->ExecuteNode = ajaOut->ExecuteNode = AJA::ExecuteNode;
    ajaIn->CopyFrom = ajaOut->CopyFrom = AJA::CopyFrom;
    ajaIn->CopyTo = ajaOut->CopyTo = AJA::CopyTo;
    ajaIn->OnMenuRequested = ajaOut->OnMenuRequested = AJA::OnMenuRequested;
    ajaIn->OnMenuCommand = ajaOut->OnMenuCommand = AJA::OnMenuCommand;
    ajaIn->OnKeyEvent = ajaOut->OnKeyEvent = AJA::OnKeyEvent;
    ajaIn->CanRemoveOrphanPin = ajaOut->CanRemoveOrphanPin= AJA::CanRemoveOrphanPin;
    ajaIn->OnOrphanPinRemoved = ajaOut->OnOrphanPinRemoved = AJA::OnOrphanPinRemoved;

	nosEngine.RequestSubsystem(NOS_NAME_STATIC(NOS_VULKAN_SUBSYSTEM_NAME), NOS_VULKAN_SUBSYSTEM_VERSION_MAJOR, 0, (void**)&nosVulkan);

    fs::path root = nosEngine.Context->RootFolderPath;
    auto rgb2ycbcrPath = (root / ".." / "Shaders" / "RGB2YCbCr.comp").generic_string();
	auto ycbcr2rgbPath = (root / ".." / "Shaders" / "YCbCr2RGB.comp").generic_string();

    const std::vector<std::pair<Name, std::tuple<nosShaderStage, const char*, std::vector<u8>>>> shaders = {
		{NSN_AJA_RGB2YCbCr_Compute_Shader, { NOS_SHADER_STAGE_COMP, rgb2ycbcrPath.c_str(), {std::begin(RGB2YCbCr_comp_spv), std::end(RGB2YCbCr_comp_spv)}}},
        {NSN_AJA_YCbCr2RGB_Compute_Shader, { NOS_SHADER_STAGE_COMP, ycbcr2rgbPath.c_str(), {std::begin(YCbCr2RGB_comp_spv), std::end(YCbCr2RGB_comp_spv)}}},
	};

	std::vector<nosShaderInfo> shaderInfos;
	for (auto& [name, data] : shaders)
	{
		auto& [stage, path, spirv] = data;
		shaderInfos.push_back(nosShaderInfo{
			.Key = name,
			.Source = { 
				.Stage = stage,
				.GLSLPath = path,
				.SpirvBlob = { (void*)spirv.data(), spirv.size() } 
			},
		});
	}
	auto ret = nosVulkan->RegisterShaders(shaderInfos.size(), shaderInfos.data());
	if (NOS_RESULT_SUCCESS != ret)
		return ret;
    std::vector<nosPassInfo> passes =
    {
	    {.Key = NSN_AJA_RGB2YCbCr_Compute_Pass, .Shader = NSN_AJA_RGB2YCbCr_Compute_Shader, .MultiSample = 1},
	    {.Key = NSN_AJA_YCbCr2RGB_Compute_Pass, .Shader = NSN_AJA_YCbCr2RGB_Compute_Shader, .MultiSample = 1},
    };
	ret = nosVulkan->RegisterPasses(passes.size(), passes.data());
    return ret;
}

}

} // namespace nos
