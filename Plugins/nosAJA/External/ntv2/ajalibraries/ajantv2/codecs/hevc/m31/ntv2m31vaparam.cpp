/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31vaparam.cpp
	@brief		Implementation of non-device-specific M31 class register getter/setter methods for "VA" params.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/

#include "ntv2m31vaparam.h"

// Constructor
CNTV2m31VAParam::CNTV2m31VAParam(CNTV2Card* device) :
mpDevice (device)
{
}

// Destructor
CNTV2m31VAParam::~CNTV2m31VAParam()
{
}

// Apparently we write to memory and not actual registers so best to zero out all values before we begin setting bit fields
bool CNTV2m31VAParam::Clear()
{
	for (uint32_t j = M31_VIRTUAL_CH0; j <= M31_VIRTUAL_CH31; j++)
	{
		for (uint32_t i = kRegVAParamCC; i <= kRegVAParamReserved2; i++)
		{
			WriteM31VAParam((M31VirtualChannel)j,(VAParamRegisterIndex)i, 0);
		}
	}
	return true;
}

// Writes a VAParamsChannel struct to the hardware for a given channel
bool CNTV2m31VAParam::SetVAParamsChannel(VAParamsChannel vAParamsChannel, M31VirtualChannel channel)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Channel specific values need to be set here before we write then out to the hardware
	vAParamsChannel.vASourceId = channel;

	SetCC(vAParamsChannel.vACC, channel);
	SetSource(vAParamsChannel.vASource, channel);
	SetSourceId(vAParamsChannel.vASourceId, channel);
	SetInterlace(vAParamsChannel.vAInterlace, channel);
	SetChromaFormat(vAParamsChannel.vAChromaFormat, channel);
	SetBitDepth(vAParamsChannel.vABitDepth, channel);
	
	SetFrameRate(vAParamsChannel.vAFrameRate, channel);
	SetChromaFormatOut(vAParamsChannel.vAChromaFormatOut, channel);
	SetVInPrivate01(vAParamsChannel.vABitDepthOut, channel);
	
	SetHSizeVA(vAParamsChannel.vAHSizeVA, channel);
	SetVSizeVA(vAParamsChannel.vAVSizeVA, channel);
	SetHSizeEH(vAParamsChannel.vAHSizeEH, channel);
	SetVSizeEH(vAParamsChannel.vAVSizeEH, channel);
	
	SetCoef(vAParamsChannel.vACoef0, 0, channel);
	SetCoef(vAParamsChannel.vACoef1, 1, channel);
	SetCoef(vAParamsChannel.vACoef2, 2, channel);
	SetCoef(vAParamsChannel.vACoef3, 3, channel);
	SetCoef(vAParamsChannel.vACoef4, 4, channel);
	SetCoef(vAParamsChannel.vACoef5, 5, channel);
	SetCoef(vAParamsChannel.vACoef6, 6, channel);
	SetCoef(vAParamsChannel.vACoef7, 7, channel);
	
	SetVAPrivate01(vAParamsChannel.vAPrivate01, channel);
	SetVAPrivate02(vAParamsChannel.vAPrivate02, channel);

	SetSceneChange(vAParamsChannel.vASceneChange, channel);

	return true;
}

