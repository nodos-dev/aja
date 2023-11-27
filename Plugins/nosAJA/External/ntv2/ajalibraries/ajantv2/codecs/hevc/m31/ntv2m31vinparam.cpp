/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31vinparam.cpp
	@brief		Implementation of non-device-specific M31 class register getter/setter methods for "VIn" params.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/

#include "ntv2m31vinparam.h"

// Constructor
CNTV2m31VInParam::CNTV2m31VInParam(CNTV2Card* device) :
mpDevice (device)
{
}

// Destructor
CNTV2m31VInParam::~CNTV2m31VInParam()
{
}

// Apparently we write to memory and not actual registers so best to zero out all values before we begin setting bit fields
bool CNTV2m31VInParam::Clear()
{
	for (uint32_t j = M31_VIRTUAL_CH0; j <= M31_VIRTUAL_CH31; j++)
	{
		for (uint32_t i = kRegVINParamCC; i <= kRegVINParamReserved1; i++)
		{
			WriteM31VInParam((M31VirtualChannel)j,(VINParamRegisterIndex)i, 0);
		}
	}
	return true;
}

// Writes a VInParamsChannel struct to the hardware for a given channel
bool CNTV2m31VInParam::SetVInParamsChannel(VInParamsChannel vInParamsChannel, M31VirtualChannel channel)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Channel specific values need to be set here before we write then out to the hardware
	vInParamsChannel.vInSourceID = channel;
	
	SetCC(vInParamsChannel.vInCC, channel);
	SetSource(vInParamsChannel.vInSource, channel);
	SetSourceId(vInParamsChannel.vInSourceID, channel);
	SetChromaFormat(vInParamsChannel.vInChromaFormat, channel);
	SetBitDepth(vInParamsChannel.vInBitDepth, channel);
	SetVInFrameRate(vInParamsChannel.vInFrameRate, channel);
	SetVInPrivate01(vInParamsChannel.vInPrivate01, channel);
	SetHSize(vInParamsChannel.vInHSize, channel);
	SetVSize(vInParamsChannel.vInVSize, channel);
	SetPTSMode(vInParamsChannel.vInPTSMode, channel);
	SetInitialPTSMSB(vInParamsChannel.vInInitialPTSMSB, channel);
	SetInitialPTS(vInParamsChannel.vInInitialPTS, channel);
	SetInitialSerialNum(vInParamsChannel.vInInitialSerialNum, channel);
	
	return true;
}

// Reads a VInParamsChannel struct from the hardware for a given channel
bool CNTV2m31VInParam::GetVInParamsChannel(VInParamsChannel* vInParamsChannel, M31VirtualChannel channel)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	GetCC(&vInParamsChannel->vInCC, channel);
	GetSource(&vInParamsChannel->vInSource, channel);
	GetSourceId(&vInParamsChannel->vInSourceID, channel);
	GetChromaFormat(&vInParamsChannel->vInChromaFormat, channel);
	GetBitDepth(&vInParamsChannel->vInBitDepth, channel);
	GetVInFrameRate(&vInParamsChannel->vInFrameRate, channel);
	GetVInPrivate01(&vInParamsChannel->vInPrivate01, channel);
	GetHSize(&vInParamsChannel->vInHSize, channel);
	GetVSize(&vInParamsChannel->vInVSize, channel);
	GetPTSMode(&vInParamsChannel->vInPTSMode, channel);
	GetInitialPTSMSB(&vInParamsChannel->vInInitialPTSMSB, channel);
	GetInitialPTS(&vInParamsChannel->vInInitialPTS, channel);
	GetInitialSerialNum(&vInParamsChannel->vInInitialSerialNum, channel);
	
	return true;
}

// Loads up a VInParamsChannel struct with all the default values for a given video preset
bool CNTV2m31VInParam::LoadVInParamsChannel(M31VideoPreset videoPreset, VInParamsChannel* vInParamsChannel)
{
	// Make sure we pass in a valid preset
	if (!IS_VALID_M31VideoPreset(videoPreset))
		return false;
	
	// Start with a clean slate
	memset(vInParamsChannel, 0, sizeof(VInParamsChannel));
	
	// Setup the common VIn params
	vInParamsChannel->vInPrivate01				= 0;

	vInParamsChannel->vInPTSMode				= M31_PTSModeAuto;
	vInParamsChannel->vInInitialPTSMSB			= 0;
	vInParamsChannel->vInInitialPTS				= 0;
	vInParamsChannel->vInInitialSerialNum		= 0;
	vInParamsChannel->vInSourceID				= M31_VIRTUAL_CH0;

	switch (videoPreset)
	{
		case M31_FILE_720X480_420_8_5994i:
		case M31_FILE_720X480_420_8_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 480;
			break;
		   
		case M31_FILE_720X480_420_8_60i:
		case M31_FILE_720X480_420_8_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 480;
			break;

		case M31_FILE_720X480_422_10_5994i:
		case M31_FILE_720X480_422_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 480;
			break;
			
		case M31_FILE_720X480_422_10_60i:
		case M31_FILE_720X480_422_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 480;
			break;
		   
		case M31_FILE_720X576_420_8_50i:
		case M31_FILE_720X576_420_8_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 576;
			break;

		case M31_FILE_720X576_422_10_50i:
		case M31_FILE_720X576_422_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 576;
			break;
			
		case M31_FILE_1280X720_420_8_2398p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2398;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_FILE_1280X720_420_8_24p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate24;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;
			
		case M31_FILE_1280X720_420_8_25p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate25;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_FILE_1280X720_420_8_2997p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2997;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_FILE_1280X720_420_8_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_FILE_1280X720_420_8_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_FILE_1280X720_420_8_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_FILE_1280X720_420_8_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_FILE_1280X720_422_10_2398p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2398;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;
			
		case M31_FILE_1280X720_422_10_24p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate24;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;
			
		case M31_FILE_1280X720_422_10_25p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate25;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;
			
		case M31_FILE_1280X720_422_10_2997p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2997;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;
			
		case M31_FILE_1280X720_422_10_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;
			
		case M31_FILE_1280X720_422_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_FILE_1280X720_422_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_FILE_1280X720_422_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_FILE_1920X1080_420_8_2398p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2398;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_420_8_24p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate24;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_420_8_25p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate25;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_420_8_2997p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2997;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_420_8_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_420_8_50i:
		case M31_FILE_1920X1080_420_8_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_420_8_5994i:
		case M31_FILE_1920X1080_420_8_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_420_8_60i:
		case M31_FILE_1920X1080_420_8_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;
			
		case M31_FILE_1920X1080_422_10_2398p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2398;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_422_10_24p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate24;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_422_10_25p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate25;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_422_10_2997p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2997;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_422_10_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_422_10_50i:
		case M31_FILE_1920X1080_422_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_422_10_5994i:
		case M31_FILE_1920X1080_422_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_1920X1080_422_10_60i:
		case M31_FILE_1920X1080_422_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;
			
		case M31_FILE_2048X1080_420_8_2398p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2398;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_420_8_24p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate24;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_420_8_25p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate25;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_420_8_2997p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2997;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_420_8_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_420_8_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_420_8_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_420_8_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;
			
		case M31_FILE_2048X1080_422_10_2398p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2398;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_422_10_24p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate24;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_422_10_25p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate25;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_422_10_2997p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2997;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_422_10_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_422_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_422_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_FILE_2048X1080_422_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;	 // 2048 no multichannel
			vInParamsChannel->vInVSize					= 1080;
			break;
			
		case M31_FILE_3840X2160_420_8_2398p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2398;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_420_8_24p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate24;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_FILE_3840X2160_420_8_25p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate25;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_FILE_3840X2160_420_8_2997p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2997;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_FILE_3840X2160_420_8_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_FILE_3840X2160_420_8_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_420_8_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_420_8_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
	
		case M31_FILE_3840X2160_420_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
		   vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_420_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_420_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_8_2398p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2398;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_24p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate24;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_25p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate25;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_2997p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2997;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_8_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_10_2398p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2398;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_10_24p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate24;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_10_25p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate25;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_10_2997p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate2997;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_10_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_3840X2160_422_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_FILE_3840X2160_422_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_FILE_4096X2160_420_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
		   vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 4096;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_4096X2160_420_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 4096;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_FILE_4096X2160_422_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 4096;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_4096X2160_422_10_5994p_IF:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 4096;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_FILE_4096X2160_422_10_60p_IF:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVEI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 4096;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_VIF_720X480_420_8_5994i:
		case M31_VIF_720X480_420_8_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 480;
			break;

		case M31_VIF_720X480_420_8_60i:
		case M31_VIF_720X480_420_8_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 480;
			break;

		case M31_VIF_720X480_422_10_5994i:
		case M31_VIF_720X480_422_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 480;
			break;

		case M31_VIF_720X480_422_10_60i:
		case M31_VIF_720X480_422_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 480;
			break;
		 
		case M31_VIF_720X576_420_8_50i:
		case M31_VIF_720X576_420_8_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 576;
			break;

		case M31_VIF_720X576_422_10_50i:
		case M31_VIF_720X576_422_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 720;
			vInParamsChannel->vInVSize					= 576;
			break;
		  
		case M31_VIF_1280X720_420_8_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_VIF_1280X720_420_8_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_VIF_1280X720_420_8_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_VIF_1280X720_422_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_VIF_1280X720_422_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;

		case M31_VIF_1280X720_422_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1280;
			vInParamsChannel->vInVSize					= 720;
			break;
		   
		case M31_VIF_1920X1080_420_8_50i:
		case M31_VIF_1920X1080_420_8_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_VIF_1920X1080_420_8_5994i:
		case M31_VIF_1920X1080_420_8_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_VIF_1920X1080_420_8_60i:
		case M31_VIF_1920X1080_420_8_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_VIF_1920X1080_420_10_50i:
		case M31_VIF_1920X1080_420_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_VIF_1920X1080_420_10_5994i:
		case M31_VIF_1920X1080_420_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_VIF_1920X1080_420_10_60i:
		case M31_VIF_1920X1080_420_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;
 
		case M31_VIF_1920X1080_422_10_5994i:
		case M31_VIF_1920X1080_422_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_VIF_1920X1080_422_10_60i:
		case M31_VIF_1920X1080_422_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 1920;
			vInParamsChannel->vInVSize					= 1080;
			break;

		case M31_VIF_3840X2160_420_8_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_VIF_3840X2160_420_8_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_VIF_3840X2160_420_8_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
		
		case M31_VIF_3840X2160_420_8_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth8;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
		
		case M31_VIF_3840X2160_420_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_VIF_3840X2160_420_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;

		case M31_VIF_3840X2160_420_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma420;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_VIF_3840X2160_422_10_30p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate30;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_VIF_3840X2160_422_10_50p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate50;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_VIF_3840X2160_422_10_5994p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate5994;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_VIF_3840X2160_422_10_60p:
			vInParamsChannel->vInFrameRate				= M31_FrameRate60;
			vInParamsChannel->vInSource					= M31_SourceVI;
			vInParamsChannel->vInChromaFormat			= M31_Chroma422;
			vInParamsChannel->vInBitDepth				= M31_BitDepth10;
			vInParamsChannel->vInHSize					= 3840;
			vInParamsChannel->vInVSize					= 2160;
			break;
			
		case M31_NUMVIDEOPRESETS:
			break;
	}
	return true;
}

