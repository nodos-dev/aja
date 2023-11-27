/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31.cpp
	@brief		Implementation of non-device-specific M31 class methods.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/

#include "ntv2m31.h"

struct M31PresetData
{
	M31VideoPreset preset;
	NTV2VideoFormat videoFormat;
	NTV2FrameBufferFormat frameBufferFormat;
	bool vif;
	bool uhd;
	bool interlaced;
	uint32_t pixelsPerLine;
	uint32_t linesPerFrame;
};

static const struct M31PresetData kM31PresetData[M31_NUMVIDEOPRESETS] =
{
	{ M31_FILE_720X480_420_8_5994i,			NTV2_FORMAT_525_5994,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	true,	720,	480 },	// 0
	{ M31_FILE_720X480_420_8_5994p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	720,	480 },	// 1
	{ M31_FILE_720X480_420_8_60i,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	true,	720,	480 },	// 2
	{ M31_FILE_720X480_420_8_60p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	720,	480 },	// 3
	{ M31_FILE_720X480_422_10_5994i,		NTV2_FORMAT_525_5994,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	true,	720,	480 },	// 4
	{ M31_FILE_720X480_422_10_5994p,		NTV2_FORMAT_UNKNOWN,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	720,	480 },	// 5
	{ M31_FILE_720X480_422_10_60i,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	true,	720,	480 },	// 6
	{ M31_FILE_720X480_422_10_60p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	720,	480 },	// 7

	{ M31_FILE_720X576_420_8_50i,			NTV2_FORMAT_625_5000,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	true,	720,	576 },	// 8
	{ M31_FILE_720X576_420_8_50p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	720,	576 },	// 9
	{ M31_FILE_720X576_422_10_50i,			NTV2_FORMAT_625_5000,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	true,	720,	576 },	// 10
	{ M31_FILE_720X576_422_10_50p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	720,	576 },	// 11
	
	{ M31_FILE_1280X720_420_8_2398p,		NTV2_FORMAT_720p_2398,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1280,	720 },	// 12
	{ M31_FILE_1280X720_420_8_24p,			NTV2_FORMAT_720p_2398,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1280,	720 },	// 13
	{ M31_FILE_1280X720_420_8_25p,			NTV2_FORMAT_720p_2500,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1280,	720 },	// 14
	{ M31_FILE_1280X720_420_8_2997p,		NTV2_FORMAT_720p_2500,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1280,	720 },	// 15
	{ M31_FILE_1280X720_420_8_30p,			NTV2_FORMAT_720p_2500,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1280,	720 },	// 16
	{ M31_FILE_1280X720_420_8_50p,			NTV2_FORMAT_720p_5000,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1280,	720 },	// 17
	{ M31_FILE_1280X720_420_8_5994p,		NTV2_FORMAT_720p_5994,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1280,	720 },	// 18
	{ M31_FILE_1280X720_420_8_60p,			NTV2_FORMAT_720p_6000,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1280,	720 },	// 19
	
	{ M31_FILE_1280X720_422_10_2398p,		NTV2_FORMAT_720p_2398,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1280,	720 },	// 20
	{ M31_FILE_1280X720_422_10_24p,			NTV2_FORMAT_720p_2398,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1280,	720 },	// 21
	{ M31_FILE_1280X720_422_10_25p,			NTV2_FORMAT_720p_2500,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1280,	720 },	// 22
	{ M31_FILE_1280X720_422_10_2997p,		NTV2_FORMAT_720p_2500,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1280,	720 },	// 23
	{ M31_FILE_1280X720_422_10_30p,			NTV2_FORMAT_720p_2500,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1280,	720 },	// 24
	{ M31_FILE_1280X720_422_10_50p,			NTV2_FORMAT_720p_5000,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1280,	720 },	// 25
	{ M31_FILE_1280X720_422_10_5994p,		NTV2_FORMAT_720p_5994,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1280,	720 },	// 26
	{ M31_FILE_1280X720_422_10_60p,			NTV2_FORMAT_720p_6000,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1280,	720 },	// 27
	
	{ M31_FILE_1920X1080_420_8_2398p,		NTV2_FORMAT_1080p_2398,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1920,	1080 }, // 28
	{ M31_FILE_1920X1080_420_8_24p,			NTV2_FORMAT_1080p_2400,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1920,	1080 }, // 29
	{ M31_FILE_1920X1080_420_8_25p,			NTV2_FORMAT_1080p_2500,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1920,	1080 }, // 30
	{ M31_FILE_1920X1080_420_8_2997p,		NTV2_FORMAT_1080p_2997,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1920,	1080 }, // 31
	{ M31_FILE_1920X1080_420_8_30p,			NTV2_FORMAT_1080p_3000,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1920,	1080 }, // 32
	{ M31_FILE_1920X1080_420_8_50i,			NTV2_FORMAT_1080i_5000,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	true,	1920,	1080 }, // 33
	{ M31_FILE_1920X1080_420_8_50p,			NTV2_FORMAT_1080p_5000_A,		NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1920,	1080 }, // 34
	{ M31_FILE_1920X1080_420_8_5994i,		NTV2_FORMAT_1080i_5994,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	true,	1920,	1080 }, // 35
	{ M31_FILE_1920X1080_420_8_5994p,		NTV2_FORMAT_1080p_5994_A,		NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1920,	1080 }, // 36
	{ M31_FILE_1920X1080_420_8_60i,			NTV2_FORMAT_1080i_6000,			NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	true,	1920,	1080 }, // 37
	{ M31_FILE_1920X1080_420_8_60p,			NTV2_FORMAT_1080p_6000_A,		NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	1920,	1080 }, // 38
	
	{ M31_FILE_1920X1080_422_10_2398p,		NTV2_FORMAT_1080p_2398,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1920,	1080 }, // 39
	{ M31_FILE_1920X1080_422_10_24p,		NTV2_FORMAT_1080p_2400,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1920,	1080 }, // 40
	{ M31_FILE_1920X1080_422_10_25p,		NTV2_FORMAT_1080p_2500,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1920,	1080 }, // 41
	{ M31_FILE_1920X1080_422_10_2997p,		NTV2_FORMAT_1080p_2997,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1920,	1080 }, // 42
	{ M31_FILE_1920X1080_422_10_30p,		NTV2_FORMAT_1080p_3000,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1920,	1080 }, // 43
	{ M31_FILE_1920X1080_422_10_50i,		NTV2_FORMAT_1080i_5000,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	true,	1920,	1080 }, // 44
	{ M31_FILE_1920X1080_422_10_50p,		NTV2_FORMAT_1080p_5000_A,		NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1920,	1080 }, // 45
	{ M31_FILE_1920X1080_422_10_5994i,		NTV2_FORMAT_1080i_5994,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	true,	1920,	1080 }, // 46
	{ M31_FILE_1920X1080_422_10_5994p,		NTV2_FORMAT_1080p_5994_A,		NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1920,	1080 }, // 47
	{ M31_FILE_1920X1080_422_10_60i,		NTV2_FORMAT_1080i_6000,			NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	true,	1920,	1080 }, // 48
	{ M31_FILE_1920X1080_422_10_60p,		NTV2_FORMAT_1080p_6000_A,		NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	1920,	1080 }, // 49

	{ M31_FILE_2048X1080_420_8_2398p,		NTV2_FORMAT_1080p_2K_2398,		NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	2048,	1080 }, // 50
	{ M31_FILE_2048X1080_420_8_24p,			NTV2_FORMAT_1080p_2K_2400,		NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	2048,	1080 }, // 51
	{ M31_FILE_2048X1080_420_8_25p,			NTV2_FORMAT_1080p_2K_2500,		NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	2048,	1080 }, // 52
	{ M31_FILE_2048X1080_420_8_2997p,		NTV2_FORMAT_1080p_2K_2997,		NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	2048,	1080 }, // 53
	{ M31_FILE_2048X1080_420_8_30p,			NTV2_FORMAT_1080p_2K_3000,		NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	2048,	1080 }, // 54
	{ M31_FILE_2048X1080_420_8_50p,			NTV2_FORMAT_1080p_2K_5000_A,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	2048,	1080 }, // 55
	{ M31_FILE_2048X1080_420_8_5994p,		NTV2_FORMAT_1080p_2K_5994_A,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	2048,	1080 }, // 56
	{ M31_FILE_2048X1080_420_8_60p,			NTV2_FORMAT_1080p_2K_6000_A,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	false,	false,	2048,	1080 }, // 57
	
	{ M31_FILE_2048X1080_422_10_2398p,		NTV2_FORMAT_1080p_2K_2398,		NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	2048,	1080 }, // 58
	{ M31_FILE_2048X1080_422_10_24p,		NTV2_FORMAT_1080p_2K_2400,		NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	2048,	1080 }, // 59
	{ M31_FILE_2048X1080_422_10_25p,		NTV2_FORMAT_1080p_2K_2500,		NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	2048,	1080 }, // 60
	{ M31_FILE_2048X1080_422_10_2997p,		NTV2_FORMAT_1080p_2K_2997,		NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	2048,	1080 }, // 61
	{ M31_FILE_2048X1080_422_10_30p,		NTV2_FORMAT_1080p_2K_3000,		NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	2048,	1080 }, // 62
	{ M31_FILE_2048X1080_422_10_50p,		NTV2_FORMAT_1080p_2K_5000_A,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	2048,	1080 }, // 63
	{ M31_FILE_2048X1080_422_10_5994p,		NTV2_FORMAT_1080p_2K_5994_A,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	2048,	1080 }, // 64
	{ M31_FILE_2048X1080_422_10_60p,		NTV2_FORMAT_1080p_2K_6000_A,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	false,	false,	2048,	1080 }, // 65

	{ M31_FILE_3840X2160_420_8_2398p,		NTV2_FORMAT_4x1920x1080p_2398,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	true,	false,	3840,	2160 }, // 66
	{ M31_FILE_3840X2160_420_8_24p,			NTV2_FORMAT_4x1920x1080p_2400,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	true,	false,	3840,	2160 }, // 67
	{ M31_FILE_3840X2160_420_8_25p,			NTV2_FORMAT_4x1920x1080p_2500,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	true,	false,	3840,	2160 }, // 68
	{ M31_FILE_3840X2160_420_8_2997p,		NTV2_FORMAT_4x1920x1080p_2997,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	true,	false,	3840,	2160 }, // 69
	{ M31_FILE_3840X2160_420_8_30p,			NTV2_FORMAT_4x1920x1080p_3000,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	true,	false,	3840,	2160 }, // 70
	{ M31_FILE_3840X2160_420_8_50p,			NTV2_FORMAT_4x1920x1080p_5000,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	true,	false,	3840,	2160 }, // 71
	{ M31_FILE_3840X2160_420_8_5994p,		NTV2_FORMAT_4x1920x1080p_5994,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	true,	false,	3840,	2160 }, // 72
	{ M31_FILE_3840X2160_420_8_60p,			NTV2_FORMAT_4x1920x1080p_6000,	NTV2_FBF_8BIT_YCBCR_420PL2,		false,	true,	false,	3840,	2160 }, // 73
	{ M31_FILE_3840X2160_420_10_50p,		NTV2_FORMAT_4x1920x1080p_5000,	NTV2_FBF_10BIT_YCBCR_420PL2,	false,	true,	false,	3840,	2160 }, // 74
	{ M31_FILE_3840X2160_420_10_5994p,		NTV2_FORMAT_4x1920x1080p_5994,	NTV2_FBF_10BIT_YCBCR_420PL2,	false,	true,	false,	3840,	2160 }, // 75
	{ M31_FILE_3840X2160_420_10_60p,		NTV2_FORMAT_4x1920x1080p_6000,	NTV2_FBF_10BIT_YCBCR_420PL2,	false,	true,	false,	3840,	2160 }, // 76
   
	{ M31_FILE_3840X2160_422_8_2398p,		NTV2_FORMAT_4x1920x1080p_2398,	NTV2_FBF_8BIT_YCBCR_422PL2,		false,	true,	false,	3840,	2160 }, // 77
	{ M31_FILE_3840X2160_422_8_24p,			NTV2_FORMAT_4x1920x1080p_2400,	NTV2_FBF_8BIT_YCBCR_422PL2,		false,	true,	false,	3840,	2160 }, // 78
	{ M31_FILE_3840X2160_422_8_25p,			NTV2_FORMAT_4x1920x1080p_2500,	NTV2_FBF_8BIT_YCBCR_422PL2,		false,	true,	false,	3840,	2160 }, // 79
	{ M31_FILE_3840X2160_422_8_2997p,		NTV2_FORMAT_4x1920x1080p_2997,	NTV2_FBF_8BIT_YCBCR_422PL2,		false,	true,	false,	3840,	2160 }, // 80
	{ M31_FILE_3840X2160_422_8_30p,			NTV2_FORMAT_4x1920x1080p_3000,	NTV2_FBF_8BIT_YCBCR_422PL2,		false,	true,	false,	3840,	2160 }, // 81
	{ M31_FILE_3840X2160_422_8_50p,			NTV2_FORMAT_4x1920x1080p_5000,	NTV2_FBF_8BIT_YCBCR_422PL2,		false,	true,	false,	3840,	2160 }, // 82
	{ M31_FILE_3840X2160_422_8_5994p,		NTV2_FORMAT_4x1920x1080p_5994,	NTV2_FBF_8BIT_YCBCR_422PL2,		false,	true,	false,	3840,	2160 }, // 83
	{ M31_FILE_3840X2160_422_8_60p,			NTV2_FORMAT_4x1920x1080p_6000,	NTV2_FBF_8BIT_YCBCR_422PL2,		false,	true,	false,	3840,	2160 }, // 84
	
	{ M31_FILE_3840X2160_422_10_2398p,		NTV2_FORMAT_4x1920x1080p_2398,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,	false,	3840,	2160 }, // 85
	{ M31_FILE_3840X2160_422_10_24p,		NTV2_FORMAT_4x1920x1080p_2400,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,	false,	3840,	2160 }, // 86
	{ M31_FILE_3840X2160_422_10_25p,		NTV2_FORMAT_4x1920x1080p_2500,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,	false,	3840,	2160 }, // 87
	{ M31_FILE_3840X2160_422_10_2997p,		NTV2_FORMAT_4x1920x1080p_2997,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,	false,	3840,	2160 }, // 88
	{ M31_FILE_3840X2160_422_10_30p,		NTV2_FORMAT_4x1920x1080p_3000,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,	false,	3840,	2160 }, // 89
	{ M31_FILE_3840X2160_422_10_50p,		NTV2_FORMAT_4x1920x1080p_5000,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,	false,	3840,	2160 }, // 90
	{ M31_FILE_3840X2160_422_10_5994p,		NTV2_FORMAT_4x1920x1080p_5994,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,	false,	3840,	2160 }, // 91
	{ M31_FILE_3840X2160_422_10_60p,		NTV2_FORMAT_4x1920x1080p_6000,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,	false,	3840,	2160 }, // 92

	{ M31_FILE_4096X2160_420_10_5994p,		NTV2_FORMAT_4x2048x1080p_5994,	NTV2_FBF_10BIT_YCBCR_420PL2,	false,	true,  false,	4096,	2160 }, // 93
	{ M31_FILE_4096X2160_420_10_60p,		NTV2_FORMAT_4x2048x1080p_6000,	NTV2_FBF_10BIT_YCBCR_420PL2,	false,	true,  false,	4096,	2160 }, // 94
	{ M31_FILE_4096X2160_422_10_50p,		NTV2_FORMAT_4x2048x1080p_5000,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,  false,	4096,	2160 }, // 95
	{ M31_FILE_4096X2160_422_10_5994p_IF,	NTV2_FORMAT_4x2048x1080p_5994,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,  false,	4096,	2160 }, // 96
	{ M31_FILE_4096X2160_422_10_60p_IF,		NTV2_FORMAT_4x2048x1080p_6000,	NTV2_FBF_10BIT_YCBCR_422PL2,	false,	true,  false,	4096,	2160 }, // 97

	{ M31_VIF_720X480_420_8_5994i,			NTV2_FORMAT_525_5994,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	true,	720,	480 },	// 98
	{ M31_VIF_720X480_420_8_5994p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	false,	720,	480 },	// 99
	{ M31_VIF_720X480_420_8_60i,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	true,	720,	480 },	// 100
	{ M31_VIF_720X480_420_8_60p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	false,	720,	480 },	// 101
	{ M31_VIF_720X480_422_10_5994i,			NTV2_FORMAT_525_5994,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	true,	720,	480 },	// 102
	{ M31_VIF_720X480_422_10_5994p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	false,	720,	480 },	// 103
	{ M31_VIF_720X480_422_10_60i,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	true,	720,	480 },	// 104
	{ M31_VIF_720X480_422_10_60p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	false,	720,	480 },	// 105

	{ M31_VIF_720X576_420_8_50i,			NTV2_FORMAT_625_5000,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	true,	720,	576 },	// 106
	{ M31_VIF_720X576_420_8_50p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	false,	720,	576 },	// 107
	{ M31_VIF_720X576_422_10_50i,			NTV2_FORMAT_625_5000,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	true,	720,	576 },	// 108
	{ M31_VIF_720X576_422_10_50p,			NTV2_FORMAT_UNKNOWN,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	false,	720,	576 },	// 109

	{ M31_VIF_1280X720_420_8_50p,			NTV2_FORMAT_720p_5000,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	false,	1280,	720 },	// 110
	{ M31_VIF_1280X720_420_8_5994p,			NTV2_FORMAT_720p_5994,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	false,	1280,	720 },	// 111
	{ M31_VIF_1280X720_420_8_60p,			NTV2_FORMAT_720p_6000,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	false,	1280,	720 },	// 112
	{ M31_VIF_1280X720_422_10_50p,			NTV2_FORMAT_720p_5000,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	false,	1280,	720 },	// 113
	{ M31_VIF_1280X720_422_10_5994p,		NTV2_FORMAT_720p_5994,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	false,	1280,	720 },	// 114
	{ M31_VIF_1280X720_422_10_60p,			NTV2_FORMAT_720p_6000,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	false,	1280,	720 },	// 115

	{ M31_VIF_1920X1080_420_8_50i,			NTV2_FORMAT_1080i_5000,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	true,	1920,	1080 }, // 116
	{ M31_VIF_1920X1080_420_8_50p,			NTV2_FORMAT_1080p_5000_A,		NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	false,	1920,	1080 }, // 117
	{ M31_VIF_1920X1080_420_8_5994i,		NTV2_FORMAT_1080i_5994,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	true,	1920,	1080 }, // 118
	{ M31_VIF_1920X1080_420_8_5994p,		NTV2_FORMAT_1080p_5994_A,		NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	false,	1920,	1080 }, // 119
	{ M31_VIF_1920X1080_420_8_60i,			NTV2_FORMAT_1080i_6000,			NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	true,	1920,	1080 }, // 120
	{ M31_VIF_1920X1080_420_8_60p,			NTV2_FORMAT_1080p_6000_A,		NTV2_FBF_8BIT_YCBCR_420PL2,		true,	false,	false,	1920,	1080 }, // 121
	{ M31_VIF_1920X1080_420_10_50i,			NTV2_FORMAT_1080i_5000,			NTV2_FBF_10BIT_YCBCR_420PL2,	true,	false,	true,	1920,	1080 }, // 122
	{ M31_VIF_1920X1080_420_10_50p,			NTV2_FORMAT_1080p_5000_A,		NTV2_FBF_10BIT_YCBCR_420PL2,	true,	false,	false,	1920,	1080 }, // 123
	{ M31_VIF_1920X1080_420_10_5994i,		NTV2_FORMAT_1080i_5994,			NTV2_FBF_10BIT_YCBCR_420PL2,	true,	false,	true,	1920,	1080 }, // 124
	{ M31_VIF_1920X1080_420_10_5994p,		NTV2_FORMAT_1080p_5994_A,		NTV2_FBF_10BIT_YCBCR_420PL2,	true,	false,	false,	1920,	1080 }, // 125
	{ M31_VIF_1920X1080_420_10_60i,			NTV2_FORMAT_1080i_6000,			NTV2_FBF_10BIT_YCBCR_420PL2,	true,	false,	true,	1920,	1080 }, // 126
	{ M31_VIF_1920X1080_420_10_60p,			NTV2_FORMAT_1080p_6000_A,		NTV2_FBF_10BIT_YCBCR_420PL2,	true,	false,	false,	1920,	1080 }, // 127
	{ M31_VIF_1920X1080_422_10_5994i,		NTV2_FORMAT_1080i_5994,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	true,	1920,	1080 }, // 128
	{ M31_VIF_1920X1080_422_10_5994p,		NTV2_FORMAT_1080p_5994_A,		NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	false,	1920,	1080 }, // 129
	{ M31_VIF_1920X1080_422_10_60i,			NTV2_FORMAT_1080i_6000,			NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	true,	1920,	1080 }, // 130
	{ M31_VIF_1920X1080_422_10_60p,			NTV2_FORMAT_1080p_6000_A,		NTV2_FBF_10BIT_YCBCR_422PL2,	true,	false,	false,	1920,	1080 }, // 131

	{ M31_VIF_3840X2160_420_8_30p,			NTV2_FORMAT_4x1920x1080p_3000,	NTV2_FBF_8BIT_YCBCR_420PL2,		true,	true,	false,	3840,	2160 }, // 132
	{ M31_VIF_3840X2160_420_8_50p,			NTV2_FORMAT_4x1920x1080p_5000,	NTV2_FBF_8BIT_YCBCR_420PL2,		true,	true,	false,	3840,	2160 }, // 133
	{ M31_VIF_3840X2160_420_8_5994p,		NTV2_FORMAT_4x1920x1080p_5994,	NTV2_FBF_8BIT_YCBCR_420PL2,		true,	true,	false,	3840,	2160 }, // 134
	{ M31_VIF_3840X2160_420_8_60p,			NTV2_FORMAT_4x1920x1080p_6000,	NTV2_FBF_8BIT_YCBCR_420PL2,		true,	true,	false,	3840,	2160 }, // 135
	{ M31_VIF_3840X2160_420_10_50p,			NTV2_FORMAT_4x1920x1080p_5000,	NTV2_FBF_10BIT_YCBCR_420PL2,	true,	true,	false,	3840,	2160 }, // 136
	{ M31_VIF_3840X2160_420_10_5994p,		NTV2_FORMAT_4x1920x1080p_5994,	NTV2_FBF_10BIT_YCBCR_420PL2,	true,	true,	false,	3840,	2160 }, // 137
	{ M31_VIF_3840X2160_420_10_60p,			NTV2_FORMAT_4x1920x1080p_6000,	NTV2_FBF_10BIT_YCBCR_420PL2,	true,	true,	false,	3840,	2160 }, // 138
	{ M31_VIF_3840X2160_422_10_30p,			NTV2_FORMAT_4x1920x1080p_3000,	NTV2_FBF_10BIT_YCBCR_422PL2,	true,	true,	false,	3840,	2160 }, // 139
	{ M31_VIF_3840X2160_422_10_50p,			NTV2_FORMAT_4x1920x1080p_5000,	NTV2_FBF_10BIT_YCBCR_422PL2,	true,	true,	false,	3840,	2160 }, // 140
	{ M31_VIF_3840X2160_422_10_5994p,		NTV2_FORMAT_4x1920x1080p_5994,	NTV2_FBF_10BIT_YCBCR_422PL2,	true,	true,	false,	3840,	2160 }, // 141
	{ M31_VIF_3840X2160_422_10_60p,			NTV2_FORMAT_4x1920x1080p_6000,	NTV2_FBF_10BIT_YCBCR_422PL2,	true,	true,	false,	3840,	2160 }	// 142
};

