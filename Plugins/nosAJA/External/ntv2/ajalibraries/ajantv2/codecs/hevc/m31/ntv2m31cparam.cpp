/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31cparam.cpp
	@brief		Implementation of non-device-specific M31 class register getter/setter methods for "C" params.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/

#include "ntv2m31cparam.h"

// Constructor
CNTV2m31CParam::CNTV2m31CParam(CNTV2Card* device) :
mpDevice (device)
{
}


// Destructor
CNTV2m31CParam::~CNTV2m31CParam()
{
}

// Apparently we write to memory and not actual registers so best to zero out all values before we begin setting bit fields
bool CNTV2m31CParam::Clear()
{
	for (uint32_t i = kRegCParamCC; i <= kRegCParamResoType31_24; i++)
	{
		WriteM31CParam((CParamRegisterIndex)i, 0);
	}
	return true;
}

// Writes a CParamsVIChannel struct to the hardware for a given channel
bool CNTV2m31CParam::SetCParamsChannel(CParamsChannel cParamsChannel, M31Channel channel)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	SetCC(M31_READCC);
	SetStillColor(cParamsChannel.cpStillColorY,
				  cParamsChannel.cpStillColorCr,
				  cParamsChannel.cpStillColorCb);
	SetScRobustMode(cParamsChannel.cpRobustMode);
	SetScBuf(cParamsChannel.cpScBuf);
	SetResoType(cParamsChannel.cpResoType, (M31VirtualChannel)channel);
	
	return true;
}

// Reads a CParamsVIChannel struct from the hardware for a given channel
bool CNTV2m31CParam::GetCParamsChannel(CParamsChannel* cParamsChannel, M31Channel channel)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31Channel(channel))
		return false;
	
	GetStillColor(&cParamsChannel->cpStillColorY,
				  &cParamsChannel->cpStillColorCr,
				  &cParamsChannel->cpStillColorCb);
	GetScRobustMode(&cParamsChannel->cpRobustMode);
	GetScBuf(&cParamsChannel->cpScBuf);
	GetResoType(&cParamsChannel->cpResoType, (M31VirtualChannel)channel);
	
	return true;
}

// Loads up a CParamsVIChannel struct with all the default values for a given video preset
bool CNTV2m31CParam::LoadCParamsChannel(M31VideoPreset videoPreset, CParamsChannel* cParamsChannel)
{
	// Make sure we pass in a valid preset
	if (!IS_VALID_M31VideoPreset(videoPreset))
		return false;
	
	// Start with a clean slate
	memset(cParamsChannel, 0, sizeof(CParamsChannel));
	
	// default still color
	cParamsChannel->cpStillColorY = 0x10;
	cParamsChannel->cpStillColorCb = 0x80;
	cParamsChannel->cpStillColorCr = 0x80;

	// defualt robust mode
	cParamsChannel->cpRobustMode = M31_RobustModeBlue;

	// Looks like this is set to 3 for all presets, indicates enable scene detection
	cParamsChannel->cpScBuf	 = 0;
	
	switch (videoPreset)
	{
		case M31_FILE_720X480_420_8_5994i:
		case M31_FILE_720X480_420_8_5994p:
		case M31_FILE_720X480_420_8_60i:
		case M31_FILE_720X480_420_8_60p:
		case M31_FILE_720X480_422_10_5994i:
		case M31_FILE_720X480_422_10_5994p:
		case M31_FILE_720X480_422_10_60i:
		case M31_FILE_720X480_422_10_60p:
		case M31_FILE_720X576_420_8_50i:
		case M31_FILE_720X576_420_8_50p:
		case M31_FILE_720X576_422_10_50i:
		case M31_FILE_720X576_422_10_50p:
		case M31_VIF_720X480_420_8_5994i:
		case M31_VIF_720X480_420_8_5994p:
		case M31_VIF_720X480_420_8_60i:
		case M31_VIF_720X480_420_8_60p:
		case M31_VIF_720X480_422_10_5994i:
		case M31_VIF_720X480_422_10_5994p:
		case M31_VIF_720X480_422_10_60i:
		case M31_VIF_720X480_422_10_60p:
		case M31_VIF_720X576_420_8_50i:
		case M31_VIF_720X576_420_8_50p:
		case M31_VIF_720X576_422_10_50i:
		case M31_VIF_720X576_422_10_50p:
			cParamsChannel->cpResoType			= M31_SD;
			break;

		case M31_FILE_1280X720_420_8_2398p:
		case M31_FILE_1280X720_420_8_24p:
		case M31_FILE_1280X720_420_8_25p:
		case M31_FILE_1280X720_420_8_2997p:
		case M31_FILE_1280X720_420_8_30p:
		case M31_FILE_1280X720_420_8_50p:
		case M31_FILE_1280X720_420_8_5994p:
		case M31_FILE_1280X720_420_8_60p:
		case M31_FILE_1280X720_422_10_2398p:
		case M31_FILE_1280X720_422_10_24p:
		case M31_FILE_1280X720_422_10_25p:
		case M31_FILE_1280X720_422_10_2997p:
		case M31_FILE_1280X720_422_10_30p:
		case M31_FILE_1280X720_422_10_50p:
		case M31_FILE_1280X720_422_10_5994p:
		case M31_FILE_1280X720_422_10_60p:
		case M31_VIF_1280X720_420_8_50p:
		case M31_VIF_1280X720_420_8_5994p:
		case M31_VIF_1280X720_420_8_60p:
		case M31_VIF_1280X720_422_10_50p:
		case M31_VIF_1280X720_422_10_5994p:
		case M31_VIF_1280X720_422_10_60p:
			cParamsChannel->cpResoType			= M31_HD;
			break;

		case M31_FILE_1920X1080_420_8_50i:
		case M31_FILE_1920X1080_420_8_2398p:
		case M31_FILE_1920X1080_420_8_24p:
		case M31_FILE_1920X1080_420_8_25p:
		case M31_FILE_1920X1080_420_8_2997p:
		case M31_FILE_1920X1080_420_8_30p:
		case M31_FILE_1920X1080_420_8_50p:
		case M31_FILE_1920X1080_420_8_5994i:
		case M31_FILE_1920X1080_420_8_5994p:
		case M31_FILE_1920X1080_420_8_60i:
		case M31_FILE_1920X1080_420_8_60p:
		case M31_FILE_1920X1080_422_10_50i:
		case M31_FILE_1920X1080_422_10_2398p:
		case M31_FILE_1920X1080_422_10_24p:
		case M31_FILE_1920X1080_422_10_25p:
		case M31_FILE_1920X1080_422_10_2997p:
		case M31_FILE_1920X1080_422_10_30p:
		case M31_FILE_1920X1080_422_10_50p:
		case M31_FILE_1920X1080_422_10_5994i:
		case M31_FILE_1920X1080_422_10_5994p:
		case M31_FILE_1920X1080_422_10_60i:
		case M31_FILE_1920X1080_422_10_60p:
		case M31_VIF_1920X1080_420_8_50i:
		case M31_VIF_1920X1080_420_8_50p:
		case M31_VIF_1920X1080_420_8_5994i:
		case M31_VIF_1920X1080_420_8_5994p:
		case M31_VIF_1920X1080_420_8_60i:
		case M31_VIF_1920X1080_420_8_60p:
		case M31_VIF_1920X1080_420_10_50i:
		case M31_VIF_1920X1080_420_10_50p:
		case M31_VIF_1920X1080_420_10_5994i:
		case M31_VIF_1920X1080_420_10_5994p:
		case M31_VIF_1920X1080_420_10_60i:
		case M31_VIF_1920X1080_420_10_60p:
		case M31_VIF_1920X1080_422_10_5994i:
		case M31_VIF_1920X1080_422_10_5994p:
		case M31_VIF_1920X1080_422_10_60i:
		case M31_VIF_1920X1080_422_10_60p:
			cParamsChannel->cpResoType			= M31_FULLHD;
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
			cParamsChannel->cpResoType			= M31_FULLHD;
			break;

		case M31_FILE_3840X2160_420_8_2398p:
		case M31_FILE_3840X2160_420_8_24p:
		case M31_FILE_3840X2160_420_8_25p:
		case M31_FILE_3840X2160_420_8_2997p:
		case M31_FILE_3840X2160_420_8_30p:
		case M31_FILE_3840X2160_420_8_50p:
		case M31_FILE_3840X2160_420_8_5994p:
		case M31_FILE_3840X2160_420_8_60p:
		case M31_FILE_3840X2160_420_10_50p:
		case M31_FILE_3840X2160_420_10_5994p:
		case M31_FILE_3840X2160_420_10_60p:
		case M31_FILE_3840X2160_422_8_2398p:
		case M31_FILE_3840X2160_422_8_24p:
		case M31_FILE_3840X2160_422_8_25p:
		case M31_FILE_3840X2160_422_8_2997p:
		case M31_FILE_3840X2160_422_8_30p:
		case M31_FILE_3840X2160_422_8_50p:
		case M31_FILE_3840X2160_422_8_5994p:
		case M31_FILE_3840X2160_422_8_60p:
		case M31_FILE_3840X2160_422_10_2398p:
		case M31_FILE_3840X2160_422_10_24p:
		case M31_FILE_3840X2160_422_10_25p:
		case M31_FILE_3840X2160_422_10_2997p:
		case M31_FILE_3840X2160_422_10_30p:
		case M31_FILE_3840X2160_422_10_50p:
		case M31_FILE_3840X2160_422_10_5994p:
		case M31_FILE_3840X2160_422_10_60p:
		case M31_FILE_4096X2160_420_10_5994p:
		case M31_FILE_4096X2160_420_10_60p:
		case M31_FILE_4096X2160_422_10_50p:
		case M31_FILE_4096X2160_422_10_5994p_IF:
		case M31_FILE_4096X2160_422_10_60p_IF:
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
			cParamsChannel->cpResoType			= M31_4K2K;
			break;
			
		case M31_NUMVIDEOPRESETS:
			break;
	}
	
	return true;
}

bool CNTV2m31CParam::SetCC(M31CC cc)
{
	return WriteM31CParam(kRegCParamCC, cc, kRegMaskCParamCC, kRegShiftCParamCC);
}

bool CNTV2m31CParam::GetCC(M31CC* cc)
{
	return ReadM31CParam(kRegCParamCC, reinterpret_cast <uint32_t*> (cc), kRegMaskCParamCC, kRegShiftCParamCC);
}

bool CNTV2m31CParam::SetStillColor(uint8_t y, uint8_t cb, uint8_t cr)
{
	if (!WriteM31CParam(kRegCParamStillColor, y, kRegMaskCParamStillColorY, kRegShiftCParamStillColorY)) return false;
	if (!WriteM31CParam(kRegCParamStillColor, cb, kRegMaskCParamStillColorCb, kRegShiftCParamStillColorCb)) return false;
	if (!WriteM31CParam(kRegCParamStillColor, cr, kRegMaskCParamStillColorCr, kRegShiftCParamStillColorCr)) return false;
	return true;
}

bool CNTV2m31CParam::GetStillColor(uint8_t* y, uint8_t* cb, uint8_t* cr)
{
	if (!ReadM31CParam(kRegCParamStillColor, reinterpret_cast <uint32_t*> (y),
					   kRegMaskCParamStillColorY, kRegShiftCParamStillColorY)) return false;
	if (!ReadM31CParam(kRegCParamStillColor, reinterpret_cast <uint32_t*> (cb),
					   kRegMaskCParamStillColorCb, kRegShiftCParamStillColorCb)) return false;
	if (!ReadM31CParam(kRegCParamStillColor, reinterpret_cast <uint32_t*> (cr),
					   kRegMaskCParamStillColorCr, kRegShiftCParamStillColorCr)) return false;
	return true;
}

bool CNTV2m31CParam::SetScRobustMode(M31RobustMode robustMode)
{
	return WriteM31CParam(kRegCParamScBuf, robustMode, kRegMaskCParamScRobustMode, kRegShiftCParamScRobustMode);
}

bool CNTV2m31CParam::GetScRobustMode(M31RobustMode* robustMode)
{
	return ReadM31CParam(kRegCParamScBuf, reinterpret_cast <uint32_t*> (robustMode), kRegMaskCParamScRobustMode, kRegShiftCParamScRobustMode);
}

bool CNTV2m31CParam::SetScBuf(uint32_t scBuf)
{
	return WriteM31CParam(kRegCParamScBuf, scBuf, kRegMaskCParamScBuf, kRegShiftCParamScBuf);
}

bool CNTV2m31CParam::GetScBuf(uint32_t* scBuf)
{
	return ReadM31CParam(kRegCParamScBuf, reinterpret_cast <uint32_t*> (scBuf), kRegMaskCParamScBuf, kRegShiftCParamScBuf);
}

bool CNTV2m31CParam::SetResoType(M31ResoType resoType, M31VirtualChannel channel)
{
	(void) channel;
	CParamRegisterIndex		reg;
	CParamRegisterMask		mask;
	CParamRegisterShift		shift;
	
	// Make sure we pass in a valid cc
	if (!IS_VALID_M31ResoType(resoType))
		return false;

	// use 4k for 4k encode otherwise fullhd
	if (resoType != M31_4K2K)
		resoType = M31_FULLHD;

	// set all resolutions the same for now
	if (!GetResoRegisters(M31_VIRTUAL_CH0, &reg, &mask, &shift)) return false;
	if (!WriteM31CParam(reg, resoType, mask, shift)) return false;

	if (!GetResoRegisters(M31_VIRTUAL_CH1, &reg, &mask, &shift)) return false;
	if (!WriteM31CParam(reg, resoType, mask, shift)) return false;

	if (!GetResoRegisters(M31_VIRTUAL_CH2, &reg, &mask, &shift)) return false;
	if (!WriteM31CParam(reg, resoType, mask, shift)) return false;

	if (!GetResoRegisters(M31_VIRTUAL_CH3, &reg, &mask, &shift)) return false;
	if (!WriteM31CParam(reg, resoType, mask, shift)) return false;

	return true;
}

bool CNTV2m31CParam::GetResoType(M31ResoType* resoType, M31VirtualChannel channel)
{
	CParamRegisterIndex		reg;
	CParamRegisterMask		mask;
	CParamRegisterShift		shift;

	if (!GetResoRegisters(channel, &reg, &mask, &shift)) return false;
	if (!ReadM31CParam(reg, reinterpret_cast <uint32_t*> (resoType), mask, shift)) return false;
	return true;
}

bool CNTV2m31CParam::GetResoRegisters(M31VirtualChannel channel, CParamRegisterIndex* reg, CParamRegisterMask* mask, CParamRegisterShift* shift)
{
	if (channel <=7)
		*reg = kRegCParamResoType7_0;
	else if (channel >= 8 && channel <=15)
		*reg = kRegCParamResoType15_8;
	else if (channel >= 16 && channel <=23)
		*reg = kRegCParamResoType23_16;
	else if (channel >= 24 && channel <=31)
		*reg = kRegCParamResoType31_24;
	else
	{
		// Failure condition but set these incase no one is paying attention to the return value
		*reg = kRegCParamResoType7_0;
		*mask = kRegMaskCParamResoType0;
		*shift = kRegShiftCParamResoType0;
		return false;
	}

	switch (channel)
	{
		case 0:
		case 8:
		case 16:
		case 24:
			*mask = kRegMaskCParamResoType0;
			*shift = kRegShiftCParamResoType0;
			break;
			
		case 1:
		case 9:
		case 17:
		case 25:
			*mask = kRegMaskCParamResoType1;
			*shift = kRegShiftCParamResoType1;
			break;
			
		case 2:
		case 10:
		case 18:
		case 26:
			*mask = kRegMaskCParamResoType2;
			*shift = kRegShiftCParamResoType2;
			break;
			
		case 3:
		case 11:
		case 19:
		case 27:
			*mask = kRegMaskCParamResoType3;
			*shift = kRegShiftCParamResoType3;
			break;
			
		case 4:
		case 12:
		case 20:
		case 28:
			*mask = kRegMaskCParamResoType4;
			*shift = kRegShiftCParamResoType4;
			break;
			
		case 5:
		case 13:
		case 21:
		case 29:
			*mask = kRegMaskCParamResoType5;
			*shift = kRegShiftCParamResoType5;
			break;
			
		case 6:
		case 14:
		case 22:
		case 30:
			*mask = kRegMaskCParamResoType6;
			*shift = kRegShiftCParamResoType6;
			break;
			
		case 7:
		case 15:
		case 23:
		case 31:
			*mask = kRegMaskCParamResoType7;
			*shift = kRegShiftCParamResoType7;
			break;
			
		default:
			return false;
	};
	return true;
}

// These routines will eventually calculate the correct register offset basesd on register enum and channel and using the C start and channel size
// which will eventually call a driver Read/Write function that will write into the params bar
bool CNTV2m31CParam::WriteM31CParam(CParamRegisterIndex registerIndex, uint32_t registerValue, CParamRegisterMask registerMask, CParamRegisterShift registerShift)
{
	// Calculate where the register number is based on the register index and start of CPARAM register block
	uint32_t regNum = CPARAM_REG_START + (registerIndex * 4);
	
	return mpDevice->HevcWriteRegister(regNum, registerValue, registerMask, registerShift);
}

bool CNTV2m31CParam::WriteM31CParam(CParamRegisterIndex registerIndex, uint32_t registerValue)
{
	// Calculate where the register number is based on the register index and start of CPARAM register block
	uint32_t regNum = CPARAM_REG_START + (registerIndex * 4);
	
	return mpDevice->HevcWriteRegister(regNum, registerValue);
}

bool CNTV2m31CParam::ReadM31CParam(CParamRegisterIndex registerIndex, uint32_t* registerValue, CParamRegisterMask registerMask, CParamRegisterShift registerShift)
{
	// Calculate where the register number is based on the register index and start of CPARAM register block
	uint32_t regNum = CPARAM_REG_START + (registerIndex * 4);

	return mpDevice->HevcReadRegister(regNum, registerValue, registerMask, registerShift);
}

bool CNTV2m31CParam::ReadM31CParam(CParamRegisterIndex registerIndex, uint32_t* registerValue)
{
	// Calculate where the register number is based on the register index and start of CPARAM register block
	uint32_t regNum = CPARAM_REG_START + (registerIndex * 4);
	
	return mpDevice->HevcReadRegister(regNum, registerValue);
}

