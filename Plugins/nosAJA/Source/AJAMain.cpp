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

NOS_INIT();
NOS_VULKAN_INIT();

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
			if (AJADevice::DeviceAvailable((char*)devpin->data()->Data(), input))
                dev = AJADevice::GetDevice((char *)devpin->data()->Data()).get();
        }
		if (!dev)
            AJADevice::GetAvailableDevice(input, &dev);

		for (auto [_, dev] : AJADevice::Devices)
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

        c->Init(node, dev);

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
    
    static void OnPathStop(void* ctx)
	{
		auto aja = ((AJAClient*)ctx);
		aja->OnPathStop();
	}
	static void OnPathStart(void* ctx)
	{
		auto aja = ((AJAClient*)ctx);
		aja->OnPathStart();
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
            th->NotifyRestart();
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
        return ((AJAClient*)ctx)->CopyFrom(*cpy);
    }

    static nosResult  CopyTo(void* ctx, nosCopyInfo * cpy)
    { 
        return ((AJAClient*)ctx)->CopyTo(*cpy);
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

    static void GetScheduleInfo(void* ctx, nosScheduleInfo* info)
	{
		((AJAClient*)ctx)->GetScheduleInfo(info);
	}
};

namespace aja
{

enum class Nodes : int
{
	AJAIn_Legacy,
	AJAOut_Legacy,
	DMAWrite,
	DMARead,
	WaitVBL,
	Output,
	Channel,
	Count
};

nosResult RegisterDMAWriteNode(nosNodeFunctions*);
nosResult RegisterDMAReadNode(nosNodeFunctions*);
nosResult RegisterWaitVBLNode(nosNodeFunctions*);
nosResult RegisterOutputNode(nosNodeFunctions*);
nosResult RegisterChannelNode(nosNodeFunctions*);

extern "C"
{

NOSAPI_ATTR nosResult NOSAPI_CALL nosExportNodeFunctions(size_t* outSize, nosNodeFunctions** outList)
{
	*outSize = static_cast<size_t>(Nodes::Count);
	if (!outList)
		return NOS_RESULT_SUCCESS;
	auto* ajaIn = outList[0];
	auto* ajaOut = outList[1];
	ajaIn->ClassName = NSN_AJA_AJAIn;
	ajaOut->ClassName = NSN_AJA_AJAOut;
	ajaIn->OnNodeCreated = AJA::OnNodeCreated<true>;
	ajaOut->OnNodeCreated = AJA::OnNodeCreated<false>;
	ajaIn->OnNodeUpdated = ajaOut->OnNodeUpdated = AJA::OnNodeUpdated;
	ajaIn->OnNodeDeleted = ajaOut->OnNodeDeleted = AJA::OnNodeDeleted;
	ajaIn->OnPinValueChanged = ajaOut->OnPinValueChanged = AJA::OnPinValueChanged;
	ajaIn->OnPathStop = ajaOut->OnPathStop = AJA::OnPathStop;
	ajaIn->OnPathStart = ajaOut->OnPathStart = AJA::OnPathStart;
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
	ajaOut->GetScheduleInfo = AJA::GetScheduleInfo;

	NOS_RETURN_ON_FAILURE(RequestVulkanSubsystem());

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
	NOS_RETURN_ON_FAILURE(nosVulkan->RegisterShaders(shaderInfos.size(), shaderInfos.data()))
	std::vector<nosPassInfo> passes =
	{
		{.Key = NSN_AJA_RGB2YCbCr_Compute_Pass, .Shader = NSN_AJA_RGB2YCbCr_Compute_Shader, .MultiSample = 1},
		{.Key = NSN_AJA_YCbCr2RGB_Compute_Pass, .Shader = NSN_AJA_YCbCr2RGB_Compute_Shader, .MultiSample = 1},
	};
	NOS_RETURN_ON_FAILURE(nosVulkan->RegisterPasses(passes.size(), passes.data()))
	
	NOS_RETURN_ON_FAILURE(RegisterDMAWriteNode(outList[(int)Nodes::DMAWrite]))
	NOS_RETURN_ON_FAILURE(RegisterWaitVBLNode(outList[(int)Nodes::WaitVBL]))
	NOS_RETURN_ON_FAILURE(RegisterOutputNode(outList[(int)Nodes::Output]))
	NOS_RETURN_ON_FAILURE(RegisterChannelNode(outList[(int)Nodes::Channel]))
	NOS_RETURN_ON_FAILURE(RegisterDMAReadNode(outList[(int)Nodes::DMARead]))
	return NOS_RESULT_SUCCESS;
}

}
}

} // namespace nos