static const M31PresetData* GetM31PresetData(M31VideoPreset preset)
{
	// check bounds
	if ((preset < 0) || (preset >= M31_NUMVIDEOPRESETS)) return NULL;

	// optimized if list is ordered
	if (kM31PresetData[preset].preset == preset)
	{
		return &kM31PresetData[preset];
	}

	// search if list not ordered
	for (int i = 0; i < M31_NUMVIDEOPRESETS; i++)
	{
		if (kM31PresetData[i].preset == preset)
		{
			return &kM31PresetData[i];
		}
	}

	// list not complete
	return NULL;
}


// Constructor
CNTV2m31::CNTV2m31(CNTV2Card* device) :
mpM31CParam (NULL),
mpM31VInParam (NULL),
mpM31VAParam (NULL),
mpM31EHParam (NULL)
{
	mpDevice = device;

	// For now lets allocate params classes in constructor
	mpM31CParam = new CNTV2m31CParam(mpDevice);
	assert (mpM31CParam);

	mpM31VIParam = new CNTV2m31VIParam(mpDevice);
	assert (mpM31VIParam);

	mpM31VInParam = new CNTV2m31VInParam(mpDevice);
	assert (mpM31VInParam);

	mpM31VAParam = new CNTV2m31VAParam(mpDevice);
	assert (mpM31VAParam);
	
	mpM31EHParam = new CNTV2m31EHParam(mpDevice);
	assert (mpM31EHParam);

}

