/*
 * Copyright MediaZ AS. All Rights Reserved.
 */

#pragma once

#include <Nodos/PluginAPI.h>
#include <Nodos/PluginHelpers.hpp>
#include <nosVulkanSubsystem/nosVulkanSubsystem.h>

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
