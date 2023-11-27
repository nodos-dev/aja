/* SPDX-License-Identifier: MIT */
/**
	@file		ntv2m31.h
	@brief		Declaration of CNTV2m31 class.
	@copyright	(C) 2015-2022 AJA Video Systems, Inc.  Proprietary and Confidential information.
**/


#ifndef NTV2M31_H
#define NTV2M31_H

#include "ntv2m31publicinterface.h"
#include "ntv2card.h"

#include "ntv2m31cparam.h"
#include "ntv2m31viparam.h"
#include "ntv2m31vinparam.h"
#include "ntv2m31vaparam.h"
#include "ntv2m31ehparam.h"

#include <assert.h>
#include <string>

#if defined (MSWindows)
#include "ntv2windriverinterface.h"
#elif defined (AJAMac)
#include "ntv2macdriverinterface.h"
#elif defined (AJALinux)
#include "ntv2linuxdriverinterface.h"
#endif


class AJAExport CNTV2m31

{
public:
									CNTV2m31(CNTV2Card* device);
	virtual							~CNTV2m31();
	
	
	/**
	 @brief			ClearAllParams - Sets all params to 0.	This is necessary before you write any params.	The register space is actually memory that must be set to zero for the M31 firmware to function.  Params are ususally set bitfields and not the entire register so it is assumed the undefinced bit fields must be zero.
	 @return		boolean true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				ClearAllParams();

	/**
	 @brief			SetupAllParams - Loads and writes out a set of params for a specified preset to a specified channel
	 @param[in]		preset			M31VideoPreset
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetupAllParams(M31VideoPreset preset, M31Channel channel);
	
	/**
	 @brief			LoadAllParams - Loads a set of params for a specified preset
	 @param[in]		preset			M31VideoPreset
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				LoadAllParams(M31VideoPreset preset);
	
	/**
	 @brief			SetAllParams - Writes a set of params for a specified channel
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetAllParams(M31Channel channel);
	
	/**
	 @brief			SetupCommonParams - Loads and writes out a set of common params for a specified preset to a specified channel
	 @param[in]		preset			M31VideoPreset
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetupCommonParams(M31VideoPreset preset, M31Channel channel);
	
	/**
	 @brief			LoadCommonParams - Loads a set of common params for a specified preset
	 @param[in]		preset			M31VideoPreset
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				LoadCommonParams(M31VideoPreset preset);
	
	/**
	 @brief			SetCommonParams - Writes a set of common params for a specified channel
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetCommonParams(M31Channel channel);

	/**
	 @brief			SetupVIParams - Loads and writes out a set of VI params for a specified preset to a specified channel
	 @param[in]		preset			M31VideoPreset
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetupVIParams(M31VideoPreset preset, M31Channel channel);
	
	/**
	 @brief			LoadVIParams - Loads a set of VI params for a specified preset
	 @param[in]		preset			M31VideoPreset
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				LoadVIParams(M31VideoPreset preset);
	
	/**
	 @brief			SetVIParams - Writes a set of VI params for a specified channel
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetVIParams(M31Channel channel);

	/**
	 @brief			SetupVInParams - Loads and writes out a set of VIn params for a specified preset to a specified channel
	 @param[in]		preset			M31VideoPreset
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetupVInParams(M31VideoPreset preset, M31Channel channel);
	
	/**
	 @brief			LoadVInParams - Loads a set of VIn params for a specified preset
	 @param[in]		preset			M31VideoPreset
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				LoadVInParams(M31VideoPreset preset);
	
	/**
	 @brief			SetVInParams - Writes a set of VIn params for a specified channel
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetVInParams(M31Channel channel);

	/**
	 @brief			SetupVAParams - Loads and writes out a set of VA params for a specified preset to a specified channel
	 @param[in]		preset			M31VideoPreset
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetupVAParams(M31VideoPreset preset, M31Channel channel);
	
	/**
	 @brief			LoadVAParams - Loads a set of VA params for a specified preset
	 @param[in]		preset			M31VideoPreset
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				LoadVAParams(M31VideoPreset preset);
	
	/**
	 @brief			SetVAParams - Writes a set of VA params for a specified channel
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetVAParams(M31Channel channel);

	/**
	 @brief			SetupEHParams - Loads and writes out a set of EH params for a specified preset to a specified channel
	 @param[in]		preset			M31VideoPreset
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetupEHParams(M31VideoPreset preset, M31Channel channel);
	
	/**
	 @brief			LoadEHParams - Loads a set of EH params for a specified preset
	 @param[in]		preset			M31VideoPreset
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				LoadEHParams(M31VideoPreset preset);
	
	/**
	 @brief			SetEHParams - Writes a set of EH params for a specified channel
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				SetEHParams(M31Channel channel);

	/**
	 @brief			ChangeMainState
	 @param[in]		mainState		Main state
	 @param[in]		encodeMode		Mode of encode (default: single)
	 @param[in]		firmwareType	Encode firmware type (default: standard)
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				ChangeMainState(HevcMainState mainState,
													HevcEncodeMode encodeMode = Hevc_EncodeMode_Single,
													HevcFirmwareType firmwareType = Hevc_FirmwareType_Standard);
	
	/**
	 @brief			GetMainState
	 @param[out]	pMainState		Main state
	 @param[out]	pEncodeMode		Encode mode
	 @param[out]	pFirmwareType	Encode firmware type
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				GetMainState(HevcMainState* pMainState,
												 HevcEncodeMode* pEncodeMode = NULL,
												 HevcFirmwareType* pFirmwareType = NULL);

	/**
	 @brief			ChangeVInState
	 @param[in]		vinState		VIn state
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				ChangeVInState(HevcVinState vinState, M31Channel channel);

	/**
	 @brief			ChangeVInState
	 @param[in]		vinState		VIN_ID state
	 @param[in]		vinStreambits	VIN_ID means bit assignment. (Multiple setting is possible.)
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				ChangeVInState(HevcVinState vinState, uint32_t vinStreambits);
	
	/**
	 @brief			GetVInState
	 @param[out]	pVInState		VIn state
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				GetVInState(HevcVinState* pVInState, M31Channel channel);

	/**
	 @brief			ChangeEHState
	 @param[in]		ehState			EH state
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				ChangeEHState(HevcEhState ehState, M31Channel channel);

	/**
	 @brief			ChangeEHState
	 @param[in]		ehState			EH_ID state
	 @param[in]		ehStreambits	EH_ID means bit assignment. (Multiple setting is possible.)
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				ChangeEHState(HevcEhState ehState, uint32_t ehStreambits);
	
	/**
	 @brief			GetEHState
	 @param[out]	pEHState		EH state
	 @param[in]		channel			M31Channel
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				GetEHState(HevcEhState* pEHState, M31Channel channel);

	/**
	 @brief			GetDeviceInfo - Get Version information
	 @param[out]	pDeviceInfo		HevcDeviceInfo
	 **/
	AJA_VIRTUAL void				GetDeviceInfo(HevcDeviceInfo* pDeviceInfo);
	
