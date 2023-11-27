/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31viparam.cpp
	@brief		Implementation of non-device-specific M31 class register getter/setter methods for "VI" params.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/

#include "ntv2m31viparam.h"

// Constructor
CNTV2m31VIParam::CNTV2m31VIParam(CNTV2Card* device) :
mpDevice (device)
{
}


// Destructor
CNTV2m31VIParam::~CNTV2m31VIParam()
{
}

// Apparently we write to memory and not actual registers so best to zero out all values before we begin setting bit fields
bool CNTV2m31VIParam::Clear()
{
	for (uint32_t i = kRegVI0ParamCC; i <= kRegVIParamReserved3; i++)
	{
		WriteM31VIParam((VIParamRegisterIndex)i, 0);
	}
	return true;
}

// Writes a CParamsVIChannel struct to the hardware for a given channel
bool CNTV2m31VIParam::SetVIParamsChannel(VIParamsChannel vIParamsChannel, M31Channel channel)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	SetYCMux(vIParamsChannel.vIYCMux, channel);
	SetYCSwap(vIParamsChannel.vIYCSwap, channel);
	SetVIPrivate01(vIParamsChannel.vIPrivate01, channel);
	SetVIPrivate02(vIParamsChannel.vIPrivate02, channel);
	SetVIPrivate03(vIParamsChannel.vIPrivate03, channel);
	SetVIPrivate04(vIParamsChannel.vIPrivate04, channel);
	SetFormat(vIParamsChannel.vIFormat, channel);
	
	SetVIPrivate05(vIParamsChannel.vIPrivate05, channel);
	SetVIPrivate06(vIParamsChannel.vIPrivate06, channel);
	SetVIPrivate07(vIParamsChannel.vIPrivate07, channel);
	SetVIPrivate08(vIParamsChannel.vIPrivate08, channel);
	SetVIPrivate09(vIParamsChannel.vIPrivate09, channel);
	SetVIPrivate10(vIParamsChannel.vIPrivate10, channel);
	SetVIPrivate11(vIParamsChannel.vIPrivate11, channel);
	SetVIPrivate12(vIParamsChannel.vIPrivate12, channel);
	SetVIPrivate13(vIParamsChannel.vIPrivate13, channel);
	SetGPIOEncStart(vIParamsChannel.vIGPIOEncStart, channel);
	SetSyncMembers(vIParamsChannel.vISyncMembers, channel);
	SetSyncMaster(vIParamsChannel.vISyncMaster, channel);
	SetInputPort(vIParamsChannel.vIInputPort, channel);
	
	// I think this kicks the registers so they are read by the firmware, so we write this last
	SetCC(M31_READCC, channel);
	
	return true;
}

// Reads a CParamsVIChannel struct from the hardware for a given channel
bool CNTV2m31VIParam::GetVIParamsChannel(VIParamsChannel* vIParamsChannel, M31Channel channel)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	GetYCMux(&vIParamsChannel->vIYCMux, channel);
	GetYCSwap(&vIParamsChannel->vIYCSwap, channel);
	GetVIPrivate01(&vIParamsChannel->vIPrivate01, channel);
	GetVIPrivate02(&vIParamsChannel->vIPrivate02, channel);
	GetVIPrivate03(&vIParamsChannel->vIPrivate03, channel);
	GetVIPrivate04(&vIParamsChannel->vIPrivate04, channel);
	GetFormat(&vIParamsChannel->vIFormat, channel);
	
	GetVIPrivate05(&vIParamsChannel->vIPrivate05, channel);
	GetVIPrivate06(&vIParamsChannel->vIPrivate06, channel);
	GetVIPrivate07(&vIParamsChannel->vIPrivate07, channel);
	GetVIPrivate08(&vIParamsChannel->vIPrivate08, channel);
	GetVIPrivate09(&vIParamsChannel->vIPrivate09, channel);
	GetVIPrivate10(&vIParamsChannel->vIPrivate10, channel);
	GetVIPrivate11(&vIParamsChannel->vIPrivate11, channel);
	GetVIPrivate12(&vIParamsChannel->vIPrivate12, channel);
	GetVIPrivate13(&vIParamsChannel->vIPrivate13, channel);
	GetGPIOEncStart(&vIParamsChannel->vIGPIOEncStart, channel);
	GetSyncMembers(&vIParamsChannel->vISyncMembers, channel);
	GetSyncMaster(&vIParamsChannel->vISyncMaster, channel);
	GetInputPort(&vIParamsChannel->vIInputPort, channel);
	
	return true;
}

