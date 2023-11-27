/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31viparam.h
	@brief		Declaration of CNTV2m31VIParam class.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/


#ifndef NTV2M31VIPARAM_H
#define NTV2M31VIPARAM_H

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
typedef struct
{
	// Public documented params
	M31YCMode			vIYCMux;
	uint32_t			vIYCSwap;
	M31VideoFormat		vIFormat;
	uint32_t			vIGPIOEncStart;
	uint32_t			vISyncMembers;
	M31SyncMasterMode	vISyncMaster;
	M31InputPort		vIInputPort;
	
	// Private params not documented
	uint32_t			vIPrivate01;
	uint32_t			vIPrivate02;
	uint32_t			vIPrivate03;
	uint32_t			vIPrivate04;
	uint32_t			vIPrivate05;
	uint32_t			vIPrivate06;
	uint32_t			vIPrivate07;
	uint32_t			vIPrivate08;
	uint32_t			vIPrivate09;
	uint32_t			vIPrivate10;
	uint32_t			vIPrivate11;
	uint32_t			vIPrivate12;
	uint32_t			vIPrivate13;

} VIParamsChannel, *pVIParamsChannel;


class AJAExport CNTV2m31VIParam

{
public:
									CNTV2m31VIParam(CNTV2Card* device);
	virtual							~CNTV2m31VIParam();
	
	// Channel structure functions
	AJA_VIRTUAL bool				Clear();
	AJA_VIRTUAL bool				SetVIParamsChannel(VIParamsChannel vIParamsChannel, M31Channel channel);
	AJA_VIRTUAL bool				GetVIParamsChannel(VIParamsChannel* vIParamsChannel, M31Channel channel);
	AJA_VIRTUAL bool				LoadVIParamsChannel(M31VideoPreset videoPreset, VIParamsChannel* vIParamsChannel);

	// Public set/get functions
	AJA_VIRTUAL bool				SetCC(M31CC cc, M31Channel channel);
	AJA_VIRTUAL bool				GetCC(M31CC* cc, M31Channel channel);
	AJA_VIRTUAL bool				SetYCMux(M31YCMode ycMux, M31Channel channel);
	AJA_VIRTUAL bool				GetYCMux(M31YCMode* ycMux, M31Channel channel);
	AJA_VIRTUAL bool				SetYCSwap(uint32_t ycSwap, M31Channel channel);
	AJA_VIRTUAL bool				GetYCSwap(uint32_t* ycSwap, M31Channel channel);
	AJA_VIRTUAL bool				SetFormat(M31VideoFormat format, M31Channel channel);
	AJA_VIRTUAL bool				GetFormat(M31VideoFormat* format, M31Channel channel);
	AJA_VIRTUAL bool				SetGPIOEncStart(uint32_t gpioEncStart, M31Channel channel);
	AJA_VIRTUAL bool				GetGPIOEncStart(uint32_t* gpioEncStart, M31Channel channel);
	AJA_VIRTUAL bool				SetSyncMembers(uint32_t syncMembers, M31Channel channel);
	AJA_VIRTUAL bool				GetSyncMembers(uint32_t* syncMembers, M31Channel channel);
	AJA_VIRTUAL bool				SetSyncMaster(M31SyncMasterMode syncMaster, M31Channel channel);
	AJA_VIRTUAL bool				GetSyncMaster(M31SyncMasterMode* syncMaster, M31Channel channel);
	AJA_VIRTUAL bool				SetInputPort(M31InputPort inputPort, M31Channel channel);
	AJA_VIRTUAL bool				GetInputPort(M31InputPort* inputPort, M31Channel channel);

protected:
	// Protected set/get functions
	AJA_VIRTUAL bool				SetVIPrivate01(uint32_t viprivate01, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate01(uint32_t* viprivate01, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate02(uint32_t viprivate02, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate02(uint32_t* viprivate02, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate03(uint32_t viprivate03, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate03(uint32_t* viprivate03, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate04(uint32_t viprivate04, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate04(uint32_t* viprivate04, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate05(uint32_t viprivate05, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate05(uint32_t* viprivate05, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate06(uint32_t viprivate06, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate06(uint32_t* viprivate06, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate07(uint32_t viprivate07, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate07(uint32_t* viprivate07, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate08(uint32_t viprivate08, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate08(uint32_t* viprivate08, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate09(uint32_t viprivate09, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate09(uint32_t* viprivate09, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate10(uint32_t viprivate10, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate10(uint32_t* viprivate10, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate11(uint32_t viprivate11, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate11(uint32_t* viprivate11, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate12(uint32_t viprivate12, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate12(uint32_t* viprivate12, M31Channel channel);
	AJA_VIRTUAL bool				SetVIPrivate13(uint32_t viprivate13, M31Channel channel);
	AJA_VIRTUAL bool				GetVIPrivate13(uint32_t* viprivate13, M31Channel channel);

private:
	// Private utility functions
	AJA_VIRTUAL bool				WriteM31VIParam(VIParamRegisterIndex registerIndex, uint32_t registerValue, VIParamRegisterMask registerMask, VIParamRegisterShift registerShift);
	AJA_VIRTUAL bool				WriteM31VIParam(VIParamRegisterIndex registerIndex, uint32_t registerValue);
	AJA_VIRTUAL bool				ReadM31VIParam(VIParamRegisterIndex registerIndex, uint32_t* registerValue, VIParamRegisterMask registerMask, VIParamRegisterShift registerShift);
	AJA_VIRTUAL bool				ReadM31VIParam(VIParamRegisterIndex registerIndex, uint32_t* registerValue);
	AJA_VIRTUAL bool				GetVIRegister(M31Channel channel, VIParamRegisterIndex* reg);
	
private:
	CNTV2Card*						mpDevice;
	
};	//	CNTV2m31CParam

#endif	//	NTV2M31CPARAM_H