	/**
	 @brief			Reset
	 @return		boolean true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				Reset();
	
	/**
	 @brief			ChangeBitRateCbr - Change CBR bit rate during encode
	 @param[in]		channel			M31Channel
	 @param[in]		cbrBitRate		the bit rate
	 @return		boolean true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool ChangeBitRateCbr(M31Channel channel, uint32_t cbrBitRate);
	
	
	/**
	 @brief			ChangeBitRateVbr - Change VBR bit rate during encode
	 @param[in]		channel				M31Channel
	 @param[in]		vbrMaxBitRate		the maximum bit rate
	 @param[in]		vbrAveBitRate		the average bit rate
	 @param[in]		ehprivate09		  the minimum bit rate
	 @param[in]		changeSeq			the change sequence
	 @return		boolean				true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool ChangeBitRateVbr(M31Channel channel, uint32_t vbrMaxBitRate, uint32_t vbrAveBitRate, uint32_t ehprivate09, HevcChangeSequence changeSeq);


	/**
	 @brief			RawTransfer - Transfer a progressive frame or stacked field to the codec
	 @param[in]		channel			M31Channel
	 @param[in]		pRawBuffer		Address of the raw frame buffer
	 @param[in]		rawDataSize		Size of the raw frame buffer (bytes)
	 @param[in]		lastFrame		true if last frame of stream
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				RawTransfer(M31Channel channel,
												uint8_t* pRawBuffer, uint32_t rawDataSize,
												bool lastFrame);

	/**
	 @brief			RawTransfer - Transfer a progressive frame or stacked field to the codec
	 @param[in]		channel			M31Channel
	 @param[in]		pRawBuffer		Address of the raw frame buffer
	 @param[in]		rawDataSize		Size of the raw frame buffer (bytes)
	 @param[in]		pPicBuffer		Address of the picture information buffer
	 @param[in]		picDataSize		Size of the picture information buffer (bytes)
	 @param[in]		lastFrame		true if last frame of stream
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				RawTransfer(M31Channel channel,
												uint8_t* pRawBuffer, uint32_t rawDataSize,
												uint8_t* pPicBuffer, uint32_t picDataSize,
												bool lastFrame);

	/**
	 @brief			RawTransfer - Transfer a progressive frame or interlaced field to the codec
	 @param[in]		preset			M31 video preset
	 @param[in]		channel			M31Channel
	 @param[in]		pRawBuffer		Address of the raw frame buffer
	 @param[in]		rawDataSize		Size of the raw frame buffer (bytes)
	 @param[in]		field2			true for field 2 transfer
	 @param[in]		lastFrame		true if last frame of stream
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				RawTransfer(M31VideoPreset preset, M31Channel channel,
												uint8_t* pRawBuffer, uint32_t rawDataSize,
												bool field2, bool lastFrame);

	/**
	 @brief			RawTransfer - Transfer a progressive frame or interlaced field to the codec
	 @param[in]		preset			M31 video preset
	 @param[in]		channel			M31 channel
	 @param[in]		pRawBuffer		Address of the raw frame buffer
	 @param[in]		rawDataSize		Size of the raw frame buffer (bytes)
	 @param[in]		pPicBuffer		Address of the picture information buffer
	 @param[in]		picDataSize		Size of the picture information buffer (bytes)
	 @param[in]		field2			true for field 2 transfer
	 @param[in]		lastFrame		true if last frame of stream
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				RawTransfer(M31VideoPreset preset, M31Channel channel,
												uint8_t* pRawBuffer, uint32_t rawDataSize,
												uint8_t* pPicBuffer, uint32_t picDataSize,
												bool field2, bool lastFrame);

	/**
	 @brief			EncTransfer - Transfer a encodec frame from the codec
	 @param[in]		channel			M31Channel
	 @param[in]		pHevcBuffer		Address of the encoded frame
	 @param[in]		hevcBufferSize	Size of the encoded frame buffer (bytes)
	 @param[out]	hevcDataSize	Size of the encoded frame data (bytes)
	 @param[out]	lastFrame		true if last frame of stream
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				EncTransfer(M31Channel channel,
												uint8_t* pHevcBuffer, uint32_t hevcBufferSize,
												uint32_t& hevcDataSize, bool& lastFrame);

	/**
	 @brief			EncTransfer - Transfer a encodec frame from the codec
	 @param[in]		channel			M31Channel
	 @param[in]		pHevcBuffer		Address of the encoded frame buffer
	 @param[in]		hevcBufferSize	Size of the encoded frame buffer (bytes)
	 @param[in]		pEsBuffer		Address of the encoded information buffer
	 @param[in]		esBufferSize	Size of the encoded information buffer (bytes)
	 @param[out]	hevcDataSize	Size of the encoded frame data (bytes)
	 @param[out]	esDataSize		Size of the encoded information data (bytes)
	 @param[out]	lastFrame		true if last frame of stream
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				EncTransfer(M31Channel channel,
												uint8_t* pHevcBuffer, uint32_t hevcBufferSize,
												uint8_t* pEsBuffer, uint32_t esBufferSize,
												uint32_t& hevcDataSize, uint32_t& esDataSize, bool& lastFrame);

	/**
	 @brief			EncTransfer - Transfer a encodec frame from the codec
	 @param[in]		channel			M31Channel
	 @param[in]		pHevcBuffer		Address of the encoded frame buffer
	 @param[in]		hevcBufferSize	Size of the encoded frame buffer (bytes)
	 @param[in]		pEsBuffer		Address of the encoded information buffer
	 @param[in]		esBufferSize	Size of the encoded information buffer (bytes)
	 @param[out]	hevcDataSize	Size of the encoded frame data (bytes)
	 @param[out]	esDataSize		Size of the encoded information data (bytes)
	 @param[out]	encodeTime		Encode time		
	 @param[out]	lastFrame		true if last frame of stream
	 @return		boolean			true if successful, otherwise false
	 **/
	AJA_VIRTUAL bool				EncTransfer(M31Channel channel,
												uint8_t* pHevcBuffer, uint32_t hevcBufferSize,
												uint8_t* pEsBuffer, uint32_t esBufferSize,
												uint32_t& hevcDataSize, uint32_t& esDataSize,
												int64_t& encodeTime, bool& lastFrame);