// Destructor
CNTV2m31::~CNTV2m31()
{
	// Delete params classes
	if (mpM31CParam)
	{
		delete mpM31CParam;
		mpM31CParam = NULL;
	}
	
	if (mpM31VIParam)
	{
		delete mpM31VIParam;
		mpM31VIParam = NULL;
	}

	if (mpM31VInParam)
	{
		delete mpM31VInParam;
		mpM31VInParam = NULL;
	}
	
	if (mpM31VAParam)
	{
		delete mpM31VAParam;
		mpM31VAParam = NULL;
	}

	if (mpM31EHParam)
	{
		delete mpM31EHParam;
		mpM31EHParam = NULL;
	}
}

// Routines used to setup the M31

// Sets/load/clears for all params

bool CNTV2m31::ClearAllParams()
{
	if (!mpM31CParam->Clear()) return false;
	if (!mpM31VIParam->Clear()) return false;
	if (!mpM31VInParam->Clear()) return false;
	if (!mpM31VAParam->Clear()) return false;
	if (!mpM31EHParam->Clear()) return false;
	
	return true;
}


bool CNTV2m31::SetupAllParams(M31VideoPreset preset, M31Channel channel)
{
	if (!LoadAllParams(preset)) return false;
	if (!SetAllParams(channel)) return false;

	return true;
}

bool CNTV2m31::LoadAllParams(M31VideoPreset preset)
{
	if (!LoadCommonParams(preset)) return false;
	if (!LoadVIParams(preset)) return false;
	if (!LoadVInParams(preset)) return false;
	if (!LoadVAParams(preset)) return false;
	if (!LoadEHParams(preset)) return false;

	return true;
}

bool CNTV2m31::SetAllParams(M31Channel channel)
{
	if (!SetCommonParams(channel)) return false;
	if (!SetVIParams(channel)) return false;
	if (!SetVInParams(channel)) return false;
	if (!SetVAParams(channel)) return false;
	if (!SetEHParams(channel)) return false;

	return true;
}

// Sets/loads for Common params
bool CNTV2m31::SetupCommonParams(M31VideoPreset preset, M31Channel channel)
{
	if (!LoadCommonParams(preset)) return false;
	if (!SetCommonParams(channel)) return false;
	
	return true;
}

bool CNTV2m31::LoadCommonParams(M31VideoPreset preset)
{
	return mpM31CParam->LoadCParamsChannel(preset, &mCParamsChannel);
}

bool CNTV2m31::SetCommonParams(M31Channel channel)
{
	return mpM31CParam->SetCParamsChannel(mCParamsChannel, channel);
}

// Sets/loads for VI params
bool CNTV2m31::SetupVIParams(M31VideoPreset preset, M31Channel channel)
{
	if (!LoadVIParams(preset)) return false;
	if (!SetVIParams(channel)) return false;
	
	return true;
}

