// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include "AJAMain.h"
#include "AJADevice.h"

#include <Nodos/PluginAPI.h>

#include <nosDeviceSubsystem/nosDeviceSubsystem.h>
#include <nosSync/nosSync.h>

NOS_INIT()
NOS_VULKAN_INIT()
NOS_DEVICE_SUBSYSTEM_INIT()
NOS_SYNC_INIT()

NOS_BEGIN_IMPORT_DEPS()
	NOS_VULKAN_IMPORT()
	NOS_DEVICE_SUBSYSTEM_IMPORT()
	NOS_SYNC_IMPORT()
NOS_END_IMPORT_DEPS()


namespace nos::aja
{
enum class Nodes : int
{
	DMAWrite,
	DMARead,
	WaitVBL,
	Channel,
	Input,
	Output,
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
		AJADevice::Init();

		NOS_RETURN_ON_FAILURE(RegisterDMAWriteNode(outList[(int)Nodes::DMAWrite]))
		NOS_RETURN_ON_FAILURE(RegisterWaitVBLNode(outList[(int)Nodes::WaitVBL]))
		NOS_RETURN_ON_FAILURE(RegisterChannelNode(outList[(int)Nodes::Channel]))
		NOS_RETURN_ON_FAILURE(RegisterDMAReadNode(outList[(int)Nodes::DMARead]))
		
		// TODO: Remove these when migration of class-named graphs become available in Nodos.
		*outList[(int)Nodes::Input] = nosNodeFunctions {
			.ClassName = NOS_NAME("nos.aja.Input"),
			.MigrateNode = MigrateInOutNodes
		};
		*outList[(int)Nodes::Output] = nosNodeFunctions{
			.ClassName = NOS_NAME("nos.aja.Output"),
			.MigrateNode = MigrateInOutNodes
		};
		return NOS_RESULT_SUCCESS;
	}

	static nosResult MigrateInOutNodes(nosFbNodePtr node, nosBuffer* outBuffer)
	{
		auto pluginVersion = node->plugin_version();
		bool needsMigration = !pluginVersion || pluginVersion->major() <= 2 && pluginVersion->minor() < 3;
		if (!needsMigration)
			return NOS_RESULT_SUCCESS;
		// In child nodes, search for Device pin and migrate it
		fb::TNode cur;
		node->UnPackTo(&cur);
		auto* graph = node->contents_as_Graph();
		if (!graph)
			return NOS_RESULT_SUCCESS;
		int i = -1;
		for (auto* childNode : *graph->nodes())
		{
			++i;
			auto* className = childNode->class_name();
			if (!className)
				continue;
			if (className->string_view().ends_with("aja.Channel"))
			{
				if (auto migrated = MigrateChannelNode(childNode))
				{
					cur.contents.AsGraph()->nodes[i] = std::make_unique<fb::TNode>(std::move(*migrated));
					continue;
				}
			}
		}
		auto nodeBuffer = nos::EngineBuffer::CopyFrom(cur);
		*outBuffer = nodeBuffer.Release();
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