	/**
	 @brief			ConvertVideoFormatToPreset - Returns a M31 preset from an NTV2 video format and frame buffer format.
	 @param[in]		videoFormat			NTV2 video format
	 @param[in]		frameBufferFormat	NTV2 frame buffer format
	 @param[in]		vif					select M31 VIF preset
	 @param[out]	preset				M31 video preset
	 @return		boolean				true if successful, otherwise false
	**/
	static bool						ConvertVideoFormatToPreset(NTV2VideoFormat videoFormat,
																	NTV2FrameBufferFormat frameBufferFormat,
																	bool vif,
																	M31VideoPreset& preset);
	/**
	 @brief			GetPresetVideoFormat - Returns a NTV2 video format based on preset.
	 @param[in]		preset				M31VideoPreset
	 @return		videoFormat			NTV2 video format
	 **/
	static NTV2VideoFormat			GetPresetVideoFormat(M31VideoPreset preset);

	/**
	 @brief			GetPresetFrameBufferFormat - Returns a NTV2 frame buffer format based on preset.
	 @param[in]		preset				M31VideoPreset
	 @return		frameBufferFormat	NTV2 frame buffer format
	 **/
	static NTV2FrameBufferFormat	GetPresetFrameBufferFormat(M31VideoPreset preset);

	/**
	 @brief			IsPresetVIF - Returns a true if preset is vif.
	 @param[in]		preset				M31VideoPreset
	 @return		boolean true if preset is vif, otherwise false
	 **/
	static bool						IsPresetVIF(M31VideoPreset preset);

