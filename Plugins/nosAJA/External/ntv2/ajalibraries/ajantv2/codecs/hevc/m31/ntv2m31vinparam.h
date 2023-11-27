/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31vinparam.h
	@brief		Declaration of CNTV2m31VInParam class.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/


#ifndef NTV2M31VINPARAM_H
#define NTV2M31VINPARAM_H

#include "ntv2m31publicinterface.h"
#include "ntv2card.h"

#if defined (MSWindows)
	#include "ntv2windriverinterface.h"
#elif defined (AJAMac)
	#include "ntv2macdriverinterface.h"
#elif defined (AJALinux)
	#include "ntv2linuxdriverinterface.h"
#endif

// This is the structure for a single VInParams channel, currently there are 32 channels defined
typedef struct
{
	// Public documented params
	M31CC				vInCC;
	M31SourceVI			vInSource;
	M31VirtualChannel	vInSourceID;
	M31ChromaFormat		vInChromaFormat;
	M31BitDepth			vInBitDepth;
	M31FrameRate		vInFrameRate;
	
	uint32_t			vInHSize;
	uint32_t			vInVSize;

	M31PTSMode			vInPTSMode;
	uint32_t			vInInitialPTSMSB;
	
	uint32_t			vInInitialPTS;
	uint32_t			vInInitialSerialNum;

	// Private params not documented
	uint32_t			vInPrivate01;

} VInParamsChannel;


class AJAExport CNTV2m31VInParam

{
public:
									CNTV2m31VInParam(CNTV2Card* device);
	virtual							~CNTV2m31VInParam();
	
	// Channel structure functions
	AJA_VIRTUAL bool				Clear();
	AJA_VIRTUAL bool				SetVInParamsChannel(VInParamsChannel vInParamsChannel, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVInParamsChannel(VInParamsChannel* vInParamsChannel, M31VirtualChannel channel);
	AJA_VIRTUAL bool				LoadVInParamsChannel(M31VideoPreset videoPreset, VInParamsChannel* vInParamsChannel);

	// Public set/get functions
	AJA_VIRTUAL bool				SetCC(M31CC cc, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetCC(M31CC* cc, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetSource(M31SourceVI source, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSource(M31SourceVI* source, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetSourceId(M31VirtualChannel sourceId, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSourceId(M31VirtualChannel* sourceId, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetChromaFormat(M31ChromaFormat chromaFormat, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetChromaFormat(M31ChromaFormat* chromaFormat, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetBitDepth(M31BitDepth bitDepth, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetBitDepth(M31BitDepth* bitDepth, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVInFrameRate(M31FrameRate frameRate, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVInFrameRate(M31FrameRate* frameRate, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetHSize(uint32_t hSize, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetHSize(uint32_t* hSize, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVSize(uint32_t vSize, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVSize(uint32_t* vSize, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetPTSMode(M31PTSMode ptsMode, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetPTSMode(M31PTSMode* ptsMode, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetInitialPTSMSB(uint32_t initialPTSMSB, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetInitialPTSMSB(uint32_t* initialPTSMSB, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetInitialPTS(uint32_t initialPTS, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetInitialPTS(uint32_t* initialPTS, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetInitialSerialNum(uint32_t initialSerialNum, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetInitialSerialNum(uint32_t* initialSerialNum, M31VirtualChannel channel);

protected:
	// Protected set/get functions
	AJA_VIRTUAL bool				SetVInPrivate01(uint32_t vinprivate01, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVInPrivate01(uint32_t* vinprivate01, M31VirtualChannel channel);

private:
	AJA_VIRTUAL bool				WriteM31VInParam(M31VirtualChannel channel, VINParamRegisterIndex registerIndex, uint32_t registerValue, VINParamRegisterMask registerMask, VINParamRegisterShift registerShift);
	AJA_VIRTUAL bool				WriteM31VInParam(M31VirtualChannel channel, VINParamRegisterIndex registerIndex, uint32_t registerValue);
	AJA_VIRTUAL bool				ReadM31VInParam(M31VirtualChannel channel, VINParamRegisterIndex registerIndex, uint32_t* registerValue, VINParamRegisterMask registerMask, VINParamRegisterShift registerShift);
	AJA_VIRTUAL bool				ReadM31VInParam(M31VirtualChannel channel, VINParamRegisterIndex registerIndex, uint32_t* registerValue);

private:
	CNTV2Card*						mpDevice;

};	//	CNTV2m31VInParam

#endif	//	NTV2M31VINPARAM_H