// Reads a VAParamsChannel struct from the hardware for a given channel
bool CNTV2m31VAParam::GetVAParamsChannel(VAParamsChannel* vAParamsChannel, M31VirtualChannel channel)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	GetCC(&vAParamsChannel->vACC, channel);
	GetSource(&vAParamsChannel->vASource, channel);
	
	GetSourceId(&vAParamsChannel->vASourceId, channel);
	GetInterlace(&vAParamsChannel->vAInterlace, channel);
	GetChromaFormat(&vAParamsChannel->vAChromaFormat, channel);
	GetBitDepth(&vAParamsChannel->vABitDepth, channel);
	
	GetFrameRate(&vAParamsChannel->vAFrameRate, channel);
	GetChromaFormatOut(&vAParamsChannel->vAChromaFormatOut, channel);
	GetVInPrivate01(&vAParamsChannel->vABitDepthOut, channel);
	
	GetHSizeVA(&vAParamsChannel->vAHSizeVA, channel);
	GetVSizeVA(&vAParamsChannel->vAVSizeVA, channel);
	GetHSizeEH(&vAParamsChannel->vAHSizeEH, channel);
	GetVSizeEH(&vAParamsChannel->vAVSizeEH, channel);
	
	GetCoef(&vAParamsChannel->vACoef0, 0, channel);
	GetCoef(&vAParamsChannel->vACoef1, 1, channel);
	GetCoef(&vAParamsChannel->vACoef2, 2, channel);
	GetCoef(&vAParamsChannel->vACoef3, 3, channel);
	GetCoef(&vAParamsChannel->vACoef4, 4, channel);
	GetCoef(&vAParamsChannel->vACoef5, 5, channel);
	GetCoef(&vAParamsChannel->vACoef6, 6, channel);
	GetCoef(&vAParamsChannel->vACoef7, 7, channel);
	
	GetVAPrivate01(&vAParamsChannel->vAPrivate01, channel);
	GetVAPrivate02(&vAParamsChannel->vAPrivate02, channel);
	
	GetSceneChange(&vAParamsChannel->vASceneChange, channel);
	
	return true;
}

