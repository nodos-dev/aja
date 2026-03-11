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
	ChannelIP,
	Input,
	Output,
	Count
};

nosResult RegisterDMAWriteNode(nosNodeFunctions*);
nosResult RegisterDMAReadNode(nosNodeFunctions*);
nosResult RegisterWaitVBLNode(nosNodeFunctions*);
nosResult RegisterChannelNode(nosNodeFunctions*);
nosResult RegisterIPVideoChannelNode(nosNodeFunctions*);

struct AJAPluginFunctions : nos::PluginFunctions
{
	nosResult Initialize() override
	{
		return NOS_RESULT_SUCCESS;
	}
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
		NOS_RETURN_ON_FAILURE(RegisterIPVideoChannelNode(outList[(int)Nodes::ChannelIP]))
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
		auto isVersionLessThan = [](auto* version, int major, int minor) -> bool
		{
			if (!version)
				return true;
			if (version->major() != major)
				return version->major() < major;
			return version->minor() < minor;
		};

		bool needsChannelMigration = !pluginVersion || pluginVersion->major() <= 2 && pluginVersion->minor() < 3;
		bool isOutputNode = node->class_name() && node->class_name()->string_view().ends_with("aja.Output");
		bool needsNodeStatusPortalMigration = isVersionLessThan(pluginVersion, 2, 8);

		if (!needsChannelMigration && !needsNodeStatusPortalMigration)
			return NOS_RESULT_SUCCESS;

		fb::TNode cur;
		node->UnPackTo(&cur);

		bool migrated = false;

		// In child nodes, search for Device pin and migrate it.
		if (needsChannelMigration)
		{
			auto* graph = node->contents_as_Graph();
			if (graph && graph->nodes())
			{
				int i = -1;
				for (auto* childNode : *graph->nodes())
				{
					++i;
					auto* className = childNode->class_name();
					if (!className)
						continue;
					if (className->string_view().ends_with("aja.Channel"))
					{
						if (auto migratedNode = MigrateChannelNode(childNode))
						{
							cur.contents.AsGraph()->nodes[i] = std::make_unique<fb::TNode>(std::move(*migratedNode));
							migrated = true;
						}
					}
				}
			}
		}

		if (needsNodeStatusPortalMigration)
		{
			constexpr auto key = "NodeStatusPortal";
			auto value = isOutputNode ? "/Auto Resize/ShowStatus;/Channel;/ShowWarningIfInterlacing/ShowStatus;/BufferRing;/WaitVBL" : "/Channel;/WaitVBL";
			for (auto& metadata : cur.meta_data_map)
			{
				if (!metadata || metadata->key != key)
					continue;
				if (metadata->value != value)
				{
					metadata->value = value;
					migrated = true;
				}
				break;
			}
		}

		if (!migrated)
			return NOS_RESULT_SUCCESS;

		auto nodeBuffer = nos::EngineBuffer::CopyFrom(cur);
		*outBuffer = nodeBuffer.Release();
		return NOS_RESULT_SUCCESS;
	}

	nosResult OnPreUnloadPlugin() override
	{
		AJADevice::Deinit();
		nosSync->UnregisterEventGroup(1);
		return NOS_RESULT_SUCCESS;
	}

};
NOS_EXPORT_PLUGIN_FUNCTIONS(AJAPluginFunctions)
} // namespace nos::aja