bool CNTV2m31::LoadVIParams(M31VideoPreset preset)
{
	return mpM31VIParam->LoadVIParamsChannel(preset, &mvIParamsChannel);
}

bool CNTV2m31::SetVIParams(M31Channel channel)
{
	return mpM31VIParam->SetVIParamsChannel(mvIParamsChannel, channel);
}

// Sets/loads for VIn params
bool CNTV2m31::SetupVInParams(M31VideoPreset preset, M31Channel channel)
{
	if (!LoadVInParams(preset)) return false;
	if (!SetVInParams(channel)) return false;
	
	return true;
}

bool CNTV2m31::LoadVInParams(M31VideoPreset preset)
{
	return mpM31VInParam->LoadVInParamsChannel(preset, &mVInParamsChannel);
}

bool CNTV2m31::SetVInParams(M31Channel channel)
{
	return mpM31VInParam->SetVInParamsChannel(mVInParamsChannel, (M31VirtualChannel)channel);
}

// Sets/loads for VA params
bool CNTV2m31::SetupVAParams(M31VideoPreset preset, M31Channel channel)
{
	if (!LoadVAParams(preset)) return false;
	if (!SetVAParams(channel)) return false;
	
	return true;
}

bool CNTV2m31::LoadVAParams(M31VideoPreset preset)
{
	return mpM31VAParam->LoadVAParamsChannel(preset, &mVAParamsChannel);
}

bool CNTV2m31::SetVAParams(M31Channel channel)
{
	return mpM31VAParam->SetVAParamsChannel(mVAParamsChannel, (M31VirtualChannel)channel);
}

// Sets/loads for EH params
bool CNTV2m31::SetupEHParams(M31VideoPreset preset, M31Channel channel)
{
	if (!LoadEHParams(preset)) return false;
	if (!SetEHParams(channel)) return false;
	
	return true;
}

bool CNTV2m31::LoadEHParams(M31VideoPreset preset)
{
	return mpM31EHParam->LoadEHParamsChannel(preset, &mEHParamsChannel);
}

bool CNTV2m31::SetEHParams(M31Channel channel)
{
	return mpM31EHParam->SetEHParamsChannel(mEHParamsChannel, (M31VirtualChannel)channel);
}


// Routines used to control the M31

bool CNTV2m31::ChangeMainState(HevcMainState mainState ,
							   HevcEncodeMode encodeMode,
							   HevcFirmwareType firmwareType)
{
	bool success = true;

	HevcDeviceCommand hevcCommand;
	memset(&hevcCommand, 0, sizeof(HevcDeviceCommand));

	hevcCommand.command = Hevc_Command_MainState;
	hevcCommand.mainState = mainState;

	if (hevcCommand.mainState == Hevc_MainState_Encode)
	{
		hevcCommand.encodeMode = encodeMode;
		hevcCommand.firmwareType = firmwareType;
	}

	if (!mpDevice->HevcSendCommand(&hevcCommand))
	{
		success = false;
	}
	return success;
}

bool CNTV2m31::GetMainState(HevcMainState* pMainState,
							HevcEncodeMode* pEncodeMode,
							HevcFirmwareType* pFirmwareType)
{
	bool success;
	
	HevcDeviceStatus hevcStatus;
	memset(&hevcStatus, 0, sizeof(HevcDeviceStatus));
	
	if (mpDevice->HevcGetStatus(&hevcStatus))
	{
		if (pMainState != NULL) *pMainState = hevcStatus.mainState;
		if (pEncodeMode != NULL) *pEncodeMode = hevcStatus.encodeMode;
		if (pFirmwareType != NULL) *pFirmwareType = hevcStatus.firmwareType;
		success = true;
	}
	else
	{
		if (pMainState != NULL) *pMainState = Hevc_MainState_Unknown;
		if (pEncodeMode != NULL) *pEncodeMode = Hevc_EncodeMode_Unknown;
		if (pFirmwareType != NULL) *pFirmwareType = Hevc_FirmwareType_Unknown;
		success = false;
	}
	return success;
}


bool CNTV2m31::ChangeVInState(HevcVinState vinState, M31Channel channel)
{
	ULWord streamBit = 0;
	bool success = true;

	switch (channel)
	{
	case M31_CH0: { streamBit = 0x1; break; }
	case M31_CH1: { streamBit = 0x2; break; }
	case M31_CH2: { streamBit = 0x4; break; }
	case M31_CH3: { streamBit = 0x8; break; }
	default: return false;
	}

	HevcDeviceCommand hevcCommand;
	memset(&hevcCommand, 0, sizeof(HevcDeviceCommand));

	hevcCommand.command = Hevc_Command_VinState;
	hevcCommand.vinState = vinState;
	hevcCommand.streamBits = streamBit;

	if (!mpDevice->HevcSendCommand(&hevcCommand))
	{
		success = false;
	}
	return success;
}


bool CNTV2m31::ChangeVInState(HevcVinState vinState, uint32_t vinStreambits)
{
	bool success = true;

	HevcDeviceCommand hevcCommand;
	memset(&hevcCommand, 0, sizeof(HevcDeviceCommand));
	
	hevcCommand.command = Hevc_Command_VinState;
	hevcCommand.vinState = vinState;
	hevcCommand.streamBits = vinStreambits;
		
	if (!mpDevice->HevcSendCommand(&hevcCommand))
	{
		success = false;
	}
	return success;
}


bool CNTV2m31::GetVInState(HevcVinState* pVInState, M31Channel channel)
{
	bool success;
	
	HevcDeviceStatus hevcStatus;
	memset(&hevcStatus, 0, sizeof(HevcDeviceStatus));
	
	if (mpDevice->HevcGetStatus(&hevcStatus))
	{
		*pVInState = hevcStatus.vinState[channel];
		success = true;
	}
	else
	{
		*pVInState = Hevc_VinState_Unknown;
		success = false;
	}
	return success;
}


bool CNTV2m31::ChangeEHState(HevcEhState ehState, M31Channel channel)
{
	ULWord streamBit = 0;
	bool success = true;

	switch (channel)
	{
	case M31_CH0: { streamBit = 0x1; break; }
	case M31_CH1: { streamBit = 0x2; break; }
	case M31_CH2: { streamBit = 0x4; break; }
	case M31_CH3: { streamBit = 0x8; break; }
	default: return false;
	}

	HevcDeviceCommand hevcCommand;
	memset(&hevcCommand, 0, sizeof(HevcDeviceCommand));

	hevcCommand.command = Hevc_Command_EhState;
	hevcCommand.ehState = ehState;
	hevcCommand.streamBits = streamBit;

	if (!mpDevice->HevcSendCommand(&hevcCommand))
	{
		success = false;
	}
	return success;
}


bool CNTV2m31::ChangeEHState(HevcEhState ehState, uint32_t ehStreamBits)
{
	bool success = true;

	HevcDeviceCommand hevcCommand;
	memset(&hevcCommand, 0, sizeof(HevcDeviceCommand));
	
	hevcCommand.command = Hevc_Command_EhState;
	hevcCommand.ehState = ehState;
	hevcCommand.streamBits = ehStreamBits;
	
	if (!mpDevice->HevcSendCommand(&hevcCommand))
	{
		success = false;
	}
	return success;
}


bool CNTV2m31::GetEHState(HevcEhState* pEHState, M31Channel channel)
{
	bool success;
	
	HevcDeviceStatus hevcStatus;
	memset(&hevcStatus, 0, sizeof(HevcDeviceStatus));
	
	if (mpDevice->HevcGetStatus(&hevcStatus))
	{
		*pEHState = hevcStatus.ehState[channel];
		success = true;
	}
	else
	{
		*pEHState = Hevc_EhState_Unknown;
		success = false;
	}
	return success;
}


void CNTV2m31::GetDeviceInfo(HevcDeviceInfo* pDeviceInfo)
{
	mpDevice->HevcGetDeviceInfo(pDeviceInfo);
}


bool CNTV2m31::Reset()
{
	bool success = true;

	HevcDeviceCommand hevcCommand;
	memset(&hevcCommand, 0, sizeof(HevcDeviceCommand));
	
	hevcCommand.command = Hevc_Command_Reset;
	
	if (!mpDevice->HevcSendCommand(&hevcCommand))
	{
		success = false;
	}
	return success;
}


bool CNTV2m31::ChangeBitRateCbr(M31Channel channel, uint32_t cbrBitRate)
{
	bool success = true;
	
	HevcDeviceCommand hevcCommand;
	memset(&hevcCommand, 0, sizeof(HevcDeviceCommand));
	
	hevcCommand.command = Hevc_Command_ChangeParam;
	hevcCommand.paramStreamId = (ULWord)channel;
	hevcCommand.paramTarget = Hevc_ParamTarget_Cbr;
	hevcCommand.aveBitRate = cbrBitRate;
	
	if (!mpDevice->HevcSendCommand(&hevcCommand))
	{
		success = false;
	}
	return success;
}


bool CNTV2m31::ChangeBitRateVbr(M31Channel channel, uint32_t vbrMaxBitRate, uint32_t vbrAveBitRate, uint32_t ehprivate09, HevcChangeSequence changeSeq)
{
	bool success = true;
	
	HevcDeviceCommand hevcCommand;
	memset(&hevcCommand, 0, sizeof(HevcDeviceCommand));
	
	hevcCommand.command = Hevc_Command_ChangeParam;
	hevcCommand.paramStreamId = (ULWord)channel;
	hevcCommand.paramTarget = Hevc_ParamTarget_Vbr;
	hevcCommand.changeSequence = changeSeq;
	hevcCommand.maxBitRate = vbrMaxBitRate;
	hevcCommand.aveBitRate = vbrAveBitRate;
	hevcCommand.minBitRate = ehprivate09;
	
	if (!mpDevice->HevcSendCommand(&hevcCommand))
	{
		success = false;
	}
	return success;
}


bool CNTV2m31::RawTransfer(M31Channel channel, uint8_t* pRawBuffer, uint32_t rawDataSize, bool lastFrame)
{
	return RawTransfer(M31_NUMVIDEOPRESETS, channel, pRawBuffer, rawDataSize, NULL, 0, false, lastFrame);
}


bool CNTV2m31::RawTransfer(M31Channel channel,
						   uint8_t* pRawBuffer, uint32_t rawDataSize,
						   uint8_t* pPicBuffer, uint32_t picDataSize,
						   bool lastFrame)
{
	return RawTransfer(M31_NUMVIDEOPRESETS, channel, 
						pRawBuffer, rawDataSize, 
						pPicBuffer, picDataSize, 
						false, lastFrame);
}


bool CNTV2m31::RawTransfer(M31VideoPreset preset, M31Channel channel, 
							uint8_t* pRawBuffer, uint32_t rawDataSize, 
							bool field2, bool lastFrame)
{
	return RawTransfer(preset, channel, pRawBuffer, rawDataSize, NULL, 0, field2, lastFrame);
}


bool CNTV2m31::RawTransfer(M31VideoPreset preset, M31Channel channel,
							uint8_t* pRawBuffer, uint32_t rawDataSize,
							uint8_t* pPicBuffer, uint32_t picDataSize,
							bool field2, bool lastFrame)
{
	const M31PresetData* pData = GetM31PresetData(preset);
	uint32_t segVideoPitch = 0;
	uint32_t segCodecPitch = 0;
	uint32_t segSize = 0;
	uint32_t segCount = 0;

	if (pData != NULL)
	{
		uint32_t linePitch = GetPresetLinePitch(preset);
		uint32_t pitchCount = GetPresetPitchCount(preset);

		// verify buffer is large enough
		if (rawDataSize < (linePitch * pitchCount)) return false;

		// setup segmented transfer for interlaced frames
		if (pData->interlaced)
		{
			// offset 1 line for field 2
			if (field2)
			{
				pRawBuffer += linePitch;
			}

			// transfer every other video line
			segVideoPitch = linePitch * 2;
			segCodecPitch = linePitch;
			segSize = linePitch;
			segCount = pitchCount / 2;
		}
	}

	// check pts high value (>1 will fail on roll over)
	if ((pPicBuffer != NULL) && (picDataSize >= sizeof(HevcPictureData)))
	{
		HevcPictureData* pPicData = (HevcPictureData*)pPicBuffer;
		if ((pPicData->ptsValueHigh & 0xfffffffe) != 0)
			return false;
	}

	HevcDeviceTransfer hevcTransfer;
	memset(&hevcTransfer, 0, sizeof(HevcDeviceTransfer));

	hevcTransfer.streamType = Hevc_Stream_VideoRaw;
	hevcTransfer.streamId = (ULWord)channel;
	hevcTransfer.pVideoBuffer = pRawBuffer;
	hevcTransfer.videoBufferSize = rawDataSize;
	hevcTransfer.videoDataSize = rawDataSize;
	hevcTransfer.segVideoPitch = segVideoPitch;
	hevcTransfer.segCodecPitch = segCodecPitch;
	hevcTransfer.segSize = segSize;
	hevcTransfer.segCount = segCount;
	hevcTransfer.pInfoBuffer = pPicBuffer;
	hevcTransfer.infoBufferSize = picDataSize;
	hevcTransfer.infoDataSize = picDataSize;
	hevcTransfer.flags = lastFrame? HEVC_TRANSFER_FLAG_IS_LAST_FRAME : 0;

	bool result = mpDevice->HevcVideoTransfer(&hevcTransfer);

	return result;
}


bool CNTV2m31::EncTransfer(M31Channel channel,
						   uint8_t* pHevcBuffer, uint32_t hevcBufferSize,
						   uint32_t& hevcDataSize, bool& lastFrame)
{
	uint32_t esDataSize;

	return EncTransfer(channel, pHevcBuffer, hevcBufferSize, NULL, 0, hevcDataSize, esDataSize, lastFrame);
}


bool CNTV2m31::EncTransfer(M31Channel channel,
						   uint8_t* pHevcBuffer, uint32_t hevcBufferSize,
						   uint8_t* pEsBuffer, uint32_t esBufferSize,
						   uint32_t& hevcDataSize, uint32_t& esDataSize, bool& lastFrame)
{
	HevcDeviceTransfer hevcTransfer;
	memset(&hevcTransfer, 0, sizeof(HevcDeviceTransfer));

	hevcTransfer.streamType = Hevc_Stream_VideoEnc;
	hevcTransfer.streamId = (ULWord)channel;
	hevcTransfer.pVideoBuffer = pHevcBuffer;
	hevcTransfer.videoBufferSize = hevcBufferSize;
	hevcTransfer.pInfoBuffer = pEsBuffer;
	hevcTransfer.infoBufferSize = esBufferSize;

	bool result = mpDevice->HevcVideoTransfer(&hevcTransfer);

	if(result)
	{
		hevcDataSize = hevcTransfer.videoDataSize;
		esDataSize = hevcTransfer.infoDataSize;
		lastFrame = (hevcTransfer.flags & HEVC_TRANSFER_FLAG_IS_LAST_FRAME) != 0;
	}

	return result;
}


