/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31ehparam.cpp
	@brief		Implementation of non-device-specific M31 class register getter/setter methods for EH params.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/

#include "ntv2m31ehparam.h"

// Constructor
CNTV2m31EHParam::CNTV2m31EHParam(CNTV2Card* device) :
mpDevice (device)
{
}

// Destructor
CNTV2m31EHParam::~CNTV2m31EHParam()
{
}

// Apparently we write to memory and not actual registers so best to zero out all values before we begin setting bit fields
bool CNTV2m31EHParam::Clear()
{
	for (uint32_t j = M31_VIRTUAL_CH0; j <= M31_VIRTUAL_CH31; j++)
	{
		for (uint32_t i = kRegEHParamCC; i <= kRegEHParamReserved24; i++)
		{
			WriteM31EHParam((M31VirtualChannel)j,(EHParamRegisterIndex)i, 0);
		}
	}
	return true;
}

// Writes an EHParamsChannel struct to the hardware for a given channel
bool CNTV2m31EHParam::SetEHParamsChannel(EHParamsChannel eHParamsChannel, M31VirtualChannel channel)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Channel specific values need to be set here before we write then out to the hardware
	eHParamsChannel.eHSourceId = channel;

	SetCC(eHParamsChannel.eHCC, channel);
	SetSource(eHParamsChannel.eHSource, channel);
	SetSourceId(eHParamsChannel.eHSourceId, channel);
	SetHsEncodeMode(eHParamsChannel.eHHsEncodeMode, channel);
	SetInterlace(eHParamsChannel.eHInterlace, channel);
	SetChromaFormat(eHParamsChannel.eHChromaFormat, channel);
	SetBitDepth(eHParamsChannel.eHBitDepth, channel);
	
	SetHSizeVA(eHParamsChannel.eHHSizeVA, channel);
	SetVSizeVA(eHParamsChannel.eHVSizeVA, channel);

	SetHSizeEH(eHParamsChannel.eHHSizeEH, channel);
	SetVSizeEH(eHParamsChannel.eHVSizeEH, channel);

	SetProfile(eHParamsChannel.eHProfile, channel);
	SetLevel(eHParamsChannel.eHLevel, channel);
	SetTier(eHParamsChannel.eHTier, channel);

	SetAspectRatio(eHParamsChannel.eHAspectRatio, channel);

	SetSARWidth(eHParamsChannel.eHSARWidth, channel);
	SetSARHeight(eHParamsChannel.eHSARHeight, channel);

	SetEHPrivate01(eHParamsChannel.ehPrivate01, channel);
	SetEHPrivate02(eHParamsChannel.ehPrivate02, channel);
	SetEHPrivate03(eHParamsChannel.ehPrivate03, channel);
	SetEHPrivate04(eHParamsChannel.ehPrivate04, channel);
	SetEHPrivate05(eHParamsChannel.ehPrivate05, channel);

	SetFrameNumInGOP(eHParamsChannel.eHFrameNumInGOP, channel);
	SetGOPHierarchy(eHParamsChannel.eHGOPHierarchy, channel);
	SetUseTemporalID(eHParamsChannel.eHUseTemporalID, channel);
	SetEHPrivate06(eHParamsChannel.ehPrivate06, channel);
	SetEHPrivate07(eHParamsChannel.ehPrivate07, channel);

	SetPASL0B(eHParamsChannel.eHPASL0B, channel);
	SetIpPeriod(eHParamsChannel.eHIpPeriod, channel);
	SetAdaptiveGOP(eHParamsChannel.eHAdaptiveGOP, channel);
	SetClosedGOP(eHParamsChannel.eHClosedGOP, channel);
	SetIDRInterval(eHParamsChannel.eHIDRInterval, channel);

	SetRCMode(eHParamsChannel.eHRCMode, channel);
	SetDisableMinQpCtrl(eHParamsChannel.eHDisableMinQpCtrl, channel);
	SetEHPrivate08(eHParamsChannel.ehPrivate08, channel);
	SetAdaptiveQuant(eHParamsChannel.eHAdaptiveQuant, channel);

	SetBitRate(eHParamsChannel.eHBitRate, channel);
	SetVBRMaxBitRate(eHParamsChannel.eHVBRMaxBitRate, channel);
	SetVBRAveBitRate(eHParamsChannel.eHVBRAveBitRate, channel);
	SetEHPrivate09(eHParamsChannel.ehPrivate09, channel);
	SetEHPrivate10(eHParamsChannel.ehPrivate10, channel);
	SetNumUnitsInTickMax(eHParamsChannel.eHNumUnitsInTickMax, channel);
	SetTimeScale(eHParamsChannel.eHTimeScale, channel);
	SetNumUnitsInTick(eHParamsChannel.eHNumUnitsInTick, channel);
	SetCPBDelay(eHParamsChannel.eHCPBDelay, channel);

	SetEHPrivate11(eHParamsChannel.ehPrivate11, channel);
	SetMinCUSize(eHParamsChannel.eHMinCUSize, channel);
	SetMaxTUSize(eHParamsChannel.eHMaxTUSize, channel);
	SetMinTUSize(eHParamsChannel.eHMinTUSize, channel);

	SetTUDepthIntra(eHParamsChannel.eHTUDepthIntra, channel);
	SetTUDepthInter(eHParamsChannel.eHTUDepthInter, channel);

	SetEHPrivate12(eHParamsChannel.ehPrivate12, channel);
	SetEHPrivate13(eHParamsChannel.ehPrivate13, channel);
	SetIntraSmoothing(eHParamsChannel.eHIntraSmoothing, channel);
	SetEHPrivate14(eHParamsChannel.ehPrivate14, channel);
	SetMergeCand(eHParamsChannel.eHMergeCand, channel);

	SetEHPrivate15(eHParamsChannel.ehPrivate15, channel);
	SetEHPrivate16(eHParamsChannel.ehPrivate16, channel);

	SetEHPrivate17(eHParamsChannel.ehPrivate17, channel);
	SetEHPrivate18(eHParamsChannel.ehPrivate18, channel);
	SetEHPrivate19(eHParamsChannel.ehPrivate19, channel);
	SetEHPrivate20(eHParamsChannel.ehPrivate20, channel);
	SetEHPrivate21(eHParamsChannel.ehPrivate21, channel);
	SetEHPrivate22(eHParamsChannel.ehPrivate22, channel);

	SetEHPrivate23(eHParamsChannel.ehPrivate23, channel);
	SetEHPrivate24(eHParamsChannel.ehPrivate24, channel);

	SetDF(eHParamsChannel.eHDF, channel);

	SetEHPrivate25(eHParamsChannel.ehPrivate25, channel);

	SetPPSInsertion(eHParamsChannel.eHPPSInsertion, channel);
	SetEOS(eHParamsChannel.eHEOS, channel);
	SetMP4(eHParamsChannel.eHMP4, channel);

	SetBufCtrl(eHParamsChannel.eHBufCtrl, channel);
	SetVCLHRD(eHParamsChannel.eHVCLHRD, channel);
	SetEHPrivate26(eHParamsChannel.ehPrivate26, channel);
	SetVPSTiming(eHParamsChannel.eHVPSTiming, channel);

	SetOverscanInfo(eHParamsChannel.eHOverscanInfo, channel);
	SetBitRestriction(eHParamsChannel.eHBitRestriction, channel);

	SetVideoSignalType(eHParamsChannel.eHVideoSignalType, channel);
	SetVideoFormat(eHParamsChannel.eHVideoFormat, channel);
	SetVideoFullRange(eHParamsChannel.eHVideoFullRange, channel);
	SetColourDescription(eHParamsChannel.eHColourDescription, channel);
	SetColourPrimaries(eHParamsChannel.eHColourPrimaries, channel);
	SetTransferChar(eHParamsChannel.eHTransferChar, channel);

	SetMatrixCoef(eHParamsChannel.eHMatrixCoef, channel);
	SetChromaLocInfo(eHParamsChannel.eHChromaLocInfo, channel);
	SetSampleLocTop(eHParamsChannel.eHSampleLocTop, channel);
	SetSampleLocBot(eHParamsChannel.eHSampleLocBot, channel);

	SetCropLeft(eHParamsChannel.eHCropLeft, channel);
	SetCropRight(eHParamsChannel.eHCropRight, channel);

	SetCropTop(eHParamsChannel.eHCropTop, channel);
	SetCropBottom(eHParamsChannel.eHCropBottom, channel);

	SetEHPrivate27(eHParamsChannel.ehPrivate27, channel);
	SetEHPrivate28(eHParamsChannel.ehPrivate28, channel);

	SetUseTpIrap(eHParamsChannel.eUseTpIrap, channel);
	SetPicTiming(eHParamsChannel.eHPicTiming, channel);
	SetScanType(eHParamsChannel.eHScanType, channel);
	SetFramePacking(eHParamsChannel.eHFramePacking, channel);

	SetScanLeft(eHParamsChannel.eHScanLeft, channel);
	SetScanRight(eHParamsChannel.eHScanRight, channel);

	SetScanTop(eHParamsChannel.eHScanTop, channel);
	SetScanBottom(eHParamsChannel.eHScanBottom, channel);

	SetEHPrivate30(eHParamsChannel.ehPrivate30, channel);

	return true;
}

// Reads an EHParamsChannel struct from the hardware for a given channel
bool CNTV2m31EHParam::GetEHParamsChannel(EHParamsChannel* eHParamsChannel, M31VirtualChannel channel)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	GetCC(&eHParamsChannel->eHCC, channel);
	
	GetSource(&eHParamsChannel->eHSource, channel);
	GetSourceId(&eHParamsChannel->eHSourceId, channel);
	GetHsEncodeMode(&eHParamsChannel->eHHsEncodeMode, channel);
	GetInterlace(&eHParamsChannel->eHInterlace, channel);
	GetChromaFormat(&eHParamsChannel->eHChromaFormat, channel);
	GetBitDepth(&eHParamsChannel->eHBitDepth, channel);
	
	GetHSizeVA(&eHParamsChannel->eHHSizeVA, channel);
	GetVSizeVA(&eHParamsChannel->eHVSizeVA, channel);
	
	GetHSizeEH(&eHParamsChannel->eHHSizeEH, channel);
	GetVSizeEH(&eHParamsChannel->eHVSizeEH, channel);
	
	GetProfile(&eHParamsChannel->eHProfile, channel);
	GetLevel(&eHParamsChannel->eHLevel, channel);
	GetTier(&eHParamsChannel->eHTier, channel);
	
	GetAspectRatio(&eHParamsChannel->eHAspectRatio, channel);
	
	GetSARWidth(&eHParamsChannel->eHSARWidth, channel);
	GetSARHeight(&eHParamsChannel->eHSARHeight, channel);
	
	GetEHPrivate01(&eHParamsChannel->ehPrivate01, channel);
	GetEHPrivate02(&eHParamsChannel->ehPrivate02, channel);
	GetEHPrivate03(&eHParamsChannel->ehPrivate03, channel);
	GetEHPrivate04(&eHParamsChannel->ehPrivate04, channel);
	GetEHPrivate05(&eHParamsChannel->ehPrivate05, channel);
	
	GetFrameNumInGOP(&eHParamsChannel->eHFrameNumInGOP, channel);
	GetGOPHierarchy(&eHParamsChannel->eHGOPHierarchy, channel);
	GetUseTemporalID(&eHParamsChannel->eHUseTemporalID, channel);
	GetEHPrivate06(&eHParamsChannel->ehPrivate06, channel);
	GetEHPrivate07(&eHParamsChannel->ehPrivate07, channel);

	GetPASL0B(&eHParamsChannel->eHPASL0B, channel);
	GetIpPeriod(&eHParamsChannel->eHIpPeriod, channel);
	GetAdaptiveGOP(&eHParamsChannel->eHAdaptiveGOP, channel);
	GetClosedGOP(&eHParamsChannel->eHClosedGOP, channel);
	GetIDRInterval(&eHParamsChannel->eHIDRInterval, channel);
	
	GetRCMode(&eHParamsChannel->eHRCMode, channel);
	GetDisableMinQpCtrl(&eHParamsChannel->eHDisableMinQpCtrl, channel);
	GetEHPrivate08(&eHParamsChannel->ehPrivate08, channel);
	GetAdaptiveQuant(&eHParamsChannel->eHAdaptiveQuant, channel);

	GetBitRate(&eHParamsChannel->eHBitRate, channel);
	GetVBRMaxBitRate(&eHParamsChannel->eHVBRMaxBitRate, channel);
	GetVBRAveBitRate(&eHParamsChannel->eHVBRAveBitRate, channel);
	GetEHPrivate09(&eHParamsChannel->ehPrivate09, channel);
	GetEHPrivate10(&eHParamsChannel->ehPrivate10, channel);
	GetNumUnitsInTickMax(&eHParamsChannel->eHNumUnitsInTickMax, channel);
	GetTimeScale(&eHParamsChannel->eHTimeScale, channel);
	GetNumUnitsInTick(&eHParamsChannel->eHNumUnitsInTick, channel);
	GetCPBDelay(&eHParamsChannel->eHCPBDelay, channel);
	
	GetEHPrivate11(&eHParamsChannel->ehPrivate11, channel);
	GetMinCUSize(&eHParamsChannel->eHMinCUSize, channel);
	GetMaxTUSize(&eHParamsChannel->eHMaxTUSize, channel);
	GetMinTUSize(&eHParamsChannel->eHMinTUSize, channel);
	
	GetTUDepthIntra(&eHParamsChannel->eHTUDepthIntra, channel);
	GetTUDepthInter(&eHParamsChannel->eHTUDepthInter, channel);

	GetEHPrivate12(&eHParamsChannel->ehPrivate12, channel);
	GetEHPrivate13(&eHParamsChannel->ehPrivate13, channel);
	GetIntraSmoothing(&eHParamsChannel->eHIntraSmoothing, channel);
	GetEHPrivate14(&eHParamsChannel->ehPrivate14, channel);
	GetMergeCand(&eHParamsChannel->eHMergeCand, channel);

	GetEHPrivate15(&eHParamsChannel->ehPrivate15, channel);
	GetEHPrivate16(&eHParamsChannel->ehPrivate16, channel);
	
	GetEHPrivate17(&eHParamsChannel->ehPrivate17, channel);
	GetEHPrivate18(&eHParamsChannel->ehPrivate18, channel);
	GetEHPrivate19(&eHParamsChannel->ehPrivate19, channel);
	GetEHPrivate20(&eHParamsChannel->ehPrivate20, channel);
	GetEHPrivate21(&eHParamsChannel->ehPrivate21, channel);
	GetEHPrivate22(&eHParamsChannel->ehPrivate22, channel);

	GetEHPrivate23(&eHParamsChannel->ehPrivate23, channel);
	GetEHPrivate24(&eHParamsChannel->ehPrivate24, channel);
	
	GetDF(&eHParamsChannel->eHDF, channel);
	
	GetEHPrivate25(&eHParamsChannel->ehPrivate25, channel);
	
	GetPPSInsertion(&eHParamsChannel->eHPPSInsertion, channel);
	GetEOS(&eHParamsChannel->eHEOS, channel);
	GetMP4(&eHParamsChannel->eHMP4, channel);
	
	GetBufCtrl(&eHParamsChannel->eHBufCtrl, channel);
	GetVCLHRD(&eHParamsChannel->eHVCLHRD, channel);
	GetEHPrivate26(&eHParamsChannel->ehPrivate26, channel);
	GetVPSTiming(&eHParamsChannel->eHVPSTiming, channel);

	GetOverscanInfo(&eHParamsChannel->eHOverscanInfo, channel);
	GetBitRestriction(&eHParamsChannel->eHBitRestriction, channel);
	
	GetVideoSignalType(&eHParamsChannel->eHVideoSignalType, channel);
	GetVideoFormat(&eHParamsChannel->eHVideoFormat, channel);
	GetVideoFullRange(&eHParamsChannel->eHVideoFullRange, channel);
	GetColourDescription(&eHParamsChannel->eHColourDescription, channel);
	GetColourPrimaries(&eHParamsChannel->eHColourPrimaries, channel);
	GetTransferChar(&eHParamsChannel->eHTransferChar, channel);

	GetMatrixCoef(&eHParamsChannel->eHMatrixCoef, channel);
	GetChromaLocInfo(&eHParamsChannel->eHChromaLocInfo, channel);
	GetSampleLocTop(&eHParamsChannel->eHSampleLocTop, channel);
	GetSampleLocBot(&eHParamsChannel->eHSampleLocBot, channel);

	GetCropLeft(&eHParamsChannel->eHCropLeft, channel);
	GetCropRight(&eHParamsChannel->eHCropRight, channel);
	
	GetCropTop(&eHParamsChannel->eHCropTop, channel);
	GetCropBottom(&eHParamsChannel->eHCropBottom, channel);
	
	GetEHPrivate27(&eHParamsChannel->ehPrivate27, channel);
	GetEHPrivate28(&eHParamsChannel->ehPrivate28, channel);

	GetUseTpIrap(&eHParamsChannel->eUseTpIrap, channel);
	GetPicTiming(&eHParamsChannel->eHPicTiming, channel);
	GetScanType(&eHParamsChannel->eHScanType, channel);
	GetFramePacking(&eHParamsChannel->eHFramePacking, channel);
	
	GetScanLeft(&eHParamsChannel->eHScanLeft, channel);
	GetScanRight(&eHParamsChannel->eHScanRight, channel);
	
	GetScanTop(&eHParamsChannel->eHScanTop, channel);
	GetScanBottom(&eHParamsChannel->eHScanBottom, channel);
	
	GetEHPrivate30(&eHParamsChannel->ehPrivate30, channel);

	return true;
}

// Loads up an EHParamsChannel struct with all the default values for a given video preset
bool CNTV2m31EHParam::LoadEHParamsChannel(M31VideoPreset videoPreset, EHParamsChannel* eHParamsChannel)
{
	// Make sure we pass in a valid preset
	if (!IS_VALID_M31VideoPreset(videoPreset))
		return false;
	
	// Start with a clean slate
	memset(eHParamsChannel, 0, sizeof(EHParamsChannel));
	
	// Setup the common EH params
	eHParamsChannel->eHSource					= M31_SourceVA;
	eHParamsChannel->eHSourceId					= M31_VIRTUAL_CH0;
	eHParamsChannel->eHHsEncodeMode				= M31_EncodeModeDefault;

	eHParamsChannel->eHCPBDelay					= 0;
	eHParamsChannel->ehPrivate11				= 0;
	eHParamsChannel->eHMinCUSize				= 0;
	eHParamsChannel->eHMaxTUSize				= 0;
	eHParamsChannel->eHMinTUSize				= 0;
	eHParamsChannel->ehPrivate01				= 0;
	eHParamsChannel->ehPrivate02				= 0;
	eHParamsChannel->ehPrivate03				= 1;
	eHParamsChannel->ehPrivate04				= 1;
	eHParamsChannel->ehPrivate05				= 1;
	eHParamsChannel->ehPrivate09				= 0;
	eHParamsChannel->ehPrivate10				= 0;
	eHParamsChannel->ehPrivate06				= 1;
	eHParamsChannel->ehPrivate07				= 1;
	
	eHParamsChannel->eHAdaptiveGOP				= 0;
	eHParamsChannel->eHClosedGOP				= 0;
	eHParamsChannel->eHIDRInterval				= 100;
	
	eHParamsChannel->eHRCMode					= 0;
	eHParamsChannel->eHDisableMinQpCtrl			= 0;
	eHParamsChannel->ehPrivate08				= 0;
	eHParamsChannel->eHAdaptiveQuant			= 0;
	
	eHParamsChannel->eHTUDepthIntra				= 4;
	eHParamsChannel->eHTUDepthInter				= 4;
	eHParamsChannel->ehPrivate12				= 0;
	eHParamsChannel->ehPrivate13				= 0;
	eHParamsChannel->eHIntraSmoothing			= 0;
	eHParamsChannel->ehPrivate14				= 0;
	eHParamsChannel->eHMergeCand				= 0;
	eHParamsChannel->ehPrivate15				= 0;
	eHParamsChannel->ehPrivate16				= 0;
	eHParamsChannel->ehPrivate17				= 0;
	eHParamsChannel->ehPrivate18				= 32;
	eHParamsChannel->ehPrivate19				= 32;
	eHParamsChannel->ehPrivate20				= 0;
	eHParamsChannel->ehPrivate21				= 0;
	eHParamsChannel->ehPrivate22				= 0;
	eHParamsChannel->ehPrivate23				= 0;
	eHParamsChannel->ehPrivate24				= 0;
	eHParamsChannel->eHDF						= 1;
	eHParamsChannel->ehPrivate25				= 0;
	eHParamsChannel->eHPPSInsertion				= 0;
	eHParamsChannel->eHEOS						= 0;
	eHParamsChannel->eHMP4						= 0;
	eHParamsChannel->eHBufCtrl					= 1;
	eHParamsChannel->eHVCLHRD					= 1;
	eHParamsChannel->ehPrivate26				= 0;
	eHParamsChannel->eHVPSTiming				= 1;
	eHParamsChannel->eHOverscanInfo				= 0;
	eHParamsChannel->eHBitRestriction			= 0;
	eHParamsChannel->eHCropLeft					= 0;
	eHParamsChannel->eHCropRight				= 0;

	eHParamsChannel->ehPrivate27				= 0;
	eHParamsChannel->ehPrivate28				= 0;
	eHParamsChannel->eUseTpIrap					= 0;
	eHParamsChannel->eHPicTiming				= 1;
	eHParamsChannel->eHScanType					= 1;
	eHParamsChannel->eHFramePacking				= 0;
	eHParamsChannel->eHScanLeft					= 0;
	eHParamsChannel->eHScanRight				= 0;
	eHParamsChannel->eHScanTop					= 0;
	eHParamsChannel->eHScanBottom				= 0;
	eHParamsChannel->ehPrivate30				= 0;
	
	eHParamsChannel->eHVideoSignalType			= 0;
	eHParamsChannel->eHVideoFormat				= 0;
	eHParamsChannel->eHVideoFullRange			= 0;
	eHParamsChannel->eHColourDescription		= 0;
	eHParamsChannel->eHColourPrimaries			= 0;
	eHParamsChannel->eHTransferChar				= 0;
	eHParamsChannel->eHMatrixCoef				= 0;
	eHParamsChannel->eHChromaLocInfo			= 0;
	eHParamsChannel->eHSampleLocTop				= 0;
	eHParamsChannel->eHSampleLocBot				= 0;
	
	switch (videoPreset)
	{
		case M31_FILE_720X480_420_8_5994i:
		case M31_VIF_720X480_420_8_5994i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 480;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 480;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 5;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_720X480_420_8_5994p:
		case M31_VIF_720X480_420_8_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 480;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 480;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 5;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_720X480_420_8_60i:
		case M31_VIF_720X480_420_8_60i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 480;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 480;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 50;
			eHParamsChannel->eHTier						= M31_MainTier;
			eHParamsChannel->eHAspectRatio				= 5;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_720X480_420_8_60p:
		case M31_VIF_720X480_420_8_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 480;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 480;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 50;
			eHParamsChannel->eHTier						= M31_MainTier;
			eHParamsChannel->eHAspectRatio				= 5;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
		  
		case M31_FILE_720X480_422_10_5994i:
		case M31_VIF_720X480_422_10_5994i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 480;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 480;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 5;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_720X480_422_10_5994p:
		case M31_VIF_720X480_422_10_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 480;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 480;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 5;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_720X480_422_10_60i:
		case M31_VIF_720X480_422_10_60i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 480;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 480;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 5;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_720X480_422_10_60p:
		case M31_VIF_720X480_422_10_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 480;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 480;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 5;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_720X576_420_8_50i:
		case M31_VIF_720X576_420_8_50i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 576;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 576;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 4;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
		   
		case M31_FILE_720X576_420_8_50p:
		case M31_VIF_720X576_420_8_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 576;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 576;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 4;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_720X576_422_10_50i:
		case M31_VIF_720X576_422_10_50i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 576;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 576;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 4;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
	   case M31_FILE_720X576_422_10_50p:
		case M31_VIF_720X576_422_10_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 720;
			eHParamsChannel->eHVSizeVA					= 576;
			eHParamsChannel->eHHSizeEH					= 720;
			eHParamsChannel->eHVSizeEH					= 576;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 4;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 2000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
  
		case M31_FILE_1280X720_420_8_2398p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 24000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_1280X720_420_8_24p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 24;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_1280X720_420_8_25p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 25;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_1280X720_420_8_2997p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 30000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_1280X720_420_8_30p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 30;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_1280X720_420_8_50p:
		case M31_VIF_1280X720_420_8_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_1280X720_420_8_5994p:
		case M31_VIF_1280X720_420_8_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_1280X720_420_8_60p:
		case M31_VIF_1280X720_420_8_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_1280X720_422_10_2398p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 24000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_1280X720_422_10_24p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 24;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_1280X720_422_10_25p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 25;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_1280X720_422_10_2997p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 30000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_1280X720_422_10_30p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 30;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_1280X720_422_10_50p:
		case M31_VIF_1280X720_422_10_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_1280X720_422_10_5994p:
		case M31_VIF_1280X720_422_10_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_1280X720_422_10_60p:
		case M31_VIF_1280X720_422_10_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1280;
			eHParamsChannel->eHVSizeVA					= 720;
			eHParamsChannel->eHHSizeEH					= 1280;
			eHParamsChannel->eHVSizeEH					= 720;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 6000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_1920X1080_420_8_50i:
		case M31_VIF_1920X1080_420_8_50i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 32;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 4;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_1920X1080_420_8_2398p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 24000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_1920X1080_420_8_24p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 24;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_1920X1080_420_8_25p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 25;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_1920X1080_420_8_2997p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 30000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_1920X1080_420_8_30p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 30;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_1920X1080_420_8_50p:
		case M31_VIF_1920X1080_420_8_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_1920X1080_420_8_5994i:
		case M31_VIF_1920X1080_420_8_5994i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 32;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 4;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_1920X1080_420_8_5994p:
		case M31_VIF_1920X1080_420_8_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_1920X1080_420_8_60i:
		case M31_VIF_1920X1080_420_8_60i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 32;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 4;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_1920X1080_420_8_60p:
		case M31_VIF_1920X1080_420_8_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
		  
		case M31_FILE_1920X1080_422_10_50i:
		case M31_VIF_1920X1080_420_10_50i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 32;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 4;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_1920X1080_422_10_2398p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 24000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_1920X1080_422_10_24p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 24;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_1920X1080_422_10_25p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 25;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_1920X1080_422_10_2997p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 3000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_1920X1080_422_10_30p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 30;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_1920X1080_422_10_50p:
		case M31_VIF_1920X1080_420_10_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_1920X1080_422_10_5994i:
		case M31_VIF_1920X1080_422_10_5994i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 32;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 4;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_1920X1080_422_10_5994p:
		case M31_VIF_1920X1080_422_10_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_1920X1080_422_10_60i:
		case M31_VIF_1920X1080_422_10_60i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 32;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 4;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_1920X1080_422_10_60p:
		case M31_VIF_1920X1080_422_10_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
  
		case M31_FILE_2048X1080_420_8_2398p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 24000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_2048X1080_420_8_24p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 24;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_2048X1080_420_8_25p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 25;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_2048X1080_420_8_2997p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 30000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_2048X1080_420_8_30p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 30;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_2048X1080_420_8_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_2048X1080_420_8_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_2048X1080_420_8_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
		  
		case M31_FILE_2048X1080_422_10_2398p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 24000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_2048X1080_422_10_24p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 24;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;

		case M31_FILE_2048X1080_422_10_25p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 25;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_2048X1080_422_10_2997p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 3000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_2048X1080_422_10_30p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 30;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_2048X1080_422_10_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_2048X1080_422_10_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_2048X1080_422_10_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 2048;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 2048;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
  
		case M31_VIF_1920X1080_420_10_5994i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 32;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 4;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
	   case M31_VIF_1920X1080_420_10_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain10;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_VIF_1920X1080_420_10_60i:
			eHParamsChannel->eHInterlace				= M31_Interlace;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 32;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 4;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
	   case M31_VIF_1920X1080_420_10_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 1920;
			eHParamsChannel->eHVSizeVA					= 1080;
			eHParamsChannel->eHHSizeEH					= 1920;
			eHParamsChannel->eHVSizeEH					= 1088;
			eHParamsChannel->eHProfile					= M31_ProfileMain10;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 8000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 8;
			break;
			
		case M31_FILE_3840X2160_420_8_2398p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 16000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 24000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_3840X2160_420_8_24p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 16000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 24;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_3840X2160_420_8_25p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 16000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 25;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_420_8_2997p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 16000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 30000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_420_8_30p:
		case M31_VIF_3840X2160_420_8_30p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 16000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 30;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_420_8_50p:
		case M31_VIF_3840X2160_420_8_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_420_8_5994p:
		case M31_VIF_3840X2160_420_8_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_420_8_60p:
		case M31_VIF_3840X2160_420_8_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
		  
		case M31_FILE_3840X2160_420_10_50p:
		case M31_VIF_3840X2160_420_10_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain10;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_3840X2160_420_10_5994p:
		case M31_VIF_3840X2160_420_10_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain10;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_3840X2160_420_10_60p:
		case M31_VIF_3840X2160_420_10_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain10;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_8_2398p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 24000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_8_24p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 24;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_8_25p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 25;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_8_2997p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 30000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_8_30p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 30;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_8_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_3840X2160_422_8_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_8_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth8;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_10_2398p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 24000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_10_24p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 24;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_10_25p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 25;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_3840X2160_422_10_2997p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 30000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_3840X2160_422_10_30p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 30;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_3840X2160_422_10_50p:
		case M31_VIF_3840X2160_422_10_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_10_5994p:
		case M31_VIF_3840X2160_422_10_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_3840X2160_422_10_60p:
		case M31_VIF_3840X2160_422_10_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_VIF_3840X2160_422_10_30p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 3840;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 3840;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 16000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 30;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_4096X2160_420_10_5994p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 4096;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 4096;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain10;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_4096X2160_420_10_60p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma420;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 4096;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 4096;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_ProfileMain10;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_4096X2160_422_10_50p:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 4096;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 4096;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 64;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 50;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_FILE_4096X2160_422_10_5994p_IF:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 4096;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 4096;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 0;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2002;
			eHParamsChannel->eHTimeScale				= 60000;
			eHParamsChannel->eHNumUnitsInTick			= 1001;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;
			
		case M31_FILE_4096X2160_422_10_60p_IF:
			eHParamsChannel->eHInterlace				= M31_Progressive;
			eHParamsChannel->eHChromaFormat				= M31_Chroma422;
			eHParamsChannel->eHBitDepth					= M31_BitDepth10;
			eHParamsChannel->eHHSizeVA					= 4096;
			eHParamsChannel->eHVSizeVA					= 2160;
			eHParamsChannel->eHHSizeEH					= 4096;
			eHParamsChannel->eHVSizeEH					= 2160;
			eHParamsChannel->eHProfile					= M31_Profile422;
			eHParamsChannel->eHLevel					= 0;
			eHParamsChannel->eHTier						= M31_HighTier;
			eHParamsChannel->eHAspectRatio				= 1;
			eHParamsChannel->eHSARWidth					= 0;
			eHParamsChannel->eHSARHeight				= 0;
			eHParamsChannel->eHFrameNumInGOP			= 0;
			eHParamsChannel->eHGOPHierarchy				= 1;
			eHParamsChannel->eHUseTemporalID			= 1;
			eHParamsChannel->eHPASL0B					= 0;
			eHParamsChannel->eHIpPeriod					= 8;
			eHParamsChannel->eHBitRate					= 32000;
			eHParamsChannel->eHVBRMaxBitRate			= 0;
			eHParamsChannel->eHVBRAveBitRate			= 0;
			eHParamsChannel->eHNumUnitsInTickMax		= 2;
			eHParamsChannel->eHTimeScale				= 60;
			eHParamsChannel->eHNumUnitsInTick			= 1;
			eHParamsChannel->eHCropTop					= 0;
			eHParamsChannel->eHCropBottom				= 0;
			break;

		case M31_NUMVIDEOPRESETS:
			break;
	}
	return true;
}

// Public setters/getters

bool CNTV2m31EHParam::SetCC(M31CC cc, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamCC, cc,
						   kRegMaskEHParamCC, kRegShiftEHParamCC);
}

bool CNTV2m31EHParam::GetCC(M31CC* cc, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamCC, reinterpret_cast <uint32_t*> (cc),
						  kRegMaskEHParamCC, kRegShiftEHParamCC);
}

bool CNTV2m31EHParam::SetSource(M31SourceVA source, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSource, source,
						   kRegMaskEHParamSource, kRegShiftEHParamSource);
}

bool CNTV2m31EHParam::GetSource(M31SourceVA* source, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSource, reinterpret_cast <uint32_t*> (source),
						  kRegMaskEHParamSource, kRegShiftEHParamSource);
}

bool CNTV2m31EHParam::SetSourceId(M31VirtualChannel sourceId, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSource, sourceId,
						   kRegMaskEHParamSourceId, kRegShiftEHParamSourceId);
}

bool CNTV2m31EHParam::GetSourceId(M31VirtualChannel* sourceId, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSource, reinterpret_cast <uint32_t*> (sourceId),
						  kRegMaskEHParamSourceId, kRegShiftEHParamSourceId);
}

bool CNTV2m31EHParam::SetHsEncodeMode(M31EncodeMode hsEncodeMode, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSource, hsEncodeMode,
						   kRegMaskEHParamHsEncodeMode, kRegShiftEHParamHsEncodeMode);
}

bool CNTV2m31EHParam::GetHsEncodeMode(M31EncodeMode* hsEncodeMode, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSource, reinterpret_cast <uint32_t*> (hsEncodeMode),
						  kRegMaskEHParamHsEncodeMode, kRegShiftEHParamHsEncodeMode);
}

bool CNTV2m31EHParam::SetInterlace(M31ScanMode interlace, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSource, interlace,
						   kRegMaskEHParamInterlace, kRegShiftEHParamInterlace);
}

bool CNTV2m31EHParam::GetInterlace(M31ScanMode* interlace, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSource, reinterpret_cast <uint32_t*> (interlace),
						  kRegMaskEHParamInterlace, kRegShiftEHParamInterlace);
}

bool CNTV2m31EHParam::SetChromaFormat(M31ChromaFormat chromaFormat, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSource, chromaFormat,
						   kRegMaskEHParamChromaFormat, kRegShiftEHParamChromaFormat);
}

bool CNTV2m31EHParam::GetChromaFormat(M31ChromaFormat* chromaFormat, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSource, reinterpret_cast <uint32_t*> (chromaFormat),
						  kRegMaskEHParamChromaFormat, kRegShiftEHParamChromaFormat);
}

bool CNTV2m31EHParam::SetBitDepth(M31BitDepth bitDepth, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSource, bitDepth,
						   kRegMaskEHParamBitDepth, kRegShiftEHParamBitDepth);
}

bool CNTV2m31EHParam::GetBitDepth(M31BitDepth* bitDepth, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSource, reinterpret_cast <uint32_t*> (bitDepth),
						  kRegMaskEHParamBitDepth, kRegShiftEHParamBitDepth);
}

bool CNTV2m31EHParam::SetHSizeVA(uint32_t hSizeVA, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSizeVA, hSizeVA,
						   kRegMaskEHParamHSizeVA, kRegShiftEHParamHSizeVA);
}

bool CNTV2m31EHParam::GetHSizeVA(uint32_t* hSizeVA, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSizeVA, reinterpret_cast <uint32_t*> (hSizeVA),
						  kRegMaskEHParamHSizeVA, kRegShiftEHParamHSizeVA);
}

bool CNTV2m31EHParam::SetVSizeVA(uint32_t vSizeVA, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSizeVA, vSizeVA,
						   kRegMaskEHParamVSizeVA, kRegShiftEHParamVSizeVA);
}

bool CNTV2m31EHParam::GetVSizeVA(uint32_t* vSizeVA, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSizeVA, reinterpret_cast <uint32_t*> (vSizeVA),
						  kRegMaskEHParamVSizeVA, kRegShiftEHParamVSizeVA);
}

bool CNTV2m31EHParam::SetHSizeEH(uint32_t hSizeEH, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSizeEH, hSizeEH,
						   kRegMaskEHParamHSizeEH, kRegShiftEHParamHSizeEH);
}

bool CNTV2m31EHParam::GetHSizeEH(uint32_t* hSizeEH, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSizeEH, reinterpret_cast <uint32_t*> (hSizeEH),
						  kRegMaskEHParamHSizeEH, kRegShiftEHParamHSizeEH);
}

bool CNTV2m31EHParam::SetVSizeEH(uint32_t vSizeEH, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSizeEH, vSizeEH,
						   kRegMaskEHParamVSizeEH, kRegShiftEHParamVSizeEH);
}

bool CNTV2m31EHParam::GetVSizeEH(uint32_t* vSizeEH, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSizeEH, reinterpret_cast <uint32_t*> (vSizeEH),
						  kRegMaskEHParamVSizeEH, kRegShiftEHParamVSizeEH);
}

bool CNTV2m31EHParam::SetProfile(M31Profile profile, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamProfile, profile,
						   kRegMaskEHParamProfile, kRegShiftEHParamProfile);
}

bool CNTV2m31EHParam::GetProfile(M31Profile* profile, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamProfile, reinterpret_cast <uint32_t*> (profile),
						  kRegMaskEHParamProfile, kRegShiftEHParamProfile);
}

bool CNTV2m31EHParam::SetLevel(uint32_t level, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamProfile, level,
						   kRegMaskEHParamLevel, kRegShiftEHParamLevel);
}

bool CNTV2m31EHParam::GetLevel(uint32_t* level, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamProfile, reinterpret_cast <uint32_t*> (level),
						  kRegMaskEHParamLevel, kRegShiftEHParamLevel);
}

bool CNTV2m31EHParam::SetTier(M31Tier tier, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamProfile, tier,
						   kRegMaskEHParamTier, kRegShiftEHParamTier);
}

bool CNTV2m31EHParam::GetTier(M31Tier* tier, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamProfile, reinterpret_cast <uint32_t*> (tier),
						  kRegMaskEHParamTier, kRegShiftEHParamTier);
}

bool CNTV2m31EHParam::SetAspectRatio(uint32_t aspectRatio, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamAspectRatio, aspectRatio,
						   kRegMaskEHParamAspectRatio, kRegShiftEHParamAspectRatio);
}

bool CNTV2m31EHParam::GetAspectRatio(uint32_t* aspectRatio, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamAspectRatio, reinterpret_cast <uint32_t*> (aspectRatio),
						  kRegMaskEHParamAspectRatio, kRegShiftEHParamAspectRatio);
}

bool CNTV2m31EHParam::SetSARWidth(uint32_t sarWidth, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSAR, sarWidth,
						   kRegMaskEHParamSARWidth, kRegShiftEHParamSARWidth);
}

bool CNTV2m31EHParam::GetSARWidth(uint32_t* sarWidth, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSAR, reinterpret_cast <uint32_t*> (sarWidth),
						  kRegMaskEHParamSARWidth, kRegShiftEHParamSARWidth);
}

bool CNTV2m31EHParam::SetSARHeight(uint32_t sarHeight, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSAR, sarHeight,
						   kRegMaskEHParamSARHeight, kRegShiftEHParamSARHeight);
}

bool CNTV2m31EHParam::GetSARHeight(uint32_t* sarHeight, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSAR, reinterpret_cast <uint32_t*> (sarHeight),
						  kRegMaskEHParamSARHeight, kRegShiftEHParamSARHeight);
}

bool CNTV2m31EHParam::SetFrameNumInGOP(uint32_t frameNumInGOP, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGop1, frameNumInGOP,
						   kRegMaskEHParamFrameNumInGOP, kRegShiftEHParamFrameNumInGOP);
}

bool CNTV2m31EHParam::GetFrameNumInGOP(uint32_t* frameNumInGOP, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGop1, reinterpret_cast <uint32_t*> (frameNumInGOP),
						  kRegMaskEHParamFrameNumInGOP, kRegShiftEHParamFrameNumInGOP);
}

bool CNTV2m31EHParam::SetGOPHierarchy(uint32_t gopHierarchy, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGop1, gopHierarchy,
						   kRegMaskEHParamGOPHierarchy, kRegShiftEHParamGOPHierarchy);
}

bool CNTV2m31EHParam::GetGOPHierarchy(uint32_t* gopHierarchy, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGop1, reinterpret_cast <uint32_t*> (gopHierarchy),
						  kRegMaskEHParamGOPHierarchy, kRegShiftEHParamGOPHierarchy);
}

bool CNTV2m31EHParam::SetUseTemporalID(uint32_t useTemporalID, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGop1, useTemporalID,
						   kRegMaskEHParamUseTemporalID, kRegShiftEHParamUseTemporalID);
}

bool CNTV2m31EHParam::GetUseTemporalID(uint32_t* useTemporalID, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGop1, reinterpret_cast <uint32_t*> (useTemporalID),
						  kRegMaskEHParamUseTemporalID, kRegShiftEHParamUseTemporalID);
}

bool CNTV2m31EHParam::SetPASL0B(uint32_t pASL0B, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGop2, pASL0B,
						   kRegMaskEHParamPASL0B, kRegShiftEHParamPASL0B);
}

bool CNTV2m31EHParam::GetPASL0B(uint32_t* pASL0B, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGop2, reinterpret_cast <uint32_t*> (pASL0B),
						  kRegMaskEHParamPASL0B, kRegShiftEHParamPASL0B);
}

bool CNTV2m31EHParam::SetIpPeriod(uint32_t ipPeriod, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGop2, ipPeriod,
						   kRegMaskEHParamIpPeriod, kRegShiftEHParamIpPeriod);
}

bool CNTV2m31EHParam::GetIpPeriod(uint32_t* ipPeriod, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGop2, reinterpret_cast <uint32_t*> (ipPeriod),
						  kRegMaskEHParamIpPeriod, kRegShiftEHParamIpPeriod);
}

bool CNTV2m31EHParam::SetAdaptiveGOP(uint32_t adaptiveGOP, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGop2, adaptiveGOP,
						   kRegMaskEHParamAdaptiveGOP, kRegShiftEHParamAdaptiveGOP);
}

bool CNTV2m31EHParam::GetAdaptiveGOP(uint32_t* adaptiveGOP, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGop2, reinterpret_cast <uint32_t*> (adaptiveGOP),
						  kRegMaskEHParamAdaptiveGOP, kRegShiftEHParamAdaptiveGOP);
}

bool CNTV2m31EHParam::SetClosedGOP(uint32_t closedGOP, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGop2, closedGOP,
						   kRegMaskEHParamClosedGOP, kRegShiftEHParamClosedGOP);
}

bool CNTV2m31EHParam::GetClosedGOP(uint32_t* closedGOP, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGop2, reinterpret_cast <uint32_t*> (closedGOP),
						  kRegMaskEHParamClosedGOP, kRegShiftEHParamClosedGOP);
}

bool CNTV2m31EHParam::SetIDRInterval(uint32_t idrInterval, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGop2, idrInterval,
						   kRegMaskEHParamIDRInterval, kRegShiftEHParamIDRInterval);
}

bool CNTV2m31EHParam::GetIDRInterval(uint32_t* idrInterval, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGop2, reinterpret_cast <uint32_t*> (idrInterval),
						  kRegMaskEHParamIDRInterval, kRegShiftEHParamIDRInterval);
}

bool CNTV2m31EHParam::SetRCMode(uint32_t rCMode, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamRCMode, rCMode,
						   kRegMaskEHParamRCMode, kRegShiftEHParamRCMode);
}

bool CNTV2m31EHParam::GetRCMode(uint32_t* rCMode, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamRCMode, reinterpret_cast <uint32_t*> (rCMode),
						  kRegMaskEHParamRCMode, kRegShiftEHParamRCMode);
}

bool CNTV2m31EHParam::SetDisableMinQpCtrl(uint32_t disableMinQpCtrl, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamRCMode, disableMinQpCtrl,
						   kRegMaskEHParamMinQpCtrl, kRegShiftEHParamMinQpCtrl);
}

bool CNTV2m31EHParam::GetDisableMinQpCtrl(uint32_t* disableMinQpCtrl, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamRCMode, reinterpret_cast <uint32_t*> (disableMinQpCtrl),
						  kRegMaskEHParamMinQpCtrl, kRegShiftEHParamMinQpCtrl);
}

bool CNTV2m31EHParam::SetAdaptiveQuant(uint32_t adaptiveQuant, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamRCMode, adaptiveQuant,
						   kRegMaskEHParamAdaptiveQuant, kRegShiftEHParamAdaptiveQuant);
}

bool CNTV2m31EHParam::GetAdaptiveQuant(uint32_t* adaptiveQuant, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamRCMode, reinterpret_cast <uint32_t*> (adaptiveQuant),
						  kRegMaskEHParamAdaptiveQuant, kRegShiftEHParamAdaptiveQuant);
}

bool CNTV2m31EHParam::SetBitRate(uint32_t bitRate, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamBitRate, bitRate,
						   kRegMaskEHParamBitRate, kRegShiftEHParamBitRate);
}

bool CNTV2m31EHParam::GetBitRate(uint32_t* bitRate, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamBitRate, reinterpret_cast <uint32_t*> (bitRate),
						  kRegMaskEHParamBitRate, kRegShiftEHParamBitRate);
}

bool CNTV2m31EHParam::SetVBRMaxBitRate(uint32_t vbrMaxBitRate, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamVBRMaxBitRate, vbrMaxBitRate,
						   kRegMaskEHParamVBRMaxBitRate, kRegShiftEHParamVBRMaxBitRate);
}

bool CNTV2m31EHParam::GetVBRMaxBitRate(uint32_t* vbrMaxBitRate, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamVBRMaxBitRate, reinterpret_cast <uint32_t*> (vbrMaxBitRate),
						  kRegMaskEHParamVBRMaxBitRate, kRegShiftEHParamVBRMaxBitRate);
}

bool CNTV2m31EHParam::SetVBRAveBitRate(uint32_t vbrAveBitRate, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamVBRAveBitRate, vbrAveBitRate,
						   kRegMaskEHParamVBRAveBitRate, kRegShiftEHParamVBRAveBitRate);
}

bool CNTV2m31EHParam::GetVBRAveBitRate(uint32_t* vbrAveBitRate, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamVBRAveBitRate, reinterpret_cast <uint32_t*> (vbrAveBitRate),
						  kRegMaskEHParamVBRAveBitRate, kRegShiftEHParamVBRAveBitRate);
}

bool CNTV2m31EHParam::SetNumUnitsInTickMax(uint32_t numUnitsInTickMax, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamNumUnitsInTickMax, numUnitsInTickMax,
						   kRegMaskEHParamNumUnitsInTickMax, kRegShiftEHParamNumUnitsInTickMax);
}

bool CNTV2m31EHParam::GetNumUnitsInTickMax(uint32_t* numUnitsInTickMax, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamNumUnitsInTickMax, reinterpret_cast <uint32_t*> (numUnitsInTickMax),
						  kRegMaskEHParamNumUnitsInTickMax, kRegShiftEHParamNumUnitsInTickMax);
}

bool CNTV2m31EHParam::SetTimeScale(uint32_t timeScale, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamTimeScale, timeScale,
						   kRegMaskEHParamTimeScale, kRegShiftEHParamTimeScale);
}

bool CNTV2m31EHParam::GetTimeScale(uint32_t* timeScale, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamTimeScale, reinterpret_cast <uint32_t*> (timeScale),
						  kRegMaskEHParamTimeScale, kRegShiftEHParamTimeScale);
}

bool CNTV2m31EHParam::SetNumUnitsInTick(uint32_t numUnitsInTick, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamNumUnitsInTick, numUnitsInTick,
						   kRegMaskEHParamNumUnitsInTick, kRegShiftEHParamNumUnitsInTick);
}

bool CNTV2m31EHParam::GetNumUnitsInTick(uint32_t* numUnitsInTick, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamNumUnitsInTick, reinterpret_cast <uint32_t*> (numUnitsInTick),
						  kRegMaskEHParamNumUnitsInTick, kRegShiftEHParamNumUnitsInTick);
}

bool CNTV2m31EHParam::SetCPBDelay(uint32_t cpbDelay, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamCPBDelay, cpbDelay,
						   kRegMaskEHParamCPBDelay, kRegShiftEHParamCPBDelay);
}

bool CNTV2m31EHParam::GetCPBDelay(uint32_t* cpbDelay, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamCPBDelay, reinterpret_cast <uint32_t*> (cpbDelay),
						  kRegMaskEHParamCPBDelay, kRegShiftEHParamCPBDelay);
}

bool CNTV2m31EHParam::SetMinCUSize(uint32_t minCUSize, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamCUTUSize, minCUSize,
						   kRegMaskEHParamMinCUSize, kRegShiftEHParamMinCUSize);
}

bool CNTV2m31EHParam::GetMinCUSize(uint32_t* minCUSize, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamCUTUSize, reinterpret_cast <uint32_t*> (minCUSize),
						  kRegMaskEHParamMinCUSize, kRegShiftEHParamMinCUSize);
}

bool CNTV2m31EHParam::SetMaxTUSize(uint32_t maxTUSize, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamCUTUSize, maxTUSize,
						   kRegMaskEHParamMaxTUSize, kRegShiftEHParamMaxTUSize);
}

bool CNTV2m31EHParam::GetMaxTUSize(uint32_t* maxTUSize, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamCUTUSize, reinterpret_cast <uint32_t*> (maxTUSize),
						  kRegMaskEHParamMaxTUSize, kRegShiftEHParamMaxTUSize);
}

bool CNTV2m31EHParam::SetMinTUSize(uint32_t minTUSize, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamCUTUSize, minTUSize,
						   kRegMaskEHParamMinTUSize, kRegShiftEHParamMinTUSize);
}

bool CNTV2m31EHParam::GetMinTUSize(uint32_t* minTUSize, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamCUTUSize, reinterpret_cast <uint32_t*> (minTUSize),
						  kRegMaskEHParamMinTUSize, kRegShiftEHParamMinTUSize);
}

bool CNTV2m31EHParam::SetTUDepthIntra(uint32_t tuDepthIntra, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamTUDepth, tuDepthIntra,
						   kRegMaskEHParamTUDepthIntra, kRegShiftEHParamTUDepthIntra);
}

bool CNTV2m31EHParam::GetTUDepthIntra(uint32_t* tuDepthIntra, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamTUDepth, reinterpret_cast <uint32_t*> (tuDepthIntra),
						  kRegMaskEHParamTUDepthIntra, kRegShiftEHParamTUDepthIntra);
}

bool CNTV2m31EHParam::SetTUDepthInter(uint32_t tuDepthInter, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamTUDepth, tuDepthInter,
						   kRegMaskEHParamTUDepthInter, kRegShiftEHParamTUDepthInter);
}

bool CNTV2m31EHParam::GetTUDepthInter(uint32_t* tuDepthInter, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamTUDepth, reinterpret_cast <uint32_t*> (tuDepthInter),
						  kRegMaskEHParamTUDepthInter, kRegShiftEHParamTUDepthInter);
}

bool CNTV2m31EHParam::SetIntraSmoothing(uint32_t intraSmoothing, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamTSkip, intraSmoothing,
						   kRegMaskEHParamIntraSmoothing, kRegShiftEHParamIntraSmoothing);
}

bool CNTV2m31EHParam::GetIntraSmoothing(uint32_t* intraSmoothing, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamTSkip, reinterpret_cast <uint32_t*> (intraSmoothing),
						  kRegMaskEHParamIntraSmoothing, kRegShiftEHParamIntraSmoothing);
}