// Loads up a VAParamsChannel struct with all the default values for a given video preset
bool CNTV2m31VAParam::LoadVAParamsChannel(M31VideoPreset videoPreset, VAParamsChannel* vAParamsChannel)
{
	// Make sure we pass in a valid preset
	if (!IS_VALID_M31VideoPreset(videoPreset))
		return false;
	
	// Start with a clean slate
	memset(vAParamsChannel, 0, sizeof(VAParamsChannel));
	
	// Setup the common VA params
	vAParamsChannel->vASourceId					= M31_VIRTUAL_CH0;
	vAParamsChannel->vACoef1					= -19;
	vAParamsChannel->vACoef0					= -3;
	vAParamsChannel->vACoef3					= 500;
	vAParamsChannel->vACoef2					= 34;
	vAParamsChannel->vACoef5					= 34;
	vAParamsChannel->vACoef4					= 500;
	vAParamsChannel->vACoef7					= -3;
	vAParamsChannel->vACoef6					= -19;
	vAParamsChannel->vAPrivate01				= 0;
	vAParamsChannel->vAPrivate02				= 0;
	vAParamsChannel->vASceneChange				= 0;

	switch (videoPreset)
	{
		case M31_FILE_720X480_420_8_5994i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;

		case M31_FILE_720X480_420_8_60i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;

		case M31_FILE_720X480_420_8_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;

		case M31_FILE_720X480_420_8_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;

		case M31_FILE_720X480_422_10_5994i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;

		case M31_FILE_720X480_422_10_60i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;

		case M31_FILE_720X480_422_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;
		   
		case M31_FILE_720X480_422_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;
		   
		case M31_FILE_720X576_420_8_50i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 576;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 576;
			break;

		case M31_FILE_720X576_420_8_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 576;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 576;
			break;

		case M31_FILE_720X576_422_10_50i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 576;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 576;
			break;

		case M31_FILE_720X576_422_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 576;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 576;
			break;
		  
		case M31_FILE_1280X720_420_8_2398p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2398;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_420_8_24p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate24;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_420_8_25p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate25;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_420_8_2997p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2997;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_420_8_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_420_8_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_420_8_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;
			
		case M31_FILE_1280X720_420_8_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;
			
		case M31_FILE_1280X720_422_10_2398p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2398;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_422_10_24p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate24;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_422_10_25p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate25;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_422_10_2997p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2997;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_422_10_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_FILE_1280X720_422_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;
			
		case M31_FILE_1280X720_422_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;
			
		case M31_FILE_1280X720_422_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;
			
		case M31_FILE_1920X1080_420_8_50i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_1920X1080_420_8_2398p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2398;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_1920X1080_420_8_24p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate24;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_1920X1080_420_8_25p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate25;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_1920X1080_420_8_2997p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2997;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_1920X1080_420_8_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_1920X1080_420_8_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_1920X1080_420_8_5994i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_1920X1080_420_8_60i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_1920X1080_420_8_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_1920X1080_420_8_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_1920X1080_422_10_50i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_1920X1080_422_10_2398p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2398;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_1920X1080_422_10_24p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate24;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_1920X1080_422_10_25p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate25;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_1920X1080_422_10_2997p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2997;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_1920X1080_422_10_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_1920X1080_422_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_1920X1080_422_10_5994i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_1920X1080_422_10_60i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_1920X1080_422_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
		  
		case M31_FILE_1920X1080_422_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
		  
		case M31_FILE_2048X1080_420_8_2398p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2398;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_2048X1080_420_8_24p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate24;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_2048X1080_420_8_25p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate25;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_2048X1080_420_8_2997p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2997;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_2048X1080_420_8_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_2048X1080_420_8_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_2048X1080_420_8_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_2048X1080_420_8_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_2048X1080_422_10_2398p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2398;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_2048X1080_422_10_24p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate24;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_2048X1080_422_10_25p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate25;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_2048X1080_422_10_2997p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2997;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_2048X1080_422_10_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_FILE_2048X1080_422_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_FILE_2048X1080_422_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
		  
		case M31_FILE_2048X1080_422_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 2048;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 2048;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
		  
		case M31_FILE_3840X2160_420_8_2398p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2398;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_420_8_24p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate24;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_420_8_25p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate25;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_420_8_2997p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2997;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_420_8_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_420_8_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_420_8_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_420_8_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_420_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_420_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_420_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_2398p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2398;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_8_24p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate24;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_8_25p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate25;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_2997p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2997;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_8_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_3840X2160_422_10_2398p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2398;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_10_24p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate24;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_10_25p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate25;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_10_2997p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate2997;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_10_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_4096X2160_420_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 4096;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 4096;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_4096X2160_420_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma420;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 4096;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 4096;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_4096X2160_422_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 4096;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 4096;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_FILE_4096X2160_422_10_5994p_IF:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 4096;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 4096;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_FILE_4096X2160_422_10_60p_IF:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 4096;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 4096;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_VIF_720X480_420_8_5994i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;

		case M31_VIF_720X480_420_8_60i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;

		case M31_VIF_720X480_420_8_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;

		case M31_VIF_720X480_420_8_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;

		case M31_VIF_720X480_422_10_5994i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;
			
		case M31_VIF_720X480_422_10_60i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;
			
		case M31_VIF_720X480_422_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;
		   
		case M31_VIF_720X480_422_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 480;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 480;
			break;
		   
		case M31_VIF_720X576_420_8_50i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 576;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 576;
			break;

		case M31_VIF_720X576_420_8_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 576;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 576;
			break;

		case M31_VIF_720X576_422_10_50i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 576;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 576;
			break;
			
		case M31_VIF_720X576_422_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 720;
			vAParamsChannel->vAVSizeVA					= 576;
			vAParamsChannel->vAHSizeEH					= 720;
			vAParamsChannel->vAVSizeEH					= 576;
			break;
			
		case M31_VIF_1280X720_420_8_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;
			
		case M31_VIF_1280X720_420_8_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;
			
		case M31_VIF_1280X720_420_8_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;
			
		case M31_VIF_1280X720_422_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;

		case M31_VIF_1280X720_422_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;
		  
		case M31_VIF_1280X720_422_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1280;
			vAParamsChannel->vAVSizeVA					= 720;
			vAParamsChannel->vAHSizeEH					= 1280;
			vAParamsChannel->vAVSizeEH					= 720;
			break;
		  
		case M31_VIF_1920X1080_420_8_50i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_VIF_1920X1080_420_8_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_VIF_1920X1080_420_8_5994i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_VIF_1920X1080_420_8_60i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_VIF_1920X1080_420_8_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_VIF_1920X1080_420_8_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_VIF_1920X1080_420_10_50i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_VIF_1920X1080_420_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_VIF_1920X1080_420_10_5994i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_VIF_1920X1080_420_10_60i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_VIF_1920X1080_420_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_VIF_1920X1080_420_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;

		case M31_VIF_1920X1080_422_10_5994i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
		   
		case M31_VIF_1920X1080_422_10_60i:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Interlace;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
		   
		case M31_VIF_1920X1080_422_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_VIF_1920X1080_422_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 1920;
			vAParamsChannel->vAVSizeVA					= 1080;
			vAParamsChannel->vAHSizeEH					= 1920;
			vAParamsChannel->vAVSizeEH					= 1088;
			break;
			
		case M31_VIF_3840X2160_420_8_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_VIF_3840X2160_420_8_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
		   
		case M31_VIF_3840X2160_420_8_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_VIF_3840X2160_420_8_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth8;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth8;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_VIF_3840X2160_420_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_VIF_3840X2160_420_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			vAParamsChannel->vACoef1					= -93;
			vAParamsChannel->vACoef0					= -7;
			vAParamsChannel->vACoef3					= 698;
			vAParamsChannel->vACoef2					= 263;
			vAParamsChannel->vACoef5					= -93;
			vAParamsChannel->vACoef4					= 263;
			vAParamsChannel->vACoef7					= 0;
			vAParamsChannel->vACoef6					= -7;
			break;
			
		case M31_VIF_3840X2160_420_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma420;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			vAParamsChannel->vACoef1					= -93;
			vAParamsChannel->vACoef0					= -7;
			vAParamsChannel->vACoef3					= 698;
			vAParamsChannel->vACoef2					= 263;
			vAParamsChannel->vACoef5					= -93;
			vAParamsChannel->vACoef4					= 263;
			vAParamsChannel->vACoef7					= 0;
			vAParamsChannel->vACoef6					= -7;
			break;
			
		case M31_VIF_3840X2160_422_10_30p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate30;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_VIF_3840X2160_422_10_50p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate50;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;

		case M31_VIF_3840X2160_422_10_5994p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate5994;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_VIF_3840X2160_422_10_60p:
			vAParamsChannel->vASource					= M31_SourceVA;
			vAParamsChannel->vAInterlace				= M31_Progressive;
			vAParamsChannel->vAChromaFormat				= M31_Chroma422;
			vAParamsChannel->vABitDepth					= M31_BitDepth10;
			vAParamsChannel->vAFrameRate				= M31_FrameRate60;
			vAParamsChannel->vAChromaFormatOut			= M31_Chroma422;
			vAParamsChannel->vABitDepthOut				= M31_BitDepth10;
			vAParamsChannel->vAHSizeVA					= 3840;
			vAParamsChannel->vAVSizeVA					= 2160;
			vAParamsChannel->vAHSizeEH					= 3840;
			vAParamsChannel->vAVSizeEH					= 2160;
			break;
			
		case M31_NUMVIDEOPRESETS:
			break;
	}
	return true;
}