// Loads up a VIParamsChannel struct with all the default values for a given video preset
bool CNTV2m31VIParam::LoadVIParamsChannel(M31VideoPreset videoPreset, VIParamsChannel* vIParamsChannel)
{
	// Make sure we pass in a valid preset
	if (!IS_VALID_M31VideoPreset(videoPreset))
		return false;
	
	// Start with a clean slate
	memset(vIParamsChannel, 0, sizeof(VIParamsChannel));
	
	// Common to all
	vIParamsChannel->vIYCMux				= M31_YCSeparate;
	vIParamsChannel->vIYCSwap				= 1;
	vIParamsChannel->vIPrivate01			= 0;
	vIParamsChannel->vIPrivate02			= 0;
	vIParamsChannel->vIPrivate03			= 0;
	vIParamsChannel->vIPrivate04			= 0;
	vIParamsChannel->vIPrivate05			= 0;
	vIParamsChannel->vIPrivate06			= 0;
	vIParamsChannel->vIPrivate07			= 0;
	vIParamsChannel->vIPrivate08			= 0;
	vIParamsChannel->vIPrivate09			= 0;
	vIParamsChannel->vIPrivate10			= 0;
	vIParamsChannel->vIPrivate11			= 0;
	vIParamsChannel->vIPrivate12			= 0;
	vIParamsChannel->vIPrivate13			= 0;
	vIParamsChannel->vIInputPort			= M31_DefaultPort;
	vIParamsChannel->vISyncMaster			= M31_Slave;
	vIParamsChannel->vISyncMembers			= 0;
	vIParamsChannel->vIGPIOEncStart			= 1;

	switch (videoPreset)
	{
		case M31_FILE_720X480_420_8_5994i:
		case M31_FILE_720X480_420_8_60i:
		case M31_FILE_720X480_422_10_5994i:
		case M31_FILE_720X480_422_10_60i:
		case M31_VIF_720X480_420_8_5994i:
		case M31_VIF_720X480_420_8_60i:
		case M31_VIF_720X480_422_10_5994i:
		case M31_VIF_720X480_422_10_60i:
			vIParamsChannel->vIFormat				= M31_720X480i;
			break;

		case M31_FILE_720X480_420_8_5994p:
		case M31_FILE_720X480_420_8_60p:
		case M31_FILE_720X480_422_10_5994p:
		case M31_FILE_720X480_422_10_60p:
		case M31_VIF_720X480_420_8_5994p:
		case M31_VIF_720X480_420_8_60p:
		case M31_VIF_720X480_422_10_5994p:
		case M31_VIF_720X480_422_10_60p:
			vIParamsChannel->vIFormat				= M31_720X480p;
			break;
			
		case M31_FILE_720X576_420_8_50i:
		case M31_FILE_720X576_422_10_50i:
		case M31_VIF_720X576_420_8_50i:
		case M31_VIF_720X576_422_10_50i:
			vIParamsChannel->vIFormat				= M31_720X576i;
			break;
			
		case M31_FILE_720X576_420_8_50p:
		case M31_FILE_720X576_422_10_50p:
		case M31_VIF_720X576_420_8_50p:
		case M31_VIF_720X576_422_10_50p:
			vIParamsChannel->vIFormat				= M31_720X576p;
			break;
			
		case M31_FILE_1280X720_420_8_50p:
		case M31_FILE_1280X720_420_8_5994p:
		case M31_FILE_1280X720_420_8_60p:
		case M31_FILE_1280X720_422_10_50p:
		case M31_FILE_1280X720_422_10_5994p:
		case M31_FILE_1280X720_422_10_60p:
		case M31_VIF_1280X720_420_8_50p:
		case M31_VIF_1280X720_420_8_5994p:
		case M31_VIF_1280X720_420_8_60p:
		case M31_VIF_1280X720_422_10_50p:
		case M31_VIF_1280X720_422_10_5994p:
		case M31_VIF_1280X720_422_10_60p:
		case M31_FILE_1280X720_420_8_2398p:
		case M31_FILE_1280X720_420_8_24p:
		case M31_FILE_1280X720_420_8_25p:
		case M31_FILE_1280X720_420_8_2997p:
		case M31_FILE_1280X720_420_8_30p:
		case M31_FILE_1280X720_422_10_2398p:
		case M31_FILE_1280X720_422_10_24p:
		case M31_FILE_1280X720_422_10_25p:
		case M31_FILE_1280X720_422_10_2997p:
		case M31_FILE_1280X720_422_10_30p:
			vIParamsChannel->vIFormat				= M31_1280X720p;
			break;

		case M31_FILE_1920X1080_420_8_50i:
		case M31_FILE_1920X1080_420_8_5994i:
		case M31_FILE_1920X1080_420_8_60i:
		case M31_FILE_1920X1080_422_10_50i:
		case M31_FILE_1920X1080_422_10_5994i:
		case M31_FILE_1920X1080_422_10_60i:
		case M31_VIF_1920X1080_420_8_50i:
		case M31_VIF_1920X1080_420_8_5994i:
		case M31_VIF_1920X1080_420_8_60i:
		case M31_VIF_1920X1080_420_10_50i:
		case M31_VIF_1920X1080_420_10_5994i:
		case M31_VIF_1920X1080_420_10_60i:
		case M31_VIF_1920X1080_422_10_5994i:
		case M31_VIF_1920X1080_422_10_60i:
			vIParamsChannel->vIFormat				= M31_1920X1080i;
			break;

		case M31_FILE_1920X1080_420_8_50p:
		case M31_FILE_1920X1080_420_8_5994p:
		case M31_FILE_1920X1080_420_8_60p:
		case M31_FILE_1920X1080_422_10_50p:
		case M31_FILE_1920X1080_422_10_5994p:
		case M31_FILE_1920X1080_422_10_60p:
		case M31_VIF_1920X1080_420_8_50p:
		case M31_VIF_1920X1080_420_8_5994p:
		case M31_VIF_1920X1080_420_8_60p:
		case M31_VIF_1920X1080_420_10_50p:
		case M31_VIF_1920X1080_420_10_5994p:
		case M31_VIF_1920X1080_420_10_60p:
		case M31_VIF_1920X1080_422_10_5994p:
		case M31_VIF_1920X1080_422_10_60p:
		case M31_FILE_1920X1080_420_8_2398p:
		case M31_FILE_1920X1080_420_8_24p:
		case M31_FILE_1920X1080_420_8_25p:
		case M31_FILE_1920X1080_420_8_2997p:
		case M31_FILE_1920X1080_420_8_30p:
		case M31_FILE_1920X1080_422_10_2398p:
		case M31_FILE_1920X1080_422_10_24p:
		case M31_FILE_1920X1080_422_10_25p:
		case M31_FILE_1920X1080_422_10_2997p:
		case M31_FILE_1920X1080_422_10_30p:
			vIParamsChannel->vIFormat				= M31_1920X1080p;
			break;
			
		case M31_FILE_3840X2160_420_8_50p:
		case M31_FILE_3840X2160_420_8_5994p:
		case M31_FILE_3840X2160_420_8_60p:
		case M31_FILE_3840X2160_420_10_50p:
		case M31_FILE_3840X2160_420_10_5994p:
		case M31_FILE_3840X2160_420_10_60p:
		case M31_FILE_3840X2160_422_8_50p:
		case M31_FILE_3840X2160_422_8_5994p:
		case M31_FILE_3840X2160_422_8_60p:
		case M31_FILE_3840X2160_422_10_50p:
		case M31_FILE_3840X2160_422_10_5994p:
		case M31_FILE_3840X2160_422_10_60p:
		case M31_VIF_3840X2160_420_8_30p:
		case M31_VIF_3840X2160_420_8_50p:
		case M31_VIF_3840X2160_420_8_5994p:
		case M31_VIF_3840X2160_420_8_60p:
		case M31_VIF_3840X2160_420_10_50p:
		case M31_VIF_3840X2160_420_10_5994p:
		case M31_VIF_3840X2160_420_10_60p:
		case M31_VIF_3840X2160_422_10_30p:
		case M31_VIF_3840X2160_422_10_50p:
		case M31_VIF_3840X2160_422_10_5994p:
		case M31_VIF_3840X2160_422_10_60p:
		case M31_FILE_3840X2160_420_8_2398p:
		case M31_FILE_3840X2160_420_8_24p:
		case M31_FILE_3840X2160_420_8_25p:
		case M31_FILE_3840X2160_420_8_2997p:
		case M31_FILE_3840X2160_420_8_30p:
		case M31_FILE_3840X2160_422_8_2398p:
		case M31_FILE_3840X2160_422_8_24p:
		case M31_FILE_3840X2160_422_8_25p:
		case M31_FILE_3840X2160_422_8_2997p:
		case M31_FILE_3840X2160_422_8_30p:
		case M31_FILE_3840X2160_422_10_2398p:
		case M31_FILE_3840X2160_422_10_24p:
		case M31_FILE_3840X2160_422_10_25p:
		case M31_FILE_3840X2160_422_10_2997p:
		case M31_FILE_3840X2160_422_10_30p:
			vIParamsChannel->vIFormat				= M31_3840X2160_SMPTE435M;
			break;
			
		case M31_FILE_4096X2160_420_10_5994p:
		case M31_FILE_4096X2160_420_10_60p:
		case M31_FILE_4096X2160_422_10_50p:
		case M31_FILE_4096X2160_422_10_5994p_IF:
		case M31_FILE_4096X2160_422_10_60p_IF:
			vIParamsChannel->vIFormat				= M31_ARBITRARY_RESOLUTION;
			break;


		case M31_FILE_2048X1080_420_8_2398p:
		case M31_FILE_2048X1080_420_8_24p:
		case M31_FILE_2048X1080_420_8_25p:
		case M31_FILE_2048X1080_420_8_2997p:
		case M31_FILE_2048X1080_420_8_30p:
		case M31_FILE_2048X1080_420_8_50p:
		case M31_FILE_2048X1080_420_8_5994p:
		case M31_FILE_2048X1080_420_8_60p:
		case M31_FILE_2048X1080_422_10_2398p:
		case M31_FILE_2048X1080_422_10_24p:
		case M31_FILE_2048X1080_422_10_25p:
		case M31_FILE_2048X1080_422_10_2997p:
		case M31_FILE_2048X1080_422_10_30p:
		case M31_FILE_2048X1080_422_10_50p:
		case M31_FILE_2048X1080_422_10_5994p:
		case M31_FILE_2048X1080_422_10_60p:
			vIParamsChannel->vIFormat				= M31_ARBITRARY_RESOLUTION;
			break;

		case M31_NUMVIDEOPRESETS:
			break;
	}
	
	return true;
}

bool CNTV2m31VIParam::SetCC(M31CC cc, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamCC; break;
		case M31_CH1: reg = kRegVI1ParamCC; break;
		case M31_CH2: reg = kRegVI2ParamCC; break;
		case M31_CH3: reg = kRegVI3ParamCC; break;
	};
	
	return WriteM31VIParam(reg, cc, kRegMaskVIParamCC, kRegShiftVIParamCC);
}

bool CNTV2m31VIParam::GetCC(M31CC* cc, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamCC; break;
		case M31_CH1: reg = kRegVI1ParamCC; break;
		case M31_CH2: reg = kRegVI2ParamCC; break;
		case M31_CH3: reg = kRegVI3ParamCC; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (cc), kRegMaskVIParamCC, kRegShiftVIParamCC);
}

bool CNTV2m31VIParam::SetYCMux(M31YCMode ycMux, M31Channel channel)
{
	VIParamRegisterIndex reg;
		
	if (GetVIRegister(channel, &reg))
	{
		return WriteM31VIParam(reg, ycMux, kRegMaskVIParamYCMux, kRegShiftVIParamYCMux);
	}
	else
		return false;
}

bool CNTV2m31VIParam::GetYCMux(M31YCMode* ycMux, M31Channel channel)
{
	VIParamRegisterIndex reg;

	if (GetVIRegister(channel, &reg))
	{
		return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (ycMux), kRegMaskVIParamYCMux, kRegShiftVIParamYCMux);;
	}
	else
		return false;
}

bool CNTV2m31VIParam::SetYCSwap(uint32_t ycSwap, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return WriteM31VIParam(reg, ycSwap, kRegMaskVIParamYCSwap, kRegShiftVIParamYCSwap);
	}
	else
		return false;
}

bool CNTV2m31VIParam::GetYCSwap(uint32_t* ycSwap, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (ycSwap), kRegMaskVIParamYCSwap, kRegShiftVIParamYCSwap);;
	}
	else
		return false;
}

bool CNTV2m31VIParam::SetFormat(M31VideoFormat format, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return WriteM31VIParam(reg, format, kRegMaskVIParamFormat, kRegShiftVIParamFormat);
	}
	else
	return false;
}

bool CNTV2m31VIParam::GetFormat(M31VideoFormat* format, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (format), kRegMaskVIParamFormat, kRegShiftVIParamFormat);;
	}
	else
	return false;
}

bool CNTV2m31VIParam::SetGPIOEncStart(uint32_t gpioEncStart, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
	return false;
	
	switch (channel)
	{
			case M31_CH0: reg = kRegVI0ParamMisc; break;
			case M31_CH1: reg = kRegVI1ParamMisc; break;
			case M31_CH2: reg = kRegVI2ParamMisc; break;
			case M31_CH3: reg = kRegVI3ParamMisc; break;
	};
	
	return WriteM31VIParam(reg, gpioEncStart, kRegMaskVIParamGPIO, kRegShiftVIParamGPIO);
}

bool CNTV2m31VIParam::GetGPIOEncStart(uint32_t* gpioEncStart, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
	return false;
	
	switch (channel)
	{
			case M31_CH0: reg = kRegVI0ParamMisc; break;
			case M31_CH1: reg = kRegVI1ParamMisc; break;
			case M31_CH2: reg = kRegVI2ParamMisc; break;
			case M31_CH3: reg = kRegVI3ParamMisc; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (gpioEncStart), kRegMaskVIParamGPIO, kRegShiftVIParamGPIO);
}

bool CNTV2m31VIParam::SetSyncMembers(uint32_t syncMembers, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
	return false;
	
	switch (channel)
	{
			case M31_CH0: reg = kRegVI0ParamMisc; break;
			case M31_CH1: reg = kRegVI1ParamMisc; break;
			case M31_CH2: reg = kRegVI2ParamMisc; break;
			case M31_CH3: reg = kRegVI3ParamMisc; break;
	};
	
	return WriteM31VIParam(reg, syncMembers, kRegMaskVIParamSyncMembers, kRegShiftVIParamSyncMembers);
}

bool CNTV2m31VIParam::GetSyncMembers(uint32_t* syncMembers, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
	return false;
	
	switch (channel)
	{
			case M31_CH0: reg = kRegVI0ParamMisc; break;
			case M31_CH1: reg = kRegVI1ParamMisc; break;
			case M31_CH2: reg = kRegVI2ParamMisc; break;
			case M31_CH3: reg = kRegVI3ParamMisc; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (syncMembers), kRegMaskVIParamSyncMembers, kRegShiftVIParamSyncMembers);
}

bool CNTV2m31VIParam::SetSyncMaster(M31SyncMasterMode syncMaster, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
	return false;
	
	switch (channel)
	{
			case M31_CH0: reg = kRegVI0ParamMisc; break;
			case M31_CH1: reg = kRegVI1ParamMisc; break;
			case M31_CH2: reg = kRegVI2ParamMisc; break;
			case M31_CH3: reg = kRegVI3ParamMisc; break;
	};
	
	return WriteM31VIParam(reg, syncMaster, kRegMaskVIParamSyncMaster, kRegShiftVIParamSyncMaster);
}

bool CNTV2m31VIParam::GetSyncMaster(M31SyncMasterMode* syncMaster, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
	return false;
	
	switch (channel)
	{
			case M31_CH0: reg = kRegVI0ParamMisc; break;
			case M31_CH1: reg = kRegVI1ParamMisc; break;
			case M31_CH2: reg = kRegVI2ParamMisc; break;
			case M31_CH3: reg = kRegVI3ParamMisc; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (syncMaster), kRegMaskVIParamSyncMaster, kRegShiftVIParamSyncMaster);
}

bool CNTV2m31VIParam::SetInputPort(M31InputPort inputPort, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
	return false;
	
	switch (channel)
	{
			case M31_CH0: reg = kRegVI0ParamMisc; break;
			case M31_CH1: reg = kRegVI1ParamMisc; break;
			case M31_CH2: reg = kRegVI2ParamMisc; break;
			case M31_CH3: reg = kRegVI3ParamMisc; break;
	};
	
	return WriteM31VIParam(reg, inputPort, kRegMaskVIParamInputPort, kRegShiftVIParamInputPort);
}

bool CNTV2m31VIParam::GetInputPort(M31InputPort* inputPort, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
	return false;
	
	switch (channel)
	{
			case M31_CH0: reg = kRegVI0ParamMisc; break;
			case M31_CH1: reg = kRegVI1ParamMisc; break;
			case M31_CH2: reg = kRegVI2ParamMisc; break;
			case M31_CH3: reg = kRegVI3ParamMisc; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (inputPort), kRegMaskVIParamInputPort, kRegShiftVIParamInputPort);
}

// Protected setters/getters

bool CNTV2m31VIParam::SetVIPrivate01(uint32_t viprivate01, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return WriteM31VIParam(reg, viprivate01, kRegMaskVIParamPrivate01, kRegShiftVIParamPrivate01);
	}
	else
		return false;
}

bool CNTV2m31VIParam::GetVIPrivate01(uint32_t* viprivate01, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate01), kRegMaskVIParamPrivate01, kRegShiftVIParamPrivate01);;
	}
	else
		return false;
}

bool CNTV2m31VIParam::SetVIPrivate02(uint32_t viprivate02, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return WriteM31VIParam(reg, viprivate02, kRegMaskVIParamPrivate02, kRegShiftVIParamPrivate02);
	}
	else
		return false;
}

bool CNTV2m31VIParam::GetVIPrivate02(uint32_t* viprivate02, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate02), kRegMaskVIParamPrivate02, kRegShiftVIParamPrivate02);;
	}
	else
		return false;
}

bool CNTV2m31VIParam::SetVIPrivate03(uint32_t viprivate03, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return WriteM31VIParam(reg, viprivate03, kRegMaskVIParamPrivate03, kRegShiftVIParamPrivate03);
	}
	else
		return false;
}

bool CNTV2m31VIParam::GetVIPrivate03(uint32_t* viprivate03, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate03), kRegMaskVIParamPrivate03, kRegShiftVIParamPrivate03);;
	}
	else
		return false;
}

bool CNTV2m31VIParam::SetVIPrivate04(uint32_t viprivate04, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return WriteM31VIParam(reg, viprivate04, kRegMaskVIParamPrivate04, kRegShiftVIParamPrivate04);
	}
	else
		return false;
}

bool CNTV2m31VIParam::GetVIPrivate04(uint32_t* viprivate04, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	if (GetVIRegister(channel, &reg))
	{
		return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate04), kRegMaskVIParamPrivate04, kRegShiftVIParamPrivate04);;
	}
	else
		return false;
}

bool CNTV2m31VIParam::SetVIPrivate05(uint32_t viprivate05, M31Channel channel)
{
	VIParamRegisterIndex reg;

	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
	return false;

	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamStartLine; break;
		case M31_CH1: reg = kRegVI1ParamStartLine; break;
		case M31_CH2: reg = kRegVI2ParamStartLine; break;
		case M31_CH3: reg = kRegVI3ParamStartLine; break;
	};
	
	return WriteM31VIParam(reg, viprivate05, kRegMaskVIParamPrivate05, kRegShiftVIParamPrivate05);
}

bool CNTV2m31VIParam::GetVIPrivate05(uint32_t* viprivate05, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamStartLine; break;
		case M31_CH1: reg = kRegVI1ParamStartLine; break;
		case M31_CH2: reg = kRegVI2ParamStartLine; break;
		case M31_CH3: reg = kRegVI3ParamStartLine; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate05), kRegMaskVIParamPrivate05, kRegShiftVIParamPrivate05);
}

bool CNTV2m31VIParam::SetVIPrivate06(uint32_t viprivate06, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamStartLine; break;
		case M31_CH1: reg = kRegVI1ParamStartLine; break;
		case M31_CH2: reg = kRegVI2ParamStartLine; break;
		case M31_CH3: reg = kRegVI3ParamStartLine; break;
	};
	
	return WriteM31VIParam(reg, viprivate06, kRegMaskVIParamPrivate06, kRegShiftVIParamPrivate06);
}

bool CNTV2m31VIParam::GetVIPrivate06(uint32_t* viprivate06, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamStartLine; break;
		case M31_CH1: reg = kRegVI1ParamStartLine; break;
		case M31_CH2: reg = kRegVI2ParamStartLine; break;
		case M31_CH3: reg = kRegVI3ParamStartLine; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate06), kRegMaskVIParamPrivate06, kRegShiftVIParamPrivate06);
}

bool CNTV2m31VIParam::SetVIPrivate07(uint32_t viprivate07, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamMaxCount; break;
		case M31_CH1: reg = kRegVI1ParamMaxCount; break;
		case M31_CH2: reg = kRegVI2ParamMaxCount; break;
		case M31_CH3: reg = kRegVI3ParamMaxCount; break;
	};
	
	return WriteM31VIParam(reg, viprivate07, kRegMaskVIParamPrivate07, kRegShiftVIParamPrivate07);
}

bool CNTV2m31VIParam::GetVIPrivate07(uint32_t* viprivate07, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamMaxCount; break;
		case M31_CH1: reg = kRegVI1ParamMaxCount; break;
		case M31_CH2: reg = kRegVI2ParamMaxCount; break;
		case M31_CH3: reg = kRegVI3ParamMaxCount; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate07), kRegMaskVIParamPrivate07, kRegShiftVIParamPrivate07);
}

bool CNTV2m31VIParam::SetVIPrivate08(uint32_t viprivate08, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamMaxCount; break;
		case M31_CH1: reg = kRegVI1ParamMaxCount; break;
		case M31_CH2: reg = kRegVI2ParamMaxCount; break;
		case M31_CH3: reg = kRegVI3ParamMaxCount; break;
	};
	
	return WriteM31VIParam(reg, viprivate08, kRegMaskVIParamPrivate08, kRegShiftVIParamPrivate08);
}

bool CNTV2m31VIParam::GetVIPrivate08(uint32_t* viprivate08, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamMaxCount; break;
		case M31_CH1: reg = kRegVI1ParamMaxCount; break;
		case M31_CH2: reg = kRegVI2ParamMaxCount; break;
		case M31_CH3: reg = kRegVI3ParamMaxCount; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate08), kRegMaskVIParamPrivate08, kRegShiftVIParamPrivate08);
}

bool CNTV2m31VIParam::SetVIPrivate09(uint32_t viprivate09, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamValidPosLine; break;
		case M31_CH1: reg = kRegVI1ParamValidPosLine; break;
		case M31_CH2: reg = kRegVI2ParamValidPosLine; break;
		case M31_CH3: reg = kRegVI3ParamValidPosLine; break;
	};
	
	return WriteM31VIParam(reg, viprivate09, kRegMaskVIParamPrivate09, kRegShiftVIParamPrivate09);
}

bool CNTV2m31VIParam::GetVIPrivate09(uint32_t* viprivate09, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamValidPosLine; break;
		case M31_CH1: reg = kRegVI1ParamValidPosLine; break;
		case M31_CH2: reg = kRegVI2ParamValidPosLine; break;
		case M31_CH3: reg = kRegVI3ParamValidPosLine; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate09), kRegMaskVIParamPrivate09, kRegShiftVIParamPrivate09);
}

bool CNTV2m31VIParam::SetVIPrivate10(uint32_t viprivate10, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamValidPosLine; break;
		case M31_CH1: reg = kRegVI1ParamValidPosLine; break;
		case M31_CH2: reg = kRegVI2ParamValidPosLine; break;
		case M31_CH3: reg = kRegVI3ParamValidPosLine; break;
	};
	
	return WriteM31VIParam(reg, viprivate10, kRegMaskVIParamPrivate10, kRegShiftVIParamPrivate10);
}

bool CNTV2m31VIParam::GetVIPrivate10(uint32_t* viprivate10, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamValidPosLine; break;
		case M31_CH1: reg = kRegVI1ParamValidPosLine; break;
		case M31_CH2: reg = kRegVI2ParamValidPosLine; break;
		case M31_CH3: reg = kRegVI3ParamValidPosLine; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate10), kRegMaskVIParamPrivate10, kRegShiftVIParamPrivate10);
}

bool CNTV2m31VIParam::SetVIPrivate11(uint32_t viprivate11, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamValidLineCount; break;
		case M31_CH1: reg = kRegVI1ParamValidLineCount; break;
		case M31_CH2: reg = kRegVI2ParamValidLineCount; break;
		case M31_CH3: reg = kRegVI3ParamValidLineCount; break;
	};
	
	return WriteM31VIParam(reg, viprivate11, kRegMaskVIParamPrivate11, kRegShiftVIParamPrivate11);
}

bool CNTV2m31VIParam::GetVIPrivate11(uint32_t* viprivate11, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamValidLineCount; break;
		case M31_CH1: reg = kRegVI1ParamValidLineCount; break;
		case M31_CH2: reg = kRegVI2ParamValidLineCount; break;
		case M31_CH3: reg = kRegVI3ParamValidLineCount; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate11), kRegMaskVIParamPrivate11, kRegShiftVIParamPrivate11);
}

bool CNTV2m31VIParam::SetVIPrivate12(uint32_t viprivate12, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamValidLineCount; break;
		case M31_CH1: reg = kRegVI1ParamValidLineCount; break;
		case M31_CH2: reg = kRegVI2ParamValidLineCount; break;
		case M31_CH3: reg = kRegVI3ParamValidLineCount; break;
	};
	
	return WriteM31VIParam(reg, viprivate12, kRegMaskVIParamPrivate12, kRegShiftVIParamPrivate12);
}

bool CNTV2m31VIParam::GetVIPrivate12(uint32_t* viprivate12, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamValidLineCount; break;
		case M31_CH1: reg = kRegVI1ParamValidLineCount; break;
		case M31_CH2: reg = kRegVI2ParamValidLineCount; break;
		case M31_CH3: reg = kRegVI3ParamValidLineCount; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate12), kRegMaskVIParamPrivate12, kRegShiftVIParamPrivate12);
}

bool CNTV2m31VIParam::SetVIPrivate13(uint32_t viprivate13, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamValidVCount; break;
		case M31_CH1: reg = kRegVI1ParamValidVCount; break;
		case M31_CH2: reg = kRegVI2ParamValidVCount; break;
		case M31_CH3: reg = kRegVI3ParamValidVCount; break;
	};
	
	return WriteM31VIParam(reg, viprivate13, kRegMaskVIParamPrivate13, kRegShiftVIParamPrivate13);
}

bool CNTV2m31VIParam::GetVIPrivate13(uint32_t* viprivate13, M31Channel channel)
{
	VIParamRegisterIndex reg;
	
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: reg = kRegVI0ParamValidVCount; break;
		case M31_CH1: reg = kRegVI1ParamValidVCount; break;
		case M31_CH2: reg = kRegVI2ParamValidVCount; break;
		case M31_CH3: reg = kRegVI3ParamValidVCount; break;
	};
	
	return ReadM31VIParam(reg, reinterpret_cast <uint32_t*> (viprivate13), kRegMaskVIParamPrivate13, kRegShiftVIParamPrivate13);
}


bool CNTV2m31VIParam::GetVIRegister(M31Channel channel, VIParamRegisterIndex* reg)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	switch (channel)
	{
		case M31_CH0: *reg = kRegVI0Param; break;
		case M31_CH1: *reg = kRegVI1Param; break;
		case M31_CH2: *reg = kRegVI2Param; break;
		case M31_CH3: *reg = kRegVI3Param; break;
	};
	
	return true;
}

// These routines will eventually calculate the correct register offset basesd on register enum and channel and using the C start and channel size
// which will eventually call a driver Read/Write function that will write into the params bar
bool CNTV2m31VIParam::WriteM31VIParam(VIParamRegisterIndex registerIndex, uint32_t registerValue, VIParamRegisterMask registerMask, VIParamRegisterShift registerShift)
{
	// Calculate where the register number is based on the register index and start of VIPARAM register block
	uint32_t regNum = VIPARAM_REG_START + (registerIndex * 4);
	
	return mpDevice->HevcWriteRegister(regNum, registerValue, registerMask, registerShift);
}

bool CNTV2m31VIParam::WriteM31VIParam(VIParamRegisterIndex registerIndex, uint32_t registerValue)
{
	// Calculate where the register number is based on the register index and start of VIPARAM register block
	uint32_t regNum = VIPARAM_REG_START + (registerIndex * 4);
	
	return mpDevice->HevcWriteRegister(regNum, registerValue);
}

bool CNTV2m31VIParam::ReadM31VIParam(VIParamRegisterIndex registerIndex, uint32_t* registerValue, VIParamRegisterMask registerMask, VIParamRegisterShift registerShift)
{
	// Calculate where the register number is based on the register index and start of VIPARAM register block
	uint32_t regNum = VIPARAM_REG_START + (registerIndex * 4);

	return mpDevice->HevcReadRegister(regNum, registerValue, registerMask, registerShift);
}

bool CNTV2m31VIParam::ReadM31VIParam(VIParamRegisterIndex registerIndex, uint32_t* registerValue)
{
	// Calculate where the register number is based on the register index and start of VIPARAM register block
	uint32_t regNum = VIPARAM_REG_START + (registerIndex * 4);
	
	return mpDevice->HevcReadRegister(regNum, registerValue);
}