// Public setters/getters

bool CNTV2m31VInParam::SetCC(M31CC cc, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamCC, cc,
							kRegMaskVINParamCC, kRegShiftVINParamCC);
}

bool CNTV2m31VInParam::GetCC(M31CC* cc, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamCC, reinterpret_cast <uint32_t*> (cc),
						   kRegMaskVINParamCC, kRegShiftVINParamCC);
}

bool CNTV2m31VInParam::SetSource(M31SourceVI source, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamSource, source,
							kRegMaskVINParamSource, kRegShiftVINParamSource);
}

bool CNTV2m31VInParam::GetSource(M31SourceVI* source, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamSource, reinterpret_cast <uint32_t*> (source),
						   kRegMaskVINParamSource, kRegShiftVINParamSource);
}

bool CNTV2m31VInParam::SetSourceId(M31VirtualChannel sourceId, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamSource, sourceId,
							kRegMaskVINParamSourceId, kRegShiftVINParamSourceId);
}

bool CNTV2m31VInParam::GetSourceId(M31VirtualChannel* sourceId, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamSource, reinterpret_cast <uint32_t*> (sourceId),
						   kRegMaskVINParamSourceId, kRegShiftVINParamSourceId);
}

bool CNTV2m31VInParam::SetChromaFormat(M31ChromaFormat chromaFormat, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamSource, chromaFormat,
							kRegMaskVINParamChromaFormat, kRegShiftVINParamChromaFormat);
}