bool CNTV2m31EHParam::SetMergeCand(uint32_t mergeCand, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamTSkip, mergeCand,
						   kRegMaskEHParamMergeCand, kRegShiftEHParamMergeCand);
}

bool CNTV2m31EHParam::GetMergeCand(uint32_t* mergeCand, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamTSkip, reinterpret_cast <uint32_t*> (mergeCand),
						  kRegMaskEHParamMergeCand, kRegShiftEHParamMergeCand);
}

bool CNTV2m31EHParam::SetDF(uint32_t df, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamDF, df,
						   kRegMaskEHParamDF, kRegShiftEHParamDF);
}

bool CNTV2m31EHParam::GetDF(uint32_t* df, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamDF, reinterpret_cast <uint32_t*> (df),
						  kRegMaskEHParamDF, kRegShiftEHParamDF);
}

bool CNTV2m31EHParam::SetPPSInsertion(uint32_t ppsInsertion, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPPS, ppsInsertion,
						   kRegMaskEHParamPPSInsertion, kRegShiftEHParamPPSInsertion);
}

bool CNTV2m31EHParam::GetPPSInsertion(uint32_t* ppsInsertion, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPPS, reinterpret_cast <uint32_t*> (ppsInsertion),
						  kRegMaskEHParamPPSInsertion, kRegShiftEHParamPPSInsertion);
}

bool CNTV2m31EHParam::SetEOS(uint32_t eos, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPPS, eos,
						   kRegMaskEHParamEOS, kRegShiftEHParamEOS);
}

bool CNTV2m31EHParam::GetEOS(uint32_t* eos, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPPS, reinterpret_cast <uint32_t*> (eos),
						  kRegMaskEHParamEOS, kRegShiftEHParamEOS);
}

bool CNTV2m31EHParam::SetMP4(uint32_t mp4, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPPS, mp4,
						   kRegMaskEHParamMP4, kRegShiftEHParamMP4);
}

bool CNTV2m31EHParam::GetMP4(uint32_t* mp4, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPPS, reinterpret_cast <uint32_t*> (mp4),
						  kRegMaskEHParamMP4, kRegShiftEHParamMP4);
}

bool CNTV2m31EHParam::SetBufCtrl(uint32_t bufCtrl, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamBufCtrl, bufCtrl,
						   kRegMaskEHParamBufCtrl, kRegShiftEHParamBufCtrl);
}

bool CNTV2m31EHParam::GetBufCtrl(uint32_t* bufCtrl, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamBufCtrl, reinterpret_cast <uint32_t*> (bufCtrl),
						  kRegMaskEHParamBufCtrl, kRegShiftEHParamBufCtrl);
}

bool CNTV2m31EHParam::SetVCLHRD(uint32_t vclHRD, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamBufCtrl, vclHRD,
						   kRegMaskEHParamVCLHRD, kRegShiftEHParamVCLHRD);
}

bool CNTV2m31EHParam::GetVCLHRD(uint32_t* vclHRD, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamBufCtrl, reinterpret_cast <uint32_t*> (vclHRD),
						  kRegMaskEHParamVCLHRD, kRegShiftEHParamVCLHRD);
}

bool CNTV2m31EHParam::SetVPSTiming(uint32_t vpsTiming, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamBufCtrl, vpsTiming,
						   kRegMaskEHParamVPSTiming, kRegShiftEHParamVPSTiming);
}

bool CNTV2m31EHParam::GetVPSTiming(uint32_t* vpsTiming, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamBufCtrl, reinterpret_cast <uint32_t*> (vpsTiming),
						  kRegMaskEHParamVPSTiming, kRegShiftEHParamVPSTiming);
}

bool CNTV2m31EHParam::SetOverscanInfo(uint32_t overscanInfo, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamOverscan, overscanInfo,
						   kRegMaskEHParamOverscanInfo, kRegShiftEHParamOverscanInfo);
}

bool CNTV2m31EHParam::GetOverscanInfo(uint32_t* overscanInfo, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamOverscan, reinterpret_cast <uint32_t*> (overscanInfo),
						  kRegMaskEHParamOverscanInfo, kRegShiftEHParamOverscanInfo);
}

bool CNTV2m31EHParam::SetBitRestriction(uint32_t bitRestriction, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamOverscan, bitRestriction,
						   kRegMaskEHParamBitRestriction, kRegShiftEHParamBitRestriction);
}

bool CNTV2m31EHParam::GetBitRestriction(uint32_t* bitRestriction, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamOverscan, reinterpret_cast <uint32_t*> (bitRestriction),
						  kRegMaskEHParamBitRestriction, kRegShiftEHParamBitRestriction);
}

bool CNTV2m31EHParam::SetVideoSignalType(uint32_t videoSignalType, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamVideo, videoSignalType,
						   kRegMaskEHParamVideoSignalType, kRegShiftEHParamVideoSignalType);
}

bool CNTV2m31EHParam::GetVideoSignalType(uint32_t* videoSignalType, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamVideo, reinterpret_cast <uint32_t*> (videoSignalType),
						  kRegMaskEHParamVideoSignalType, kRegShiftEHParamVideoSignalType);
}

bool CNTV2m31EHParam::SetVideoFormat(uint32_t videoFormat, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamVideo, videoFormat,
						   kRegMaskEHParamVideoFormat, kRegShiftEHParamVideoFormat);
}

bool CNTV2m31EHParam::GetVideoFormat(uint32_t* videoFormat, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamVideo, reinterpret_cast <uint32_t*> (videoFormat),
						  kRegMaskEHParamVideoFormat, kRegShiftEHParamVideoFormat);
}

bool CNTV2m31EHParam::SetVideoFullRange(uint32_t videoFullRange, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamVideo, videoFullRange,
						   kRegMaskEHParamVideoFullRange, kRegShiftEHParamVideoFullRange);
}

bool CNTV2m31EHParam::GetVideoFullRange(uint32_t* videoFullRange, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamVideo, reinterpret_cast <uint32_t*> (videoFullRange),
						  kRegMaskEHParamVideoFullRange, kRegShiftEHParamVideoFullRange);
}

bool CNTV2m31EHParam::SetColourDescription(uint32_t colourDescription, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamVideo, colourDescription,
						   kRegMaskEHParamColourDescription, kRegShiftEHParamColourDescription);
}

bool CNTV2m31EHParam::GetColourDescription(uint32_t* colourDescription, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamVideo, reinterpret_cast <uint32_t*> (colourDescription),
						  kRegMaskEHParamColourDescription, kRegShiftEHParamColourDescription);
}

bool CNTV2m31EHParam::SetColourPrimaries(uint32_t colourPrimaries, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamVideo, colourPrimaries,
						   kRegMaskEHParamColourPrimaries, kRegShiftEHParamColourPrimaries);
}

bool CNTV2m31EHParam::GetColourPrimaries(uint32_t* colourPrimaries, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamVideo, reinterpret_cast <uint32_t*> (colourPrimaries),
						  kRegMaskEHParamColourPrimaries, kRegShiftEHParamColourPrimaries);
}

bool CNTV2m31EHParam::SetTransferChar(uint32_t transferChar, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamVideo, transferChar,
						   kRegMaskEHParamTransferChar, kRegShiftEHParamTransferChar);
}

bool CNTV2m31EHParam::GetTransferChar(uint32_t* transferChar, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamVideo, reinterpret_cast <uint32_t*> (transferChar),
						  kRegMaskEHParamTransferChar, kRegShiftEHParamTransferChar);
}

bool CNTV2m31EHParam::SetMatrixCoef(uint32_t matrixCoef, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamMatrixCoef, matrixCoef,
						   kRegMaskEHParamMatrixCoef, kRegShiftEHParamMatrixCoef);
}

bool CNTV2m31EHParam::GetMatrixCoef(uint32_t* matrixCoef, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamMatrixCoef, reinterpret_cast <uint32_t*> (matrixCoef),
						  kRegMaskEHParamMatrixCoef, kRegShiftEHParamMatrixCoef);
}

bool CNTV2m31EHParam::SetChromaLocInfo(uint32_t chromaLocInfo, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamMatrixCoef, chromaLocInfo,
						   kRegMaskEHParamChromaLocInfo, kRegShiftEHParamChromaLocInfo);
}

bool CNTV2m31EHParam::GetChromaLocInfo(uint32_t* chromaLocInfo, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamMatrixCoef, reinterpret_cast <uint32_t*> (chromaLocInfo),
						  kRegMaskEHParamChromaLocInfo, kRegShiftEHParamChromaLocInfo);
}

bool CNTV2m31EHParam::SetSampleLocTop(uint32_t sampleLocTop, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamMatrixCoef, sampleLocTop,
						   kRegMaskEHParamSampleLocTop, kRegShiftEHParamSampleLocTop);
}

bool CNTV2m31EHParam::GetSampleLocTop(uint32_t* sampleLocTop, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamMatrixCoef, reinterpret_cast <uint32_t*> (sampleLocTop),
						  kRegMaskEHParamSampleLocTop, kRegShiftEHParamSampleLocTop);
}

bool CNTV2m31EHParam::SetSampleLocBot(uint32_t sampleLocBot, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamMatrixCoef, sampleLocBot,
						   kRegMaskEHParamSampleLocBot, kRegShiftEHParamSampleLocBot);
}

bool CNTV2m31EHParam::GetSampleLocBot(uint32_t* sampleLocBot, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamMatrixCoef, reinterpret_cast <uint32_t*> (sampleLocBot),
						  kRegMaskEHParamSampleLocBot, kRegShiftEHParamSampleLocBot);
}

bool CNTV2m31EHParam::SetCropLeft(uint32_t cropLeft, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamCropLR, cropLeft,
						   kRegMaskEHParamCropLeft, kRegShiftEHParamCropLeft);
}

bool CNTV2m31EHParam::GetCropLeft(uint32_t* cropLeft, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamCropLR, reinterpret_cast <uint32_t*> (cropLeft),
						  kRegMaskEHParamCropLeft, kRegShiftEHParamCropLeft);
}

bool CNTV2m31EHParam::SetCropRight(uint32_t cropRight, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamCropLR, cropRight,
						   kRegMaskEHParamCropRight, kRegShiftEHParamCropRight);
}

bool CNTV2m31EHParam::GetCropRight(uint32_t* cropRight, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamCropLR, reinterpret_cast <uint32_t*> (cropRight),
						  kRegMaskEHParamCropRight, kRegShiftEHParamCropRight);
}

bool CNTV2m31EHParam::SetCropTop(uint32_t cropTop, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamCropTB, cropTop,
						   kRegMaskEHParamCropTop, kRegShiftEHParamCropTop);
}