// Public setters/getters

bool CNTV2m31VAParam::SetCC(M31CC cc, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamCC, cc,
						   kRegMaskVAParamCC, kRegShiftVAParamCC);
}

bool CNTV2m31VAParam::GetCC(M31CC* cc, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamCC, reinterpret_cast <uint32_t*> (cc),
						  kRegMaskVAParamCC, kRegShiftVAParamCC);
}

bool CNTV2m31VAParam::SetSource(M31SourceVA source, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamSource, source,
						   kRegMaskVAParamSource, kRegShiftVAParamSource);
}

bool CNTV2m31VAParam::GetSource(M31SourceVA* source, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamSource, reinterpret_cast <uint32_t*> (source),
						  kRegMaskVAParamSource, kRegShiftVAParamSource);
}

bool CNTV2m31VAParam::SetSourceId(M31VirtualChannel sourceId, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamSource, sourceId,
						   kRegMaskVAParamSourceId, kRegShiftVAParamSourceId);
}

bool CNTV2m31VAParam::GetSourceId(M31VirtualChannel* sourceId, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamSource, reinterpret_cast <uint32_t*> (sourceId),
						  kRegMaskVAParamSourceId, kRegShiftVAParamSourceId);
}

bool CNTV2m31VAParam::SetInterlace(M31ScanMode interlace, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamSource, interlace,
						   kRegMaskVAInterlace, kRegShiftVAInterlace);
}

