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

extern nos::Name NSN_Device;
extern nos::Name NSN_ReferenceSource;
extern nos::Name NSN_Debug;
extern nos::Name NSN_Dispatch_Size;
extern nos::Name NSN_Shader_Type;

extern nos::Name NSN_AJA_RGB2YCbCr_Compute_Shader;
extern nos::Name NSN_AJA_YCbCr2RGB_Compute_Shader;
extern nos::Name NSN_AJA_RGB2YCbCr_Shader;
extern nos::Name NSN_AJA_YCbCr2RGB_Shader;
extern nos::Name NSN_AJA_RGB2YCbCr_Compute_Pass;
extern nos::Name NSN_AJA_YCbCr2RGB_Compute_Pass;
extern nos::Name NSN_AJA_RGB2YCbCr_Pass;
extern nos::Name NSN_AJA_YCbCr2RGB_Pass;

extern nos::Name NSN_Colorspace;
extern nos::Name NSN_Source;
extern nos::Name NSN_Interlaced;
extern nos::Name NSN_ssbo;
extern nos::Name NSN_Output;

extern nos::Name NSN_AJA_AJAIn;
extern nos::Name NSN_AJA_AJAOut;

namespace nos::aja
{
inline auto GenerateID() 
{
	struct {
		nos::fb::UUID id; operator nos::fb::UUID *() { return &id;}
	} re {nos::GenerateUUID()}; 
	return re;
}

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