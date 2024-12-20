/*
 * Copyright MediaZ Teknoloji A.S. All Rights Reserved.
 */

#pragma once

#include <Nodos/PluginAPI.h>
#include <Nodos/PluginHelpers.hpp>
#include <nosVulkanSubsystem/nosVulkanSubsystem.h>

#include "ntv2enums.h"
#include "ntv2utils.h"

extern nosVulkanSubsystem* nosVulkan;

NOS_REGISTER_NAME(Device)
NOS_REGISTER_NAME(ReferenceSource)

NOS_REGISTER_NAME(Colorspace);
NOS_REGISTER_NAME(Source);
NOS_REGISTER_NAME(Interlaced);
NOS_REGISTER_NAME(ssbo);
NOS_REGISTER_NAME(Output);

namespace nos::aja
{
inline nosVec2u GetDeltaSeconds(NTV2VideoFormat format, bool interlaced)
{
	NTV2FrameRate frameRate = GetNTV2FrameRateFromVideoFormat(format);
	nosVec2u deltaSeconds = { 1,50 };
	switch (frameRate)
	{
	case NTV2_FRAMERATE_6000:	deltaSeconds = { 1, 60 }; break;
	case NTV2_FRAMERATE_5994:	deltaSeconds = { 1001, 60000 }; break;
	case NTV2_FRAMERATE_3000:	deltaSeconds = { 1, 30 }; break;
	case NTV2_FRAMERATE_2997:	deltaSeconds = { 1001, 30000 }; break;
	case NTV2_FRAMERATE_2500:	deltaSeconds = { 1, 25 }; break;
	case NTV2_FRAMERATE_2400:	deltaSeconds = { 1, 24 }; break;
	case NTV2_FRAMERATE_2398:	deltaSeconds = { 1001, 24000 }; break;
	case NTV2_FRAMERATE_5000:	deltaSeconds = { 1, 50 }; break;
	case NTV2_FRAMERATE_4800:	deltaSeconds = { 1, 48 }; break;
	case NTV2_FRAMERATE_4795:	deltaSeconds = { 1001, 48000 }; break;
	case NTV2_FRAMERATE_12000:	deltaSeconds = { 1, 120 }; break;
	case NTV2_FRAMERATE_11988:	deltaSeconds = { 1001, 120000 }; break;
	case NTV2_FRAMERATE_1500:	deltaSeconds = { 1, 15 }; break;
	case NTV2_FRAMERATE_1498:	deltaSeconds = { 1001, 15000 }; break;
	default:					deltaSeconds = { 1, 50 }; break;
	}
	if (interlaced)
		deltaSeconds.y = deltaSeconds.y * 2;
	return deltaSeconds;
}
}