// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include "AJAMain.h"
#include "AJADevice.h"

#include <Nodos/PluginAPI.h>

NOS_INIT()
NOS_VULKAN_INIT()

NOS_BEGIN_IMPORT_DEPS()
	NOS_VULKAN_IMPORT()
NOS_END_IMPORT_DEPS()

NOS_REGISTER_NAME(Device);
NOS_REGISTER_NAME(ReferenceSource);
NOS_REGISTER_NAME(Debug);
NOS_REGISTER_NAME_SPACED(Dispatch_Size, "Dispatch Size");
NOS_REGISTER_NAME_SPACED(Shader_Type, "Shader Type");

NOS_REGISTER_NAME(Colorspace);
NOS_REGISTER_NAME(Source);
NOS_REGISTER_NAME(Interlaced);
NOS_REGISTER_NAME(ssbo);
NOS_REGISTER_NAME(Output);

namespace nos::aja
{
enum class Nodes : int
{
	DMAWrite,
	DMARead,
	WaitVBL,
	Channel,
	Count
};

nosResult RegisterDMAWriteNode(nosNodeFunctions*);
nosResult RegisterDMAReadNode(nosNodeFunctions*);
nosResult RegisterWaitVBLNode(nosNodeFunctions*);
nosResult RegisterChannelNode(nosNodeFunctions*);

struct AJAPluginFunctions : nos::PluginFunctions
{
	nosResult ExportNodeFunctions(size_t& outSize, nosNodeFunctions** outList) override
	{
		outSize = static_cast<size_t>(Nodes::Count);
		if (!outList)
			return NOS_RESULT_SUCCESS;

		AJADevice::AvailableDevices = AJADevice::EnumerateDevices();

		NOS_RETURN_ON_FAILURE(RegisterDMAWriteNode(outList[(int)Nodes::DMAWrite]))
		NOS_RETURN_ON_FAILURE(RegisterWaitVBLNode(outList[(int)Nodes::WaitVBL]))
		NOS_RETURN_ON_FAILURE(RegisterChannelNode(outList[(int)Nodes::Channel]))
		NOS_RETURN_ON_FAILURE(RegisterDMAReadNode(outList[(int)Nodes::DMARead]))
		return NOS_RESULT_SUCCESS;
	}

	nosResult OnPreUnloadPlugin() override
	{
		AJADevice::Deinit();
		return NOS_RESULT_SUCCESS;
	}

};
NOS_EXPORT_PLUGIN_FUNCTIONS(AJAPluginFunctions)
} // namespace nos::aja
