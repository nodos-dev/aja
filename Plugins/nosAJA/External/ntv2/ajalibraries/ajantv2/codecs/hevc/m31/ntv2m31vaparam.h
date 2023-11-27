/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31vaparam.h
	@brief		Declaration of CNTV2m31VAParam class.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/


#ifndef NTV2M31VAPARAM_H
#define NTV2M31VAPARAM_H

#include "ntv2m31publicinterface.h"
#include "ntv2card.h"

#if defined (MSWindows)
	#include "ntv2windriverinterface.h"
#elif defined (AJAMac)
	#include "ntv2macdriverinterface.h"
#elif defined (AJALinux)
	#include "ntv2linuxdriverinterface.h"
#endif

typedef struct
{
	// Public documented params
	M31CC				vACC;
	M31SourceVA			vASource;
	M31VirtualChannel	vASourceId;
	M31ScanMode			vAInterlace;
	M31ChromaFormat		vAChromaFormat;
	M31BitDepth			vABitDepth;
	
	M31FrameRate		vAFrameRate;
	M31ChromaFormat		vAChromaFormatOut;
	M31BitDepth			vABitDepthOut;

	uint32_t			vAHSizeVA;
	uint32_t			vAVSizeVA;
	uint32_t			vAHSizeEH;
	uint32_t			vAVSizeEH;
	
	uint32_t			vACoef0;
	uint32_t			vACoef1;
	uint32_t			vACoef2;
	uint32_t			vACoef3;
	uint32_t			vACoef4;
	uint32_t			vACoef5;
	uint32_t			vACoef6;
	uint32_t			vACoef7;

	uint32_t			vASceneChange;
	
	// Private params not documented
	uint32_t			vAPrivate01;
	uint32_t			vAPrivate02;

} VAParamsChannel;


class AJAExport CNTV2m31VAParam

{
public:
									CNTV2m31VAParam(CNTV2Card* device);
	virtual							~CNTV2m31VAParam();
	
	// Channel structure functions
	AJA_VIRTUAL bool				Clear();
	AJA_VIRTUAL bool				SetVAParamsChannel(VAParamsChannel vAParamsChannel, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVAParamsChannel(VAParamsChannel* vAParamsChannel, M31VirtualChannel channel);
	AJA_VIRTUAL bool				LoadVAParamsChannel(M31VideoPreset videoPreset, VAParamsChannel* vAParamsChannel);
	
	// Public set/get functions
	AJA_VIRTUAL bool				SetCC(M31CC cc, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetCC(M31CC* cc, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetSource(M31SourceVA source, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSource(M31SourceVA* source, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetSourceId(M31VirtualChannel sourceId, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSourceId(M31VirtualChannel* sourceId, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetInterlace(M31ScanMode interlace, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetInterlace(M31ScanMode* interlace, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetChromaFormat(M31ChromaFormat chromaFormat, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetChromaFormat(M31ChromaFormat* chromaFormat, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetBitDepth(M31BitDepth bitDepth, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetBitDepth(M31BitDepth* bitDepth, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetFrameRate(M31FrameRate frameRate, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetFrameRate(M31FrameRate* frameRate, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetChromaFormatOut(M31ChromaFormat chromaFormatOut, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetChromaFormatOut(M31ChromaFormat* chromaFormatOut, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVInPrivate01(M31BitDepth vinprivate01, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVInPrivate01(M31BitDepth* vinprivate01, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetHSizeVA(uint32_t hSizeVA, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetHSizeVA(uint32_t* hSizeVA, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVSizeVA(uint32_t vSizeVA, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVSizeVA(uint32_t* vSizeVA, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetHSizeEH(uint32_t hSizeEH, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetHSizeEH(uint32_t* hSizeEH, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVSizeEH(uint32_t vSizeEH, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVSizeEH(uint32_t* vSizeEH, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetCoef(uint32_t coef, uint32_t coefNum, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetCoef(uint32_t* coef, uint32_t coefNum, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetSceneChange(uint32_t sceneChange, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSceneChange(uint32_t* sceneChange, M31VirtualChannel channel);

protected:
	// Protected set/get functions
	AJA_VIRTUAL bool				SetVAPrivate01(uint32_t vaprivate01, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVAPrivate01(uint32_t* vaprivate01, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVAPrivate02(uint32_t vaprivate02, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVAPrivate02(uint32_t* vaprivate02, M31VirtualChannel channel);

private:
	AJA_VIRTUAL bool				WriteM31VAParam(M31VirtualChannel channel, VAParamRegisterIndex registerIndex, uint32_t registerValue, VAParamRegisterMask registerMask, VAParamRegisterShift registerShift);
	AJA_VIRTUAL bool				WriteM31VAParam(M31VirtualChannel channel, VAParamRegisterIndex registerIndex, uint32_t registerValue);
	AJA_VIRTUAL bool				ReadM31VAParam(M31VirtualChannel channel, VAParamRegisterIndex registerIndex, uint32_t* registerValue, VAParamRegisterMask registerMask, VAParamRegisterShift registerShift);
	AJA_VIRTUAL bool				ReadM31VAParam(M31VirtualChannel channel, VAParamRegisterIndex registerIndex, uint32_t* registerValue);

private:
	CNTV2Card*						mpDevice;

};	//	CNTV2m31VAParam

#endif	//	NTV2M31VAPARAM_H