bool CNTV2m31VInParam::GetChromaFormat(M31ChromaFormat* chromaFormat, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamSource, reinterpret_cast <uint32_t*> (chromaFormat),
						   kRegMaskVINParamChromaFormat, kRegShiftVINParamChromaFormat);
}

bool CNTV2m31VInParam::SetBitDepth(M31BitDepth bitDepth, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamSource, bitDepth,
							kRegMaskVINParamBitDepth, kRegShiftVINParamBitDepth);
}

bool CNTV2m31VInParam::GetBitDepth(M31BitDepth* bitDepth, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamSource, reinterpret_cast <uint32_t*> (bitDepth),
						   kRegMaskVINParamBitDepth, kRegShiftVINParamBitDepth);
}

bool CNTV2m31VInParam::SetHSize(uint32_t hSize, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamSize, hSize,
							kRegMaskVINParamHSize, kRegShiftVINParamHSize);
}

bool CNTV2m31VInParam::GetHSize(uint32_t* hSize, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamSize, reinterpret_cast <uint32_t*> (hSize),
						   kRegMaskVINParamHSize, kRegShiftVINParamHSize);
}

bool CNTV2m31VInParam::SetVSize(uint32_t vSize, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamSize, vSize,
							kRegMaskVINParamVSize, kRegShiftVINParamVSize);
}

bool CNTV2m31VInParam::GetVSize(uint32_t* vSize, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamSize, reinterpret_cast <uint32_t*> (vSize),
						   kRegMaskVINParamVSize, kRegShiftVINParamVSize);
}

bool CNTV2m31VInParam::SetPTSMode(M31PTSMode ptsMode, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamPTSModeInitialMSB, ptsMode,
							kRegMaskVINParamPTSMode, kRegShiftVINParamPTSMode);
}

bool CNTV2m31VInParam::GetPTSMode(M31PTSMode* ptsMode, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamPTSModeInitialMSB, reinterpret_cast <uint32_t*> (ptsMode),
						   kRegMaskVINParamPTSMode, kRegShiftVINParamPTSMode);
}

bool CNTV2m31VInParam::SetInitialPTSMSB(uint32_t initialPTSMSB, M31VirtualChannel channel)
{
	
	return WriteM31VInParam(channel, kRegVINParamPTSModeInitialMSB, initialPTSMSB,
							kRegMaskVINParamInitialPTSMSB, kRegShiftVINParamInitialPTSMSB);
}

bool CNTV2m31VInParam::GetInitialPTSMSB(uint32_t* initialPTSMSB, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamPTSModeInitialMSB, reinterpret_cast <uint32_t*> (initialPTSMSB),
						   kRegMaskVINParamInitialPTSMSB, kRegShiftVINParamInitialPTSMSB);
}

bool CNTV2m31VInParam::SetInitialPTS(uint32_t initialPTS, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamInitialPTS, initialPTS,
							kRegMaskVINParamInitialPTS, kRegShiftVINParamInitialPTS);
}