bool CNTV2m31VAParam::GetInterlace(M31ScanMode* interlace, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamSource, reinterpret_cast <uint32_t*> (interlace),
						  kRegMaskVAInterlace, kRegShiftVAInterlace);
}

bool CNTV2m31VAParam::SetChromaFormat(M31ChromaFormat chromaFormat, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamSource, chromaFormat,
						   kRegMaskVAParamChromaFormat, kRegShiftVAParamChromaFormat);
}

bool CNTV2m31VAParam::GetChromaFormat(M31ChromaFormat* chromaFormat, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamSource, reinterpret_cast <uint32_t*> (chromaFormat),
						  kRegMaskVAParamChromaFormat, kRegShiftVAParamChromaFormat);
}

bool CNTV2m31VAParam::SetBitDepth(M31BitDepth bitDepth, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamSource, bitDepth,
						   kRegMaskVAParamBitDepth, kRegShiftVAParamBitDepth);
}

bool CNTV2m31VAParam::GetBitDepth(M31BitDepth* bitDepth, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamSource, reinterpret_cast <uint32_t*> (bitDepth),
						  kRegMaskVAParamBitDepth, kRegShiftVAParamBitDepth);
}

bool CNTV2m31VAParam::SetFrameRate(M31FrameRate frameRate, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamRateFormat, frameRate,
						   kRegMaskVAParamFrameRate, kRegShiftVAParamFrameRate);
}

bool CNTV2m31VAParam::GetFrameRate(M31FrameRate* frameRate, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamRateFormat, reinterpret_cast <uint32_t*> (frameRate),
						  kRegMaskVAParamFrameRate, kRegShiftVAParamFrameRate);
}

bool CNTV2m31VAParam::SetChromaFormatOut(M31ChromaFormat chromaFormatOut, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamRateFormat, chromaFormatOut,
						   kRegMaskVAParamChromaFormatOut, kRegShiftVAParamChromaFormatOut);
}

bool CNTV2m31VAParam::GetChromaFormatOut(M31ChromaFormat* chromaFormatOut, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamRateFormat, reinterpret_cast <uint32_t*> (chromaFormatOut),
						  kRegMaskVAParamChromaFormatOut, kRegShiftVAParamChromaFormatOut);
}

bool CNTV2m31VAParam::SetVInPrivate01(M31BitDepth vinprivate01, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamRateFormat, vinprivate01,
						   kRegMaskVAParamBitDepthOut, kRegShiftVAParamBitDepthOut);
}

bool CNTV2m31VAParam::GetVInPrivate01(M31BitDepth* vinprivate01, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamRateFormat, reinterpret_cast <uint32_t*> (vinprivate01),
						  kRegMaskVAParamBitDepthOut, kRegShiftVAParamBitDepthOut);
}

bool CNTV2m31VAParam::SetHSizeVA(uint32_t hSizeVA, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamSizeVA, hSizeVA,
						   kRegMaskVAParamHSizeVA, kRegShiftVAParamHSizeVA);
}

bool CNTV2m31VAParam::GetHSizeVA(uint32_t* hSizeVA, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamSizeVA, reinterpret_cast <uint32_t*> (hSizeVA),
						  kRegMaskVAParamHSizeVA, kRegShiftVAParamHSizeVA);
}

bool CNTV2m31VAParam::SetVSizeVA(uint32_t vSizeVA, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamSizeVA, vSizeVA,
						   kRegMaskVAParamVSizeVA, kRegShiftVAParamVSizeVA);
}

bool CNTV2m31VAParam::GetVSizeVA(uint32_t* vSizeVA, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamSizeVA, reinterpret_cast <uint32_t*> (vSizeVA),
						  kRegMaskVAParamVSizeVA, kRegShiftVAParamVSizeVA);
}

bool CNTV2m31VAParam::SetHSizeEH(uint32_t hSizeEH, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamSizeEH, hSizeEH,
						   kRegMaskVAParamHSizeEH, kRegShiftVAParamHSizeEH);
}

bool CNTV2m31VAParam::GetHSizeEH(uint32_t* hSizeEH, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamSizeEH, reinterpret_cast <uint32_t*> (hSizeEH),
						  kRegMaskVAParamHSizeEH, kRegShiftVAParamHSizeEH);
}

bool CNTV2m31VAParam::SetVSizeEH(uint32_t vSizeEH, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamSizeEH, vSizeEH,
						   kRegMaskVAParamVSizeEH, kRegShiftVAParamVSizeEH);
}

bool CNTV2m31VAParam::GetVSizeEH(uint32_t* vSizeEH, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamSizeEH, reinterpret_cast <uint32_t*> (vSizeEH),
						  kRegMaskVAParamVSizeEH, kRegShiftVAParamVSizeEH);
}

bool CNTV2m31VAParam::SetSceneChange(uint32_t sceneChange, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamSceneChange, sceneChange,
						   kRegMaskVAParamSceneChange, kRegShiftVAParamSceneChange);
}

bool CNTV2m31VAParam::GetSceneChange(uint32_t* sceneChange, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamSceneChange, reinterpret_cast <uint32_t*> (sceneChange),
						  kRegMaskVAParamSceneChange, kRegShiftVAParamSceneChange);
}

// Protected setters/getters

bool CNTV2m31VAParam::SetVAPrivate01(uint32_t vaprivate01, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamStartOffset, vaprivate01,
						   kRegMaskVAParamPrivate01, kRegShiftVAParamPrivate01);
}

bool CNTV2m31VAParam::GetVAPrivate01(uint32_t* vaprivate01, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamStartOffset, reinterpret_cast <uint32_t*> (vaprivate01),
						  kRegMaskVAParamPrivate01, kRegShiftVAParamPrivate01);
}

bool CNTV2m31VAParam::SetVAPrivate02(uint32_t vaprivate02, M31VirtualChannel channel)
{
	return WriteM31VAParam(channel, kRegVAParamStartOffset, vaprivate02,
						   kRegMaskVAParamPrivate02, kRegShiftVAParamPrivate02);
}

bool CNTV2m31VAParam::GetVAPrivate02(uint32_t* vaprivate02, M31VirtualChannel channel)
{
	return ReadM31VAParam(channel, kRegVAParamStartOffset, reinterpret_cast <uint32_t*> (vaprivate02),
						  kRegMaskVAParamPrivate02, kRegShiftVAParamPrivate02);
}

// Utility methods

bool CNTV2m31VAParam::SetCoef(uint32_t coef, uint32_t coefNum, M31VirtualChannel channel)
{
	VAParamRegisterIndex	reg;
	VAParamRegisterMask		mask;
	VAParamRegisterShift	shift;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Make sure we pass in a valid coefficient number
	if (coefNum > 7)
		return false;
	
	switch (coefNum)
	{
		default:
		case 0:
			reg = kRegVAParamCoef1_0;
			mask = kRegMaskVAParamCoef0;
			shift = kRegShiftVAParamCoef0;
			break;
		case 1:
			reg = kRegVAParamCoef1_0;
			mask = kRegMaskVAParamCoef1;
			shift = kRegShiftVAParamCoef1;
			break;
		case 2:
			reg = kRegVAParamCoef3_2;
			mask = kRegMaskVAParamCoef2;
			shift = kRegShiftVAParamCoef2;
			break;
		case 3:
			reg = kRegVAParamCoef3_2;
			mask = kRegMaskVAParamCoef3;
			shift = kRegShiftVAParamCoef3;
			break;
		case 4:
			reg = kRegVAParamCoef5_4;
			mask = kRegMaskVAParamCoef4;
			shift = kRegShiftVAParamCoef4;
			break;
		case 5:
			reg = kRegVAParamCoef5_4;
			mask = kRegMaskVAParamCoef5;
			shift = kRegShiftVAParamCoef5;
			break;
		case 6:
			reg = kRegVAParamCoef7_6;
			mask = kRegMaskVAParamCoef6;
			shift = kRegShiftVAParamCoef6;
			break;
		case 7:
			reg = kRegVAParamCoef7_6;
			mask = kRegMaskVAParamCoef7;
			shift = kRegShiftVAParamCoef7;
			break;
			
	};
	
	return WriteM31VAParam(channel, reg, coef, mask, shift);
}