bool CNTV2m31EHParam::GetCropTop(uint32_t* cropTop, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamCropTB, reinterpret_cast <uint32_t*> (cropTop),
						  kRegMaskEHParamCropTop, kRegShiftEHParamCropTop);
}

bool CNTV2m31EHParam::SetCropBottom(uint32_t cropBottom, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamCropTB, cropBottom,
						   kRegMaskEHParamCropBottom, kRegShiftEHParamCropBottom);
}

bool CNTV2m31EHParam::GetCropBottom(uint32_t* cropBottom, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamCropTB, reinterpret_cast <uint32_t*> (cropBottom),
						  kRegMaskEHParamCropBottom, kRegShiftEHParamCropBottom);
}

bool CNTV2m31EHParam::SetUseTpIrap(uint32_t useTpIrap, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamRecovery, useTpIrap,
						   kRegMaskEHParamUseTpIrap, kRegShiftEHParamUseTpIrap);
}

bool CNTV2m31EHParam::GetUseTpIrap(uint32_t* useTpIrap, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamRecovery, reinterpret_cast <uint32_t*> (useTpIrap),
						  kRegMaskEHParamUseTpIrap, kRegShiftEHParamUseTpIrap);
}

bool CNTV2m31EHParam::SetPicTiming(uint32_t picTiming, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamRecovery, picTiming,
						   kRegMaskEHParamPicTiming, kRegShiftEHParamPicTiming);
}

bool CNTV2m31EHParam::GetPicTiming(uint32_t* picTiming, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamRecovery, reinterpret_cast <uint32_t*> (picTiming),
						  kRegMaskEHParamPicTiming, kRegShiftEHParamPicTiming);
}

bool CNTV2m31EHParam::SetScanType(uint32_t scanType, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamRecovery, scanType,
						   kRegMaskEHParamScanType, kRegShiftEHParamScanType);
}

bool CNTV2m31EHParam::GetScanType(uint32_t* scanType, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamRecovery, reinterpret_cast <uint32_t*> (scanType),
						  kRegMaskEHParamScanType, kRegShiftEHParamScanType);
}

bool CNTV2m31EHParam::SetFramePacking(uint32_t framePacking, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamRecovery, framePacking,
						   kRegMaskEHParamFramePacking, kRegShiftEHParamFramePacking);
}

bool CNTV2m31EHParam::GetFramePacking(uint32_t* framePacking, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamRecovery, reinterpret_cast <uint32_t*> (framePacking),
						  kRegMaskEHParamFramePacking, kRegShiftEHParamFramePacking);
}

bool CNTV2m31EHParam::SetScanLeft(uint32_t scanLeft, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPanScanLR, scanLeft,
						   kRegMaskEHParamScanLeft, kRegShiftEHParamScanLeft);
}

bool CNTV2m31EHParam::GetScanLeft(uint32_t* scanLeft, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPanScanLR, reinterpret_cast <uint32_t*> (scanLeft),
						  kRegMaskEHParamScanLeft, kRegShiftEHParamScanLeft);
}

bool CNTV2m31EHParam::SetScanRight(uint32_t scanRight, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPanScanLR, scanRight,
						   kRegMaskEHParamScanRight, kRegShiftEHParamScanRight);
}

bool CNTV2m31EHParam::GetScanRight(uint32_t* scanRight, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPanScanLR, reinterpret_cast <uint32_t*> (scanRight),
						  kRegMaskEHParamScanRight, kRegShiftEHParamScanRight);
}

bool CNTV2m31EHParam::SetScanTop(uint32_t scanTop, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPanScanTB, scanTop,
						   kRegMaskEHParamScanTop, kRegShiftEHParamScanTop);
}

bool CNTV2m31EHParam::GetScanTop(uint32_t* scanTop, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPanScanTB, reinterpret_cast <uint32_t*> (scanTop),
						  kRegMaskEHParamScanTop, kRegShiftEHParamScanTop);
}

bool CNTV2m31EHParam::SetScanBottom(uint32_t scanBottom, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPanScanTB, scanBottom,
						   kRegMaskEHParamScanBottom, kRegShiftEHParamScanBottom);
}

bool CNTV2m31EHParam::GetScanBottom(uint32_t* scanBottom, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPanScanTB, reinterpret_cast <uint32_t*> (scanBottom),
						  kRegMaskEHParamScanBottom, kRegShiftEHParamScanBottom);
}

// Protected setters/getters

bool CNTV2m31EHParam::SetEHPrivate01(uint32_t ehprivate01, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSlice, ehprivate01,
						   kRegMaskEHParamPrivate01, kRegShiftEHParamPrivate01);
}

bool CNTV2m31EHParam::GetEHPrivate01(uint32_t* ehprivate01, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSlice, reinterpret_cast <uint32_t*> (ehprivate01),
						  kRegMaskEHParamPrivate01, kRegShiftEHParamPrivate01);
}

bool CNTV2m31EHParam::SetEHPrivate02(uint32_t ehprivate02, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSlice, ehprivate02,
						   kRegMaskEHParamPrivate02, kRegShiftEHParamPrivate02);
}

bool CNTV2m31EHParam::GetEHPrivate02(uint32_t* ehprivate02, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSlice, reinterpret_cast <uint32_t*> (ehprivate02),
						  kRegMaskEHParamPrivate02, kRegShiftEHParamPrivate02);
}

bool CNTV2m31EHParam::SetEHPrivate03(uint32_t ehprivate03, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSlice, ehprivate03,
						   kRegMaskEHParamPrivate03, kRegShiftEHParamPrivate03);
}

bool CNTV2m31EHParam::GetEHPrivate03(uint32_t* ehprivate03, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSlice, reinterpret_cast <uint32_t*> (ehprivate03),
						  kRegMaskEHParamPrivate03, kRegShiftEHParamPrivate03);
}

bool CNTV2m31EHParam::SetEHPrivate04(uint32_t ehprivate04, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSlice, ehprivate04,
						   kRegMaskEHParamPrivate04, kRegShiftEHParamPrivate04);
}

bool CNTV2m31EHParam::GetEHPrivate04(uint32_t* ehprivate04, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSlice, reinterpret_cast <uint32_t*> (ehprivate04),
						  kRegMaskEHParamPrivate04, kRegShiftEHParamPrivate04);
}

bool CNTV2m31EHParam::SetEHPrivate05(uint32_t ehprivate05, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSlice, ehprivate05,
						   kRegMaskEHParamPrivate05, kRegShiftEHParamPrivate05);
}

bool CNTV2m31EHParam::GetEHPrivate05(uint32_t* ehprivate05, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSlice, reinterpret_cast <uint32_t*> (ehprivate05),
						  kRegMaskEHParamPrivate05, kRegShiftEHParamPrivate05);
}

bool CNTV2m31EHParam::SetEHPrivate06(uint32_t ehprivate06, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGop1, ehprivate06,
						   kRegMaskEHParamPrivate06, kRegShiftEHParamPrivate06);
}

bool CNTV2m31EHParam::GetEHPrivate06(uint32_t* ehprivate06, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGop1, reinterpret_cast <uint32_t*> (ehprivate06),
						  kRegMaskEHParamPrivate06, kRegShiftEHParamPrivate06);
}

bool CNTV2m31EHParam::SetEHPrivate07(uint32_t ehprivate07, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGop1, ehprivate07,
						   kRegMaskEHParamPrivate07, kRegShiftEHParamPrivate07);
}

bool CNTV2m31EHParam::GetEHPrivate07(uint32_t* ehprivate07, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGop1, reinterpret_cast <uint32_t*> (ehprivate07),
						  kRegMaskEHParamPrivate07, kRegShiftEHParamPrivate07);
}

bool CNTV2m31EHParam::SetEHPrivate08(uint32_t ehprivate08, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamRCMode, ehprivate08,
						   kRegMaskEHParamPrivate08, kRegShiftEHParamPrivate08);
}

bool CNTV2m31EHParam::GetEHPrivate08(uint32_t* ehprivate08, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamRCMode, reinterpret_cast <uint32_t*> (ehprivate08),
						  kRegMaskEHParamPrivate08, kRegShiftEHParamPrivate08);
}

bool CNTV2m31EHParam::SetEHPrivate09(uint32_t ehprivate09, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamVBRMinBitRate, ehprivate09,
						   kRegMaskEHParamPrivate09, kRegShiftEHParamPrivate09);
}

bool CNTV2m31EHParam::GetEHPrivate09(uint32_t* ehprivate09, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamVBRMinBitRate, reinterpret_cast <uint32_t*> (ehprivate09),
						  kRegMaskEHParamPrivate09, kRegShiftEHParamPrivate09);
}

bool CNTV2m31EHParam::SetEHPrivate10(uint32_t ehprivate10, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamVBRFillerBitRate, ehprivate10,
						   kRegMaskEHParamPrivate10, kRegShiftEHParamPrivate10);
}

bool CNTV2m31EHParam::GetEHPrivate10(uint32_t* ehprivate10, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamVBRFillerBitRate, reinterpret_cast <uint32_t*> (ehprivate10),
						  kRegMaskEHParamPrivate10, kRegShiftEHParamPrivate10);
}

bool CNTV2m31EHParam::SetEHPrivate11(uint32_t ehprivate11, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamCUTUSize, ehprivate11,
						   kRegMaskEHParamPrivate11, kRegShiftEHParamPrivate11);
}

bool CNTV2m31EHParam::GetEHPrivate11(uint32_t* ehprivate11, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamCUTUSize, reinterpret_cast <uint32_t*> (ehprivate11),
						  kRegMaskEHParamPrivate11, kRegShiftEHParamPrivate11);
}

bool CNTV2m31EHParam::SetEHPrivate12(uint32_t ehprivate12, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamTSkip, ehprivate12,
						   kRegMaskEHParamPrivate12, kRegShiftEHParamPrivate12);
}

bool CNTV2m31EHParam::GetEHPrivate12(uint32_t* ehprivate12, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamTSkip, reinterpret_cast <uint32_t*> (ehprivate12),
						  kRegMaskEHParamPrivate12, kRegShiftEHParamPrivate12);
}

bool CNTV2m31EHParam::SetEHPrivate13(uint32_t ehprivate13, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamTSkip, ehprivate13,
						   kRegMaskEHParamPrivate13, kRegShiftEHParamPrivate13);
}

bool CNTV2m31EHParam::GetEHPrivate13(uint32_t* ehprivate13, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamTSkip, reinterpret_cast <uint32_t*> (ehprivate13),
						  kRegMaskEHParamPrivate13, kRegShiftEHParamPrivate13);
}

bool CNTV2m31EHParam::SetEHPrivate14(uint32_t ehprivate14, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamTSkip, ehprivate14,
						   kRegMaskEHParamPrivate14, kRegShiftEHParamPrivate14);
}

bool CNTV2m31EHParam::GetEHPrivate14(uint32_t* ehprivate14, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamTSkip, reinterpret_cast <uint32_t*> (ehprivate14),
						  kRegMaskEHParamPrivate14, kRegShiftEHParamPrivate14);
}

bool CNTV2m31EHParam::SetEHPrivate15(uint32_t ehprivate15, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamAMP_WP, ehprivate15,
						   kRegMaskEHParamPrivate15, kRegShiftEHParamPrivate15);
}

bool CNTV2m31EHParam::GetEHPrivate15(uint32_t* ehprivate15, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamAMP_WP, reinterpret_cast <uint32_t*> (ehprivate15),
						  kRegMaskEHParamPrivate15, kRegShiftEHParamPrivate15);
}

bool CNTV2m31EHParam::SetEHPrivate16(uint32_t ehprivate16, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamAMP_WP, ehprivate16,
						   kRegMaskEHParamPrivate16, kRegShiftEHParamPrivate16);
}

bool CNTV2m31EHParam::GetEHPrivate16(uint32_t* ehprivate16, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamAMP_WP, reinterpret_cast <uint32_t*> (ehprivate16),
						  kRegMaskEHParamPrivate16, kRegShiftEHParamPrivate16);
}

bool CNTV2m31EHParam::SetEHPrivate17(uint32_t ehprivate17, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPCM, ehprivate17,
						   kRegMaskEHParamPrivate17, kRegShiftEHParamPrivate17);
}

bool CNTV2m31EHParam::GetEHPrivate17(uint32_t* ehprivate17, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPCM, reinterpret_cast <uint32_t*> (ehprivate17),
						  kRegMaskEHParamPrivate17, kRegShiftEHParamPrivate17);
}

bool CNTV2m31EHParam::SetEHPrivate18(uint32_t ehprivate18, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPCM, ehprivate18,
						   kRegMaskEHParamPrivate18, kRegShiftEHParamPrivate18);
}

bool CNTV2m31EHParam::GetEHPrivate18(uint32_t* ehprivate18, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPCM, reinterpret_cast <uint32_t*> (ehprivate18),
						  kRegMaskEHParamPrivate18, kRegShiftEHParamPrivate18);
}

bool CNTV2m31EHParam::SetEHPrivate19(uint32_t ehprivate19, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPCM, ehprivate19,
						   kRegMaskEHParamPrivate19, kRegShiftEHParamPrivate19);
}

bool CNTV2m31EHParam::GetEHPrivate19(uint32_t* ehprivate19, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPCM, reinterpret_cast <uint32_t*> (ehprivate19),
						  kRegMaskEHParamPrivate19, kRegShiftEHParamPrivate19);
}

bool CNTV2m31EHParam::SetEHPrivate20(uint32_t ehprivate20, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPCM, ehprivate20,
						   kRegMaskEHParamPrivate20, kRegShiftEHParamPrivate20);
}

bool CNTV2m31EHParam::GetEHPrivate20(uint32_t* ehprivate20, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPCM, reinterpret_cast <uint32_t*> (ehprivate20),
						  kRegMaskEHParamPrivate20, kRegShiftEHParamPrivate20);
}

bool CNTV2m31EHParam::SetEHPrivate21(uint32_t ehprivate21, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPCM, ehprivate21,
						   kRegMaskEHParamPrivate21, kRegShiftEHParamPrivate21);
}

bool CNTV2m31EHParam::GetEHPrivate21(uint32_t* ehprivate21, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPCM, reinterpret_cast <uint32_t*> (ehprivate21),
						  kRegMaskEHParamPrivate21, kRegShiftEHParamPrivate21);
}

bool CNTV2m31EHParam::SetEHPrivate22(uint32_t ehprivate22, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamPCM, ehprivate22,
						   kRegMaskEHParamPrivate22, kRegShiftEHParamPrivate22);
}

bool CNTV2m31EHParam::GetEHPrivate22(uint32_t* ehprivate22, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamPCM, reinterpret_cast <uint32_t*> (ehprivate22),
						  kRegMaskEHParamPrivate22, kRegShiftEHParamPrivate22);
}

bool CNTV2m31EHParam::SetEHPrivate23(uint32_t ehprivate23, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSAO, ehprivate23,
						   kRegMaskEHParamPrivate23, kRegShiftEHParamPrivate23);
}

bool CNTV2m31EHParam::GetEHPrivate23(uint32_t* ehprivate23, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSAO, reinterpret_cast <uint32_t*> (ehprivate23),
						  kRegMaskEHParamPrivate23, kRegShiftEHParamPrivate23);
}

bool CNTV2m31EHParam::SetEHPrivate24(uint32_t ehprivate24, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamSAO, ehprivate24,
						   kRegMaskEHParamPrivate24, kRegShiftEHParamPrivate24);
}

bool CNTV2m31EHParam::GetEHPrivate24(uint32_t* ehprivate24, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamSAO, reinterpret_cast <uint32_t*> (ehprivate24),
						  kRegMaskEHParamPrivate24, kRegShiftEHParamPrivate24);
}

bool CNTV2m31EHParam::SetEHPrivate25(uint32_t ehprivate25, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamRDOQ, ehprivate25,
						   kRegMaskEHParamPrivate25, kRegShiftEHParamPrivate25);
}

bool CNTV2m31EHParam::GetEHPrivate25(uint32_t* ehprivate25, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamRDOQ, reinterpret_cast <uint32_t*> (ehprivate25),
						  kRegMaskEHParamPrivate25, kRegShiftEHParamPrivate25);
}

bool CNTV2m31EHParam::SetEHPrivate26(uint32_t ehprivate26, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamBufCtrl, ehprivate26,
						   kRegMaskEHParamPrivate26, kRegShiftEHParamPrivate26);
}

bool CNTV2m31EHParam::GetEHPrivate26(uint32_t* ehprivate26, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamBufCtrl, reinterpret_cast <uint32_t*> (ehprivate26),
						  kRegMaskEHParamPrivate26, kRegShiftEHParamPrivate26);
}

bool CNTV2m31EHParam::SetEHPrivate27(uint32_t ehprivate27, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGDR, ehprivate27,
						   kRegMaskEHParamPrivate27, kRegShiftEHParamPrivate27);
}

bool CNTV2m31EHParam::GetEHPrivate27(uint32_t* ehprivate27, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGDR, reinterpret_cast <uint32_t*> (ehprivate27),
						  kRegMaskEHParamPrivate27, kRegShiftEHParamPrivate27);
}

bool CNTV2m31EHParam::SetEHPrivate28(uint32_t ehprivate28, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamGDR, ehprivate28,
						   kRegMaskEHParamPrivate28, kRegShiftEHParamPrivate28);
}

bool CNTV2m31EHParam::GetEHPrivate28(uint32_t* ehprivate28, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamGDR, reinterpret_cast <uint32_t*> (ehprivate28),
						  kRegMaskEHParamPrivate28, kRegShiftEHParamPrivate28);
}

bool CNTV2m31EHParam::SetEHPrivate29(uint32_t ehprivate29, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamRecovery, ehprivate29,
						   kRegMaskEHParamPrivate29, kRegShiftEHParamPrivate29);
}

bool CNTV2m31EHParam::GetEHPrivate29(uint32_t* ehprivate29, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamRecovery, reinterpret_cast <uint32_t*> (ehprivate29),
						  kRegMaskEHParamPrivate29, kRegShiftEHParamPrivate29);
}

bool CNTV2m31EHParam::SetEHPrivate30(uint32_t ehprivate30, M31VirtualChannel channel)
{
	return WriteM31EHParam(channel, kRegEHParamHash, ehprivate30,
						   kRegMaskEHParamPrivate30, kRegShiftEHParamPrivate30);
}

bool CNTV2m31EHParam::GetEHPrivate30(uint32_t* ehprivate30, M31VirtualChannel channel)
{
	return ReadM31EHParam(channel, kRegEHParamHash, reinterpret_cast <uint32_t*> (ehprivate30),
						  kRegMaskEHParamPrivate30, kRegShiftEHParamPrivate30);
}

// Utility methods

// These routines will eventually calculate the correct register offset basesd on register enum and channel and using the EH start and channel size
// which will eventually call a driver Read/Write function that will write into the params bar
bool CNTV2m31EHParam::WriteM31EHParam(M31VirtualChannel channel, EHParamRegisterIndex registerIndex, uint32_t registerValue, EHParamRegisterMask registerMask, EHParamRegisterShift registerShift)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Now figure out where this register is based on channel and index
	uint32_t regNum = ((channel * EHPARAM_CH_SIZE) + EHPARAM_REG_START) + (registerIndex * 4);

	return mpDevice->HevcWriteRegister(regNum, registerValue, registerMask, registerShift);
}

bool CNTV2m31EHParam::WriteM31EHParam(M31VirtualChannel channel, EHParamRegisterIndex registerIndex, uint32_t registerValue)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Now figure out where this register is based on channel and index
	uint32_t regNum = ((channel * EHPARAM_CH_SIZE) + EHPARAM_REG_START) + (registerIndex * 4);
	
	return mpDevice->HevcWriteRegister(regNum, registerValue);
}

bool CNTV2m31EHParam::ReadM31EHParam(M31VirtualChannel channel, EHParamRegisterIndex registerIndex, uint32_t* registerValue, EHParamRegisterMask registerMask, EHParamRegisterShift registerShift)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Now figure out where this register is based on channel and index
	uint32_t regNum = ((channel * EHPARAM_CH_SIZE) + EHPARAM_REG_START) + (registerIndex * 4);

	return mpDevice->HevcReadRegister(regNum, registerValue, registerMask, registerShift);
}

bool CNTV2m31EHParam::ReadM31EHParam(M31VirtualChannel channel, EHParamRegisterIndex registerIndex, uint32_t* registerValue)
{
	// Make sure we pass in a valid channel
	if (!IS_VALID_M31VirtualChannel(channel))
		return false;
	
	// Now figure out where this register is based on channel and index
	uint32_t regNum = ((channel * EHPARAM_CH_SIZE) + EHPARAM_REG_START) + (registerIndex * 4);
	
	return mpDevice->HevcReadRegister(regNum, registerValue);
}

