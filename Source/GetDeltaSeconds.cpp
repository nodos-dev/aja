// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include <Nodos/Plugin.hpp>

// External
#include <nosSysVulkan/nosVulkanSubsystem.h>
#include <Nodos/Utils/Stopwatch.hpp>

#include "nosAja/AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"

namespace nos::aja
{

struct GetDeltaSecondsNodeContext : NodeContext
{
	using NodeContext::NodeContext;

	nosResult OnCreate(nosFbNodePtr node) override
	{
		return NOS_RESULT_SUCCESS;
	}

	nosResult ExecuteNode(NodeExecuteParams const& params) override
	{
		auto& channel = *params.GetPinData<aja::ChannelInfo>(NOS_NAME("Channel"));
		auto delta = GetDeltaSeconds(static_cast<NTV2VideoFormat>(channel.video_format_idx()), channel.is_interlaced());
		SetPinValue(NOS_NAME("DeltaSeconds"), nos::Buffer::From(delta));
		return NOS_RESULT_SUCCESS;
	}
};

nosResult RegisterGetDeltaSecondsNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.GetDeltaSeconds"), GetDeltaSecondsNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}
}