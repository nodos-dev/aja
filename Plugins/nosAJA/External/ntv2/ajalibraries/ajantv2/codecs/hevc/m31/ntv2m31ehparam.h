/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31ehparam.h
	@brief		Declaration of CNTV2m31EHParam class.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/


#ifndef NTV2M31EHPARAM_H
#define NTV2M31EHPARAM_H

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
	M31CC				eHCC;
	
	M31SourceVA			eHSource;
	M31VirtualChannel	eHSourceId;
	M31EncodeMode		eHHsEncodeMode;
	M31ScanMode			eHInterlace;
	M31ChromaFormat		eHChromaFormat;
	M31BitDepth			eHBitDepth;

	uint32_t			eHHSizeVA;
	uint32_t			eHVSizeVA;
	
	uint32_t			eHHSizeEH;
	uint32_t			eHVSizeEH;
	
	M31Profile			eHProfile;
	uint32_t			eHLevel;
	M31Tier				eHTier;
	
	uint32_t			eHAspectRatio;
	
	uint32_t			eHSARWidth;
	uint32_t			eHSARHeight;
	
	uint32_t			eHFrameNumInGOP;
	uint32_t			eHGOPHierarchy;
	uint32_t			eHUseTemporalID;
	
	uint32_t			eHPASL0B;
	uint32_t			eHIpPeriod;
	uint32_t			eHAdaptiveGOP;
	uint32_t			eHClosedGOP;
	uint32_t			eHIDRInterval;
	
	uint32_t			eHRCMode;
	uint32_t			eHDisableMinQpCtrl;
	uint32_t			eHAdaptiveQuant;
	
	uint32_t			eHBitRate;
	uint32_t			eHVBRMaxBitRate;
	uint32_t			eHVBRAveBitRate;
	uint32_t			eHNumUnitsInTickMax;
	uint32_t			eHTimeScale;
	uint32_t			eHNumUnitsInTick;
	uint32_t			eHCPBDelay;
	
	uint32_t			eHMinCUSize;
	uint32_t			eHMaxTUSize;
	uint32_t			eHMinTUSize;
	
	uint32_t			eHTUDepthIntra;
	uint32_t			eHTUDepthInter;
	
	uint32_t			eHIntraSmoothing;
	uint32_t			eHMergeCand;
		
	uint32_t			eHDF;
	
	uint32_t			eHPPSInsertion;
	uint32_t			eHEOS;
	uint32_t			eHMP4;
	
	uint32_t			eHBufCtrl;
	uint32_t			eHVCLHRD;
	uint32_t			eHVPSTiming;
	
	uint32_t			eHOverscanInfo;
	uint32_t			eHBitRestriction;
	
	uint32_t			eHVideoSignalType;
	uint32_t			eHVideoFormat;
	uint32_t			eHVideoFullRange;
	uint32_t			eHColourDescription;
	uint32_t			eHColourPrimaries;
	uint32_t			eHTransferChar;
	
	uint32_t			eHMatrixCoef;
	uint32_t			eHChromaLocInfo;
	uint32_t			eHSampleLocTop;
	uint32_t			eHSampleLocBot;
	
	uint32_t			eHCropLeft;
	uint32_t			eHCropRight;
	
	uint32_t			eHCropTop;
	uint32_t			eHCropBottom;
	
	uint32_t			eUseTpIrap;
	uint32_t			eHPicTiming;
	uint32_t			eHScanType;
	uint32_t			eHFramePacking;
	
	uint32_t			eHScanLeft;
	uint32_t			eHScanRight;
	
	uint32_t			eHScanTop;
	uint32_t			eHScanBottom;
	
	// Private params not documented
	uint32_t			ehPrivate01;
	uint32_t			ehPrivate02;
	uint32_t			ehPrivate03;
	uint32_t			ehPrivate04;
	uint32_t			ehPrivate05;

	uint32_t			ehPrivate06;
	uint32_t			ehPrivate07;

	uint32_t			ehPrivate08;

	uint32_t			ehPrivate09;
	uint32_t			ehPrivate10;

	uint32_t			ehPrivate11;

	uint32_t			ehPrivate12;
	uint32_t			ehPrivate13;
	uint32_t			ehPrivate14;

	uint32_t			ehPrivate15;
	uint32_t			ehPrivate16;
	
	uint32_t			ehPrivate17;
	uint32_t			ehPrivate18;
	uint32_t			ehPrivate19;
	uint32_t			ehPrivate20;
	uint32_t			ehPrivate21;
	uint32_t			ehPrivate22;
	
	uint32_t			ehPrivate23;
	uint32_t			ehPrivate24;

	uint32_t			ehPrivate25;
	
	uint32_t			ehPrivate26;

	uint32_t			ehPrivate27;
	uint32_t			ehPrivate28;
	
	uint32_t			ehPrivate29;

	uint32_t			ehPrivate30;

} EHParamsChannel;


class AJAExport CNTV2m31EHParam

{
public:
									CNTV2m31EHParam(CNTV2Card* device);
	virtual							~CNTV2m31EHParam();
	
	// Channel structure functions
	AJA_VIRTUAL bool				Clear();
	AJA_VIRTUAL bool				SetEHParamsChannel(EHParamsChannel eHParamsChannel, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHParamsChannel(EHParamsChannel* eHParamsChannel, M31VirtualChannel channel);
	AJA_VIRTUAL bool				LoadEHParamsChannel(M31VideoPreset videoPreset, EHParamsChannel* eHParamsChannel);
	
	// Public set/get functions
	AJA_VIRTUAL bool				SetCC(M31CC cc, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetCC(M31CC* cc, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetSource(M31SourceVA source, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSource(M31SourceVA* source, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetSourceId(M31VirtualChannel sourceId, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSourceId(M31VirtualChannel* sourceId, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetHsEncodeMode(M31EncodeMode hsEncodeMode, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetHsEncodeMode(M31EncodeMode* hsEncodeMode, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetInterlace(M31ScanMode interlace, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetInterlace(M31ScanMode* interlace, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetChromaFormat(M31ChromaFormat chromaFormat, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetChromaFormat(M31ChromaFormat* chromaFormat, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetBitDepth(M31BitDepth bitDepth, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetBitDepth(M31BitDepth* bitDepth, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetHSizeVA(uint32_t hSizeVA, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetHSizeVA(uint32_t* hSizeVA, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVSizeVA(uint32_t vSizeVA, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVSizeVA(uint32_t* vSizeVA, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetHSizeEH(uint32_t hSizeEH, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetHSizeEH(uint32_t* hSizeEH, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVSizeEH(uint32_t vSizeEH, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVSizeEH(uint32_t* vSizeEH, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetProfile(M31Profile profile, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetProfile(M31Profile* profile, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetLevel(uint32_t level, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetLevel(uint32_t* level, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetTier(M31Tier tier, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetTier(M31Tier* tier, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetAspectRatio(uint32_t aspectRatio, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetAspectRatio(uint32_t* aspectRatio, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetSARWidth(uint32_t sarWidth, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSARWidth(uint32_t* sarWidth, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetSARHeight(uint32_t sarHeight, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSARHeight(uint32_t* sarHeight, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetFrameNumInGOP(uint32_t frameNumInGOP, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetFrameNumInGOP(uint32_t* frameNumInGOP, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetGOPHierarchy(uint32_t gopHierarchy, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetGOPHierarchy(uint32_t* gopHierarchy, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetUseTemporalID(uint32_t useTemporalID, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetUseTemporalID(uint32_t* useTemporalID, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetPASL0B(uint32_t pASL0B, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetPASL0B(uint32_t* pASL0B, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetIpPeriod(uint32_t ipPeriod, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetIpPeriod(uint32_t* ipPeriod, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetAdaptiveGOP(uint32_t adaptiveGOP, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetAdaptiveGOP(uint32_t* adaptiveGOP, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetClosedGOP(uint32_t closedGOP, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetClosedGOP(uint32_t* closedGOP, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetIDRInterval(uint32_t idrInterval, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetIDRInterval(uint32_t* idrInterval, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetRCMode(uint32_t rCMode, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetRCMode(uint32_t* rCMode, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetDisableMinQpCtrl(uint32_t disableMinQpCtrl, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetDisableMinQpCtrl(uint32_t* disableMinQpCtrl, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetAdaptiveQuant(uint32_t adaptiveQuant, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetAdaptiveQuant(uint32_t* adaptiveQuant, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetBitRate(uint32_t bitRate, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetBitRate(uint32_t* bitRate, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVBRMaxBitRate(uint32_t vbrMaxBitRate, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVBRMaxBitRate(uint32_t* vbrMaxBitRate, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVBRAveBitRate(uint32_t vbrAveBitRate, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVBRAveBitRate(uint32_t* vbrAveBitRate, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetNumUnitsInTickMax(uint32_t numUnitsInTickMax, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetNumUnitsInTickMax(uint32_t* numUnitsInTickMax, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetTimeScale(uint32_t timeScale, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetTimeScale(uint32_t* timeScale, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetNumUnitsInTick(uint32_t numUnitsInTick, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetNumUnitsInTick(uint32_t* numUnitsInTick, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetCPBDelay(uint32_t cpbDelay, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetCPBDelay(uint32_t* cpbDelay, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetMinCUSize(uint32_t minCUSize, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetMinCUSize(uint32_t* minCUSize, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetMaxTUSize(uint32_t maxTUSize, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetMaxTUSize(uint32_t* maxTUSize, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetMinTUSize(uint32_t minTUSize, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetMinTUSize(uint32_t* minTUSize, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetTUDepthIntra(uint32_t tuDepthIntra, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetTUDepthIntra(uint32_t* tuDepthIntra, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetTUDepthInter(uint32_t tuDepthInter, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetTUDepthInter(uint32_t* tuDepthInter, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetIntraSmoothing(uint32_t intraSmoothing, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetIntraSmoothing(uint32_t* intraSmoothing, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetMergeCand(uint32_t mergeCand, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetMergeCand(uint32_t* mergeCand, M31VirtualChannel channel);
		
	AJA_VIRTUAL bool				SetDF(uint32_t df, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetDF(uint32_t* df, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetPPSInsertion(uint32_t ppsInsertion, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetPPSInsertion(uint32_t* ppsInsertion, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEOS(uint32_t eos, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEOS(uint32_t* eos, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetMP4(uint32_t mp4, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetMP4(uint32_t* mp4, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetBufCtrl(uint32_t bufCtrl, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetBufCtrl(uint32_t* bufCtrl, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVCLHRD(uint32_t vclHRD, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVCLHRD(uint32_t* vclHRD, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVPSTiming(uint32_t vpsTiming, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVPSTiming(uint32_t* vpsTiming, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetOverscanInfo(uint32_t overscanInfo, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetOverscanInfo(uint32_t* overscanInfo, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetBitRestriction(uint32_t bitRestriction, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetBitRestriction(uint32_t* bitRestriction, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetVideoSignalType(uint32_t videoSignalType, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVideoSignalType(uint32_t* videoSignalType, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVideoFormat(uint32_t videoFormat, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVideoFormat(uint32_t* videoFormat, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetVideoFullRange(uint32_t videoFullRange, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetVideoFullRange(uint32_t* videoFullRange, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetColourDescription(uint32_t colourDescription, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetColourDescription(uint32_t* colourDescription, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetColourPrimaries(uint32_t colourPrimaries, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetColourPrimaries(uint32_t* colourPrimaries, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetTransferChar(uint32_t transferChar, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetTransferChar(uint32_t* transferChar, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetMatrixCoef(uint32_t matrixCoef, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetMatrixCoef(uint32_t* matrixCoef, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetChromaLocInfo(uint32_t chromaLocInfo, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetChromaLocInfo(uint32_t* chromaLocInfo, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetSampleLocTop(uint32_t sampleLocTop, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSampleLocTop(uint32_t* sampleLocTop, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetSampleLocBot(uint32_t sampleLocBot, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetSampleLocBot(uint32_t* sampleLocBot, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetCropLeft(uint32_t cropLeft, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetCropLeft(uint32_t* cropLeft, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetCropRight(uint32_t cropRight, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetCropRight(uint32_t* cropRight, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetCropTop(uint32_t cropTop, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetCropTop(uint32_t* cropTop, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetCropBottom(uint32_t cropBottom, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetCropBottom(uint32_t* cropBottom, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetUseTpIrap(uint32_t useTpIrap, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetUseTpIrap(uint32_t* useTpIrap, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetPicTiming(uint32_t picTiming, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetPicTiming(uint32_t* picTiming, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetScanType(uint32_t scanType, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetScanType(uint32_t* scanType, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetFramePacking(uint32_t framePacking, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetFramePacking(uint32_t* framePacking, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetScanLeft(uint32_t scanLeft, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetScanLeft(uint32_t* scanLeft, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetScanRight(uint32_t scanRight, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetScanRight(uint32_t* scanRight, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetScanTop(uint32_t scanTop, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetScanTop(uint32_t* scanTop, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetScanBottom(uint32_t scanBottom, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetScanBottom(uint32_t* scanBottom, M31VirtualChannel channel);
	
protected:
	// Protected set/get functions
	AJA_VIRTUAL bool				SetEHPrivate01(uint32_t ehprivate01, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate01(uint32_t* ehprivate01, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate02(uint32_t ehprivate02, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate02(uint32_t* ehprivate02, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate03(uint32_t ehprivate03, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate03(uint32_t* ehprivate03, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate04(uint32_t ehprivate04, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate04(uint32_t* ehprivate04, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate05(uint32_t ehprivate05, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate05(uint32_t* ehprivate05, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate06(uint32_t ehprivate06, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate06(uint32_t* ehprivate06, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate07(uint32_t ehprivate07, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate07(uint32_t* ehprivate07, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate08(uint32_t ehprivate08, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate08(uint32_t* ehprivate08, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate09(uint32_t ehprivate09, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate09(uint32_t* ehprivate09, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate10(uint32_t ehprivate10, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate10(uint32_t* ehprivate10, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate11(uint32_t ehprivate11, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate11(uint32_t* ehprivate11, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate12(uint32_t ehprivate12, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate12(uint32_t* ehprivate12, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate13(uint32_t ehprivate13, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate13(uint32_t* ehprivate13, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate14(uint32_t ehprivate14, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate14(uint32_t* ehprivate14, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate15(uint32_t ehprivate15, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate15(uint32_t* ehprivate15, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate16(uint32_t ehprivate16, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate16(uint32_t* ehprivate16, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetEHPrivate17(uint32_t ehprivate17, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate17(uint32_t* ehprivate17, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate18(uint32_t ehprivate18, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate18(uint32_t* ehprivate18, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate19(uint32_t ehprivate19, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate19(uint32_t* ehprivate19, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate20(uint32_t ehprivate20, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate20(uint32_t* ehprivate20, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate21(uint32_t ehprivate21, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate21(uint32_t* ehprivate21, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate22(uint32_t ehprivate22, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate22(uint32_t* ehprivate22, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate23(uint32_t ehprivate23, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate23(uint32_t* ehprivate23, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate24(uint32_t ehprivate24, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate24(uint32_t* ehprivate24, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate25(uint32_t ehprivate25, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate25(uint32_t* ehprivate25, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate26(uint32_t ehprivate26, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate26(uint32_t* ehprivate26, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate27(uint32_t ehprivate27, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate27(uint32_t* ehprivate27, M31VirtualChannel channel);
	AJA_VIRTUAL bool				SetEHPrivate28(uint32_t ehprivate28, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate28(uint32_t* ehprivate28, M31VirtualChannel channel);
	
	AJA_VIRTUAL bool				SetEHPrivate29(uint32_t ehprivate29, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate29(uint32_t* ehprivate29, M31VirtualChannel channel);

	AJA_VIRTUAL bool				SetEHPrivate30(uint32_t ehprivate30, M31VirtualChannel channel);
	AJA_VIRTUAL bool				GetEHPrivate30(uint32_t* ehprivate30, M31VirtualChannel channel);

private:
	AJA_VIRTUAL bool				WriteM31EHParam(M31VirtualChannel channel, EHParamRegisterIndex registerIndex, uint32_t registerValue, EHParamRegisterMask registerMask, EHParamRegisterShift registerShift);
	AJA_VIRTUAL bool				WriteM31EHParam(M31VirtualChannel channel, EHParamRegisterIndex registerIndex, uint32_t registerValue);
	AJA_VIRTUAL bool				ReadM31EHParam(M31VirtualChannel channel, EHParamRegisterIndex registerIndex, uint32_t* registerValue, EHParamRegisterMask registerMask, EHParamRegisterShift registerShift);
	AJA_VIRTUAL bool				ReadM31EHParam(M31VirtualChannel channel, EHParamRegisterIndex registerIndex, uint32_t* registerValue);

private:
	CNTV2Card*						mpDevice;

};	//	CNTV2m31EHParam

#endif	//	NTV2M31EHPARAM_H