	/**
	 @brief			IsPresetUHD - Returns a true if preset is UHD.
	 @param[in]		preset				M31VideoPreset
	 @return		boolean true if preset is UHD, otherwise false
	 **/
	static bool						IsPresetUHD(M31VideoPreset preset);

	/**
	 @brief			IsPresetInterlaced - Returns a true if frame buffer is interlaced.
	 @param[in]		preset				M31VideoPreset
	 @return		boolean true if preset is interlaced, otherwise false
	 **/
	static bool						IsPresetInterlaced(M31VideoPreset preset);

	/**
	 @brief			GetPresetPixelsPerLine - Returns the frame buffer number of pixels per line
	 @param[in]		preset				M31VideoPreset
	 @return		number of pixels per line
	 **/
	static uint32_t					GetPresetPixelsPerLine(M31VideoPreset preset);

	/**
	 @brief			GetPresetLinesPerFrame - Returns the frame buffer number of lines per frame
	 @param[in]		preset				M31VideoPreset
	 @return		number of lines per frame
	 **/
	static uint32_t					GetPresetLinesPerFrame(M31VideoPreset preset);

	/**
	 @brief			GetPresetLinePitch - Returns the frame buffer line pitch
	 @param[in]		preset				M31VideoPreset
	 @return		line pitch (bytes)
	 **/
	static uint32_t					GetPresetLinePitch(M31VideoPreset preset);

	/**
	 @brief			GetPresetPitchCount - Returns the frame buffer total pitch lines
	 @param[in]		preset				M31VideoPreset
	 @return		pitch count
	 **/
	static uint32_t					GetPresetPitchCount(M31VideoPreset preset);

	/**
	 @brief			GetPresetFrameSize - Returns the frame buffer frame size
	 @param[in]		preset				M31VideoPreset
	 @return		size (bytes)
	 **/
	static uint32_t					GetPresetFrameSize(M31VideoPreset preset);

	// Helper classes used to set and get params
	CNTV2m31CParam*					mpM31CParam;
	CNTV2m31VIParam*				mpM31VIParam;
	CNTV2m31VInParam*				mpM31VInParam;
	CNTV2m31VAParam*				mpM31VAParam;
	CNTV2m31EHParam*				mpM31EHParam;
	
	// Structs used to get and set params
	CParamsChannel					mCParamsChannel;
	VIParamsChannel					mvIParamsChannel;
	VInParamsChannel				mVInParamsChannel;
	VAParamsChannel					mVAParamsChannel;
	EHParamsChannel					mEHParamsChannel;
	
private:
	CNTV2Card*						mpDevice;

};	//	CNTV2m31

#endif	//	NTV2M31_H