bool CNTV2m31VInParam::GetInitialPTS(uint32_t* initialPTS, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamInitialPTS, reinterpret_cast <uint32_t*> (initialPTS),
						   kRegMaskVINParamInitialPTS, kRegShiftVINParamInitialPTS);
}

bool CNTV2m31VInParam::SetInitialSerialNum(uint32_t initialSerialNum, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamInitialSerialNum, initialSerialNum,
							kRegMaskVINParamInitialSerialNum, kRegShiftVINParamInitialSerialNum);
}

bool CNTV2m31VInParam::GetInitialSerialNum(uint32_t* initialSerialNum, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamInitialSerialNum, reinterpret_cast <uint32_t*> (initialSerialNum),
						   kRegMaskVINParamInitialSerialNum, kRegShiftVINParamInitialSerialNum);
}

// Protected setters/getters

bool CNTV2m31VInParam::SetVInFrameRate(M31FrameRate frameRate, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamOut, frameRate,
							kRegMaskVInParamFrameRate, kRegShiftVInParamFrameRate);
}

bool CNTV2m31VInParam::GetVInFrameRate(M31FrameRate* frameRate, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamOut, reinterpret_cast <uint32_t*> (frameRate),
						   kRegMaskVInParamFrameRate, kRegShiftVInParamFrameRate);
}

bool CNTV2m31VInParam::SetVInPrivate01(uint32_t vinprivate01, M31VirtualChannel channel)
{
	return WriteM31VInParam(channel, kRegVINParamOut, vinprivate01,
							kRegMaskVInParamPrivate01, kRegShiftVInParamPrivate01);
}

bool CNTV2m31VInParam::GetVInPrivate01(uint32_t* vinprivate01, M31VirtualChannel channel)
{
	return ReadM31VInParam(channel, kRegVINParamOut, reinterpret_cast <uint32_t*> (vinprivate01),
						   kRegMaskVInParamPrivate01, kRegShiftVInParamPrivate01);
}

// Utility methods

// These routines will eventually calculate the correct register offset basesd on register enum and channel and using the VIn start and channel size
// which will eventually call a driver Read/Write function that will write into the params bar
bool CNTV2m31VInParam::WriteM31VInParam(M31VirtualChannel channel, VINParamRegisterIndex registerIndex, uint32_t registerValue, VINParamRegisterMask registerMask, VINParamRegisterShift registerShift)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Now figure out where this register is based on channel and index
	uint32_t regNum = ((channel * VINPARAM_CH_SIZE) + VINPARAM_REG_START) + (registerIndex * 4);

	return mpDevice->HevcWriteRegister(regNum, registerValue, registerMask, registerShift);
}

bool CNTV2m31VInParam::WriteM31VInParam(M31VirtualChannel channel, VINParamRegisterIndex registerIndex, uint32_t registerValue)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Calculate where the register number is based on the register index and start of CPARAM register block
	uint32_t regNum = ((channel * VINPARAM_CH_SIZE) + VINPARAM_REG_START) + (registerIndex * 4);
	
	return mpDevice->HevcWriteRegister(regNum, registerValue);
}

bool CNTV2m31VInParam::ReadM31VInParam(M31VirtualChannel channel, VINParamRegisterIndex registerIndex, uint32_t* registerValue, VINParamRegisterMask registerMask, VINParamRegisterShift registerShift)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;

	// Now figure out where this register is based on channel and index
	uint32_t regNum = ((channel * VINPARAM_CH_SIZE) + VINPARAM_REG_START) + (registerIndex * 4);

	return mpDevice->HevcReadRegister(regNum, registerValue, registerMask, registerShift);
}

bool CNTV2m31VInParam::ReadM31VInParam(M31VirtualChannel channel, VINParamRegisterIndex registerIndex, uint32_t* registerValue)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Calculate where the register number is based on the register index and start of CPARAM register block
	uint32_t regNum = ((channel * VINPARAM_CH_SIZE) + VINPARAM_REG_START) + (registerIndex * 4);
	
	return mpDevice->HevcReadRegister(regNum, registerValue);
}



