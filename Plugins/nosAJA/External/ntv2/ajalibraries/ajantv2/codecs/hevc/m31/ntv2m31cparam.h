/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31cparam.h
	@brief		Declaration of CNTV2m31CParam class.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/


#ifndef NTV2M31CPARAM_H
#define NTV2M31CPARAM_H

#include "ntv2m31publicinterface.h"
#include "ntv2card.h"

#if defined (MSWindows)
#include "ntv2windriverinterface.h"
#elif defined (AJAMac)
#include "ntv2macdriverinterface.h"
#elif defined (AJALinux)
#include "ntv2linuxdriverinterface.h"
#endif

// This is the structure for a single CParams channel, currently there are 4 channels defined
typedef struct {
	M31CC				cpCC;
	uint8_t				cpStillColorY;
	uint8_t				cpStillColorCb;
	uint8_t				cpStillColorCr;
	M31RobustMode		cpRobustMode;
	uint32_t			cpScBuf;
	M31ResoType			cpResoType;
} CParamsChannel, *pCParamsChannel;


class AJAExport CNTV2m31CParam

{
public:
									CNTV2m31CParam(CNTV2Card* device);
	virtual							~CNTV2m31CParam();
	
	// Channel structure functions
	AJA_VIRTUAL bool				Clear();
	AJA_VIRTUAL bool				SetCParamsChannel(CParamsChannel cParamsVIChannel, M31Channel channel);
	AJA_VIRTUAL bool				GetCParamsChannel(CParamsChannel* cParamsVIChannel, M31Channel channel);
	AJA_VIRTUAL bool				LoadCParamsChannel(M31VideoPreset videoPreset, CParamsChannel* cParamsVIChannel);
	
	// Global set/get functions
	AJA_VIRTUAL bool				SetCC(M31CC cc);
	AJA_VIRTUAL bool				GetCC(M31CC* cc);
	AJA_VIRTUAL bool				SetStillColor(uint8_t y, uint8_t cb, uint8_t cr);
	AJA_VIRTUAL bool				GetStillColor(uint8_t* y, uint8_t* cb, uint8_t* cr);
	AJA_VIRTUAL bool				SetScRobustMode(M31RobustMode robustMode);
	AJA_VIRTUAL bool				GetScRobustMode(M31RobustMode* robustMode);

	AJA_VIRTUAL bool				SetScBuf(uint32_t scBuf);
	AJA_VIRTUAL bool				GetScBuf(uint32_t* scBuf);

	// Channel based set/get functions
	AJA_VIRTUAL bool				SetResoType(M31ResoType resoType, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetResoType(M31ResoType* resoType, M31VirtualChannel channel);

private:
	// Private utility functions
	AJA_VIRTUAL bool				GetResoRegisters(M31VirtualChannel channel, CParamRegisterIndex* reg, CParamRegisterMask* mask, CParamRegisterShift* shift);
	AJA_VIRTUAL bool				WriteM31CParam(CParamRegisterIndex registerIndex, uint32_t registerValue, CParamRegisterMask registerMask, CParamRegisterShift registerShift);
	AJA_VIRTUAL bool				WriteM31CParam(CParamRegisterIndex registerIndex, uint32_t registerValue);
	AJA_VIRTUAL bool				ReadM31CParam(CParamRegisterIndex registerIndex, uint32_t* registerValue, CParamRegisterMask registerMask, CParamRegisterShift registerShift);
	AJA_VIRTUAL bool				ReadM31CParam(CParamRegisterIndex registerIndex, uint32_t* registerValue);
	
private:
	CNTV2Card*						mpDevice;
	
};	//	CNTV2m31CParam

#endif	//	NTV2M31CPARAM_H