bool CNTV2m31::EncTransfer(M31Channel channel,
						   uint8_t* pHevcBuffer, uint32_t hevcBufferSize,
						   uint8_t* pEsBuffer, uint32_t esBufferSize,
						   uint32_t& hevcDataSize, uint32_t& esDataSize,
						   int64_t& encodeTime, bool& lastFrame)
{
	HevcDeviceTransfer hevcTransfer;
	memset(&hevcTransfer, 0, sizeof(HevcDeviceTransfer));

	hevcTransfer.streamType = Hevc_Stream_VideoEnc;
	hevcTransfer.streamId = (ULWord)channel;
	hevcTransfer.pVideoBuffer = pHevcBuffer;
	hevcTransfer.videoBufferSize = hevcBufferSize;
	hevcTransfer.pInfoBuffer = pEsBuffer;
	hevcTransfer.infoBufferSize = esBufferSize;

	bool result = mpDevice->HevcVideoTransfer(&hevcTransfer);

	if(result)
	{
		hevcDataSize = hevcTransfer.videoDataSize;
		esDataSize = hevcTransfer.infoDataSize;
		encodeTime = hevcTransfer.encodeTime;
		lastFrame = (hevcTransfer.flags & HEVC_TRANSFER_FLAG_IS_LAST_FRAME) != 0;
	}

	return result;
}


bool CNTV2m31::ConvertVideoFormatToPreset(NTV2VideoFormat videoFormat,
											NTV2FrameBufferFormat frameBufferFormat,
											bool vif,
											M31VideoPreset& preset)
{
	for (int i = 0; i < M31_NUMVIDEOPRESETS; i++)
	{
		if ((kM31PresetData[i].videoFormat == videoFormat) &&
			(kM31PresetData[i].frameBufferFormat == frameBufferFormat) &&
			(kM31PresetData[i].vif == vif))
		{
			preset = (M31VideoPreset)i;
			return true;
		}
	}

	return false;
}


NTV2VideoFormat CNTV2m31::GetPresetVideoFormat(M31VideoPreset preset)
{
	const M31PresetData* pData = GetM31PresetData(preset);

	if (pData == NULL) return NTV2_FORMAT_UNKNOWN;

	return pData->videoFormat;
}


NTV2FrameBufferFormat CNTV2m31::GetPresetFrameBufferFormat(M31VideoPreset preset)
{
	const M31PresetData* pData = GetM31PresetData(preset);

	if (pData == NULL) return NTV2_FBF_10BIT_YCBCR;

	return pData->frameBufferFormat;
}


bool CNTV2m31::IsPresetVIF(M31VideoPreset preset)
{
	const M31PresetData* pData = GetM31PresetData(preset);

	if (pData == NULL) return false;

	return pData->vif;
}


bool CNTV2m31::IsPresetUHD(M31VideoPreset preset)
{
	const M31PresetData* pData = GetM31PresetData(preset);

	if (pData == NULL) return false;

	return pData->uhd;
}


bool CNTV2m31::IsPresetInterlaced(M31VideoPreset preset)
{
	const M31PresetData* pData = GetM31PresetData(preset);

	if (pData == NULL) return false;

	return pData->interlaced;
}


uint32_t CNTV2m31::GetPresetPixelsPerLine(M31VideoPreset preset)
{
	const M31PresetData* pData = GetM31PresetData(preset);

	if (pData == NULL) return 0;

	return pData->pixelsPerLine;
}


uint32_t CNTV2m31::GetPresetLinesPerFrame(M31VideoPreset preset)
{
	const M31PresetData* pData = GetM31PresetData(preset);

	if (pData == NULL) return 0;

	return pData->linesPerFrame;
}


uint32_t CNTV2m31::GetPresetLinePitch(M31VideoPreset preset)
{
	const M31PresetData* pData = GetM31PresetData(preset);
	uint32_t linePitch = 0;

	if (pData == NULL) return 0;

	switch (pData->frameBufferFormat)
	{
	case NTV2_FBF_8BIT_YCBCR_420PL2:
		linePitch = pData->pixelsPerLine;
		break;
	case NTV2_FBF_8BIT_YCBCR_422PL2:
		linePitch = pData->pixelsPerLine * 10 / 8;
		break;
	case NTV2_FBF_8BIT_YCBCR:
		linePitch = pData->pixelsPerLine * 2;
		break;
	case NTV2_FBF_10BIT_YCBCR:
		linePitch = (pData->pixelsPerLine * 16 / 6);
		linePitch = ((linePitch - 1)/64 + 1) * 64;
		break;
	default:
		break;
	}

	return linePitch;
}


uint32_t CNTV2m31::GetPresetPitchCount(M31VideoPreset preset)
{
	const M31PresetData* pData = GetM31PresetData(preset);
	uint32_t pitchCount = 0;

	if (pData == NULL) return 0;

	switch (pData->frameBufferFormat)
	{
	case NTV2_FBF_8BIT_YCBCR_420PL2:
		pitchCount = pData->linesPerFrame * 3 / 2;
		break;
	case NTV2_FBF_10BIT_YCBCR_422PL2:
		pitchCount = pData->linesPerFrame * 2;
		break;
	case NTV2_FBF_8BIT_YCBCR:
	case NTV2_FBF_10BIT_YCBCR:
		pitchCount = pData->linesPerFrame;
		break;
	default:
		break;
	}

	return pitchCount;
}


uint32_t CNTV2m31::GetPresetFrameSize(M31VideoPreset preset)
{
	const M31PresetData* pData = GetM31PresetData(preset);

	if (pData == NULL) return 0;

	return GetPresetLinePitch(preset) * GetPresetPitchCount(preset);
}