bool CNTV2m31VAParam::GetCoef(uint32_t* coef, uint32_t coefNum, M31VirtualChannel channel)
{
	VAParamRegisterIndex	reg;
	VAParamRegisterMask		mask;
	VAParamRegisterShift	shift;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Make sure we pass in a valid coefficient number
	if (coefNum > 7)
		return false;
	
	switch (coefNum)
	{
		default:
		case 0:
			reg = kRegVAParamCoef1_0;
			mask = kRegMaskVAParamCoef0;
			shift = kRegShiftVAParamCoef0;
			break;
		case 1:
			reg = kRegVAParamCoef1_0;
			mask = kRegMaskVAParamCoef1;
			shift = kRegShiftVAParamCoef1;
			break;
		case 2:
			reg = kRegVAParamCoef3_2;
			mask = kRegMaskVAParamCoef2;
			shift = kRegShiftVAParamCoef2;
			break;
		case 3:
			reg = kRegVAParamCoef3_2;
			mask = kRegMaskVAParamCoef3;
			shift = kRegShiftVAParamCoef3;
			break;
		case 4:
			reg = kRegVAParamCoef5_4;
			mask = kRegMaskVAParamCoef4;
			shift = kRegShiftVAParamCoef4;
			break;
		case 5:
			reg = kRegVAParamCoef5_4;
			mask = kRegMaskVAParamCoef5;
			shift = kRegShiftVAParamCoef5;
			break;
		case 6:
			reg = kRegVAParamCoef7_6;
			mask = kRegMaskVAParamCoef6;
			shift = kRegShiftVAParamCoef6;
			break;
		case 7:
			reg = kRegVAParamCoef7_6;
			mask = kRegMaskVAParamCoef7;
			shift = kRegShiftVAParamCoef7;
			break;
			
	};
	
	return ReadM31VAParam(channel, reg, reinterpret_cast <uint32_t*> (coef), mask, shift);
}

// These routines will eventually calculate the correct register offset basesd on register enum and channel and using the VA start and channel size
// which will eventually call a driver Read/Write function that will write into the params bar
bool CNTV2m31VAParam::WriteM31VAParam(M31VirtualChannel channel, VAParamRegisterIndex registerIndex, uint32_t registerValue, VAParamRegisterMask registerMask, VAParamRegisterShift registerShift)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Now figure out where this register is based on channel and index
	uint32_t regNum = ((channel * VAPARAM_CH_SIZE) + VAPARAM_REG_START) + (registerIndex * 4);
	
	return mpDevice->HevcWriteRegister(regNum, registerValue, registerMask, registerShift);
}

bool CNTV2m31VAParam::WriteM31VAParam(M31VirtualChannel channel, VAParamRegisterIndex registerIndex, uint32_t registerValue)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Now figure out where this register is based on channel and index
	uint32_t regNum = ((channel * VAPARAM_CH_SIZE) + VAPARAM_REG_START) + (registerIndex * 4);
	
	return mpDevice->HevcWriteRegister(regNum, registerValue);
}

bool CNTV2m31VAParam::ReadM31VAParam(M31VirtualChannel channel, VAParamRegisterIndex registerIndex, uint32_t* registerValue, VAParamRegisterMask registerMask, VAParamRegisterShift registerShift)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Now figure out where this register is based on channel and index
	uint32_t regNum = ((channel * VAPARAM_CH_SIZE) + VAPARAM_REG_START) + (registerIndex * 4);

	return mpDevice->HevcReadRegister(regNum, registerValue, registerMask, registerShift);
}

bool CNTV2m31VAParam::ReadM31VAParam(M31VirtualChannel channel, VAParamRegisterIndex registerIndex, uint32_t* registerValue)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Now figure out where this register is based on channel and index
	uint32_t regNum = ((channel * VAPARAM_CH_SIZE) + VAPARAM_REG_START) + (registerIndex * 4);
	
	return mpDevice->HevcReadRegister(regNum, registerValue);
}



