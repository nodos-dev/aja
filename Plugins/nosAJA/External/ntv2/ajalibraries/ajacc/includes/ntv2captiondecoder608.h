/**
	@file		ntv2captiondecoder608.h
	@brief		Declares the CNTV2CaptionDecoder608 class.
	@copyright	(C) 2006-2022 AJA Video Systems, Inc. All rights reserved.
**/

#ifndef __NTV2_CEA608_DECODER_
#define __NTV2_CEA608_DECODER_

#include "ntv2captiondecodechannel608.h"
#include "ntv2xdscaptiondecodechannel608.h"
#include "ntv2formatdescriptor.h"
#include "ajabase/common/ajarefptr.h"
#include "ccfont.h"
#include <string>
#include <vector>

#ifdef AJAMac
	#define	odprintf	printf
#endif	//	AJAMac

#ifdef MSWindows
	#include "windows.h"
	#include "stdio.h"
#endif


/**
	@brief	I am a full-featured CEA-608 ("Line 21") captioning decoder. I accept caption data (2 bytes/field),
			parse it to the appropriate channel (CC1, CC2, etc.), where the current state and buffer status is
			maintained. I currently handle parsing for captioning channels CC1-CC4, Text1-Text4, and XDS in
			parallel -- i.e. I'm decoding all channels simultaneously.

			For display purposes, I provide CNTV2CaptionDecoder608::SetDisplayChannel to select the current displayed channel.
			However, since all channels are being decoded in parallel, you may change the displayed channel at
			any time and immediately see the state of the newly selected channel. You don't have to wait for the
			channel to receive enough new data to get in-sync.

			I also have methods to "burn-in" caption glyphs into video for the selected display channel.
			CNTV2CaptionDecoder608::BurnCaptions will do the entire burn-in (for a limited number of frame buffer
			sizes and formats), or you can call lower-level methods to extract the current state of the displayed channel.

			To implement a basic decoder:
			-	Call CNTV2CaptionDecoder608::Create to create a ::CNTV2CaptionDecoder608 instance;
			-	Call CNTV2CaptionDecoder608::SetDisplayChannel methods to select the current displayed channel (if any);
			-	Call CNTV2CaptionDecoder608::ProcessNew608FrameData every frame, passing it the four bytes of CEA-608 data
				that arrive every frame (2 bytes per field for SD/interlaced video).

			CNTV2CaptionDecoder608::ProcessNew608FrameData should be called every frame, even if the captioning data is "null"
			(zeros). CEA-608 caption rules call for certain commands to be sent twice on adjacent frames, which means there's
			a difference between "Command|Command|Command" and "Command|null|Command", and if the ::CNTV2CaptionDecoder608
			instance never sees the intervening NULL frames, it can mistakenly think two commands come from adjacent frames
			and misinterpret them.

			To display the caption data, call CNTV2CaptionDecoder608::SetDisplayChannel to select the captioning channel to
			display, then call CNTV2CaptionDecoder608::BurnCaptions with a pointer to the frame buffer holding the video.

			Clients can implement their own caption burn-in, if needed. Just call CNTV2CaptionDecoder608::GetOnAirCharacter
			to get the ASCII character at each "cell" of the raster. If using the burn-in feature, CNTV2CaptionDecoder608::IdleFrame
			needs to called once per frame to "blink" text having the "Flash" attribute.
**/
class CNTV2CaptionDecoder608;
typedef AJARefPtr <CNTV2CaptionDecoder608>	CNTV2CaptionDecoder608Ptr;

class AJAExport CNTV2CaptionDecoder608 : public CNTV2CaptionLogConfig
{
	//	Class Methods
	public:
		/**
			@brief		Creates a new CNTV2CaptionEncoder608 instance.
			@param[out]	outEncoder	Receives the newly-created encoder instance.
			@return		True if successful; otherwise False.
		**/
		static bool				Create (CNTV2CaptionDecoder608Ptr & outEncoder);


		/**
			@brief	Given a complete frame of interlaced SD video, returns a CaptionData struct that contains
					the four possible byte values decoded from line 21 of both fields.

			@param[in]	pInFrameData	A valid, non-NULL pointer of the start of the YCbCr 8-bit or YCbCr 10-bit frame buffer.

			@param[in]	inPixelFormat	Specifies the frame buffer format of the given buffer.
										Must be NTV2_FBF_10BIT_YCBCR or NTV2_FBF_8BIT_YCBCR.

			@param[in]	inVideoFormat	Specifies the video format of the video in the given buffer.
										Must be an SD format.

			@param[in]	inFrameGeometry	Optionally Specifies the video frame geometry for the given buffer.
										Defaults to NTV2_FG_720x486 (no VANC).

			@return	CaptionData found, if any.
		**/
		static CaptionData		DecodeCaptionData (const UByte *				pInFrameData,
													const NTV2FrameBufferFormat	inPixelFormat,
													const NTV2VideoFormat		inVideoFormat,
													const NTV2FrameGeometry		inFrameGeometry = NTV2_FG_720x486);


	//	Instance Methods
	public:
		/**
			@brief		Flushes me, clearing all in-progress data.
			@note		This is NOT thread-safe!
		**/
		virtual void						Reset (void);

		/**
			@brief		Changes the caption channel that I'm focused on (or that I'm currently "burning" into video).
			@param[in]	inChannel	Specifies the new caption channel that I am to focus on.
			@return		True if successful; otherwise False.
		**/
		virtual	bool						SetDisplayChannel (const NTV2Line21Channel inChannel);

		/**
			@brief		Answers with the caption channel that I'm currently focused on (or that I'm currently "burning" into video).
			@return		My current NTV2Line21Channel of interest.
		**/
		virtual	inline NTV2Line21Channel	GetDisplayChannel (void) const		{return mDisplayChannel;}


		/**
			@brief		Call this once per video frame when I'm doing display "burn-in", whether there is new caption
						data to parse or not. This keeps "flash" mode regular during "burn-in".
		**/
		virtual void						IdleFrame (void);


		/**
			@brief		Notifies me that new frame data has arrived. Clients should call this method
						once per video frame with the four bytes (2 per field) of new captioning data.
			@param[in]	inCC608Data		Specifies the caption data that arrived for the lastest frame.
			@return		True if successful;  otherwise False.
		**/
		virtual bool						ProcessNew608FrameData (const CaptionData & inCC608Data);

		/**
			@brief		Retrieves the "on-air" character and its attributes at the given on-screen row and column position.
			@param[in]	inRowNumber		Specifies the row number of interest. Values less than one or
										greater than 15 will be clamped to those limits.
			@param[in]	inColNumber		Specifies the column number of interest. Values less than one or
										greater than 32 will be clamped to those limits.
			@param[out]	outAttribs		Receives the attributes of the "on-air" character of interest.
			@return		The "on-air" character if successful;  otherwise zero.
		**/
		virtual UByte						GetOnAirCharacterWithAttributes (const UWord			inRowNumber,
																			const UWord				inColNumber,
																			NTV2Line21Attributes &	outAttribs) const;

		/**
			@brief		Returns the UTF-16 character that best represents the caption character at the given screen position.
			@param[in]	inRow		The row number (1-15).
			@param[in]	inCol		The column number (1-32).
			@param[out]	outAttr		Receives the NTV2Line21Attributes of the on-screen character.
			@return		The UTF-16 character that best represents the on-screen caption character, or zero upon failure.
		**/
		virtual UWord						GetOnAirUTF16CharacterWithAttributes (const UWord inRow, const UWord inCol,
																					NTV2Line21Attributes & outAttr) const;


		/**
			@brief		Retrieves the "on-air" character and its attributes at the given on-screen row and column position.
			@param[in]	inRowNumber		Specifies the row number of interest. Values less than one or
										greater than 15 will be clamped to those limits.
			@param[in]	inColNumber		Specifies the column number of interest. Values less than one or
										greater than 32 will be clamped to those limits.
			@param[out]	outAttribs		Receives the attributes of the "on-air" character of interest.
			@return		A UTF-8 encoded string that contains the "on-air" character, if successful;  otherwise an empty string.
		**/
		virtual std::string					GetOnAirCharacter (const UWord				inRowNumber,
																const UWord				inColNumber,
																NTV2Line21Attributes &	outAttribs) const;

		/**
			@brief		Retrieves the "on-air" character at the given on-screen row and column position.
			@param[in]	inRowNumber		Specifies the row number of interest. Values less than one or
										greater than 15 will be clamped to those limits.
			@param[in]	inColNumber		Specifies the column number of interest. Values less than one or
										greater than 32 will be clamped to those limits.
			@return		A UTF-8 encoded string that contains the "on-air" character, if successful;  otherwise an empty string.
		**/
		virtual std::string					GetOnAirCharacter (const UWord		inRowNumber,
																const UWord		inColNumber) const;

		/**
			@brief		Retrieves all "on-air" characters.
			@param[in]	inRowNumber		Specifies the row number of interest. Zero, the default, will return all rows.
			@return		A UTF-8 encoded string that contains all "on-air" characters, if successful;  otherwise an empty string.
						Multiple rows will include newline character(s) between successive rows.
		**/
		virtual std::string					GetOnAirCharacters (const UWord inRowNumber = 0) const;

		/**
			@brief		Blits all of my current caption channel's "on-air" captions into the given host buffer
						with the correct colors, display attributes and positioning.
			@param		inFB	Specifies a valid host buffer that is to be blitted into.
			@param[in]	inFD	Describes the raster and pixel format of the given host buffer.
			@return		True if successful;  otherwise False.
		**/
		virtual bool						BurnCaptions (NTV2_POINTER & inFB, const NTV2FormatDescriptor & inFD);	//	New in SDK 16.0

		/**
			@brief		Returns the number of rows used for displaying Text Mode captions for the given (TxN) caption channel.
			@param[in]	inChannel	Specifies the [Text] caption channel of interest.
			@return		True if successful;  otherwise false.
		**/
		virtual UWord						GetTextModeDisplayRowCount (const NTV2Line21Channel inChannel);

		/**
			@brief		Changes the number of rows used for displaying Text Mode captions for the given (TxN) caption channel.
			@param[in]	inChannel	Specifies the [Text] caption channel to be configured.
			@param[in]	inNumRows	Specifies the number of rows to use for displaying Text Mode captions.
									Must be at least 1 and no more than 32.
			@return		True if successful;  otherwise false.
		**/
		virtual bool						SetTextModeDisplayRowCount (const NTV2Line21Channel inChannel, const UWord	inNumRows);

		/**
			@brief		Returns the display attributes that Text Mode captions are currently using (assuming my caption
						channel is Tx1/Tx2/Tx3/Tx4).
			@param[in]	inChannel		Specifies the [Text] caption channel of interest.
			@return		My current Text Mode caption display attributes.
		**/
		virtual const NTV2Line21Attributes &	GetTextModeDisplayAttributes (const NTV2Line21Channel inChannel) const;

		/**
			@brief		Sets the display attributes that Text Mode captions will use henceforth for the given (TxN) caption channel.
			@param[in]	inChannel		Specifies the [Text] caption channel to be configured.
			@param[in]	inAttributes	Specifies the new display attributes for Text Mode captions will have going forward.
			@note		This has no effect on captions that have already been decoded (that may be currently displayed).
		**/
		virtual void						SetTextModeDisplayAttributes (const NTV2Line21Channel inChannel, const NTV2Line21Attributes & inAttributes);


		/**
			@brief		Subscribes to change notifications.
			@param[in]	pInCallback		Specifies a pointer to the callback function to be called if/when changes occur.
			@param[in]	pInUserData		Optionally specifies a data pointer to be passed to the callback function.
			@return		True if successful;  otherwise False.
			@note		In this implementation, each decoder instance only accommodates a single subscriber.
						Thus, each call to this function replaces the callback/userData used in prior calls to this function.
		**/
		virtual bool						SubscribeChangeNotification (NTV2Caption608Changed * pInCallback,
																		void * pInUserData = NULL);

		/**
			@brief		Unsubscribes a prior change notification subscription.
			@param[in]	pInCallback		Specifies a pointer to the callback function that was specified in the prior call to SubscribeChangeNotification.
			@param[in]	pInUserData		Specifies the userData pointer that was specified in the prior call to SubscribeChangeNotification.
			@return		True if successful;  otherwise False.
		**/
		virtual bool						UnsubscribeChangeNotification (NTV2Caption608Changed *	pInCallback,
																			void * pInUserData = NULL);


		/**
			@brief	My destructor.
		**/
		virtual								~CNTV2CaptionDecoder608 ();

		virtual NTV2CaptionLogMask			SetLogMask (const NTV2CaptionLogMask inLogMask);

#if !defined(NTV2_DEPRECATE_16_0)
		/**
			@deprecated	Use the other BurnCaptions function that accepts an NTV2_POINTER and NTV2FormatDescriptor.
		**/
		virtual NTV2_SHOULD_BE_DEPRECATED(bool BurnCaptions (UByte*	pBuf, const NTV2FrameDimensions	fd, const NTV2PixelFormat pf, const UWord rb));
#endif	//	!defined(NTV2_DEPRECATE_16_0)


	//	PRIVATE INSTANCE METHODS
	private:
		virtual bool						New608FieldData (const UByte inCharP1, const UByte inCharP2, const NTV2Line21Field inField);
		virtual bool						ParseCaptionData (const UByte inCharP1, const UByte inCharP2, const NTV2Line21Field inField, const NTV2Line21Channel inChannel);
		virtual bool						ParseXDSData (const UByte inCharP1, const UByte inCharP2, const NTV2Line21Field inField, const NTV2Line21Channel inChannel);

		virtual NTV2Line21Channel			GetCaptionChannel (const UByte inCharP1, const UByte inCharP2, const NTV2Line21Field inField);

		//	Hidden constructors & assignment operators
		explicit							CNTV2CaptionDecoder608 ();
		explicit inline						CNTV2CaptionDecoder608 (const CNTV2CaptionDecoder608 & inDecoderToCopy);
		virtual CNTV2CaptionDecoder608 &	operator = (const CNTV2CaptionDecoder608 & inDecoderToCopy);
	public:
		// Debug
		virtual void						DebugPrintCurrentScreen (const bool inAllChannels = false, const bool inShowChars = true, const bool inShowTextChannels = false);
		virtual void						SetDebugRowsOfInterest (const NTV2Line21Channel inChannel, const UWord inFromRow, const UWord inToRow, const bool inAdd = false);
		virtual void						SetDebugColumnsOfInterest (const NTV2Line21Channel inChannel, const UWord inFromCol, const UWord inToCol, const bool inAdd = false);
		virtual CNTV2CaptionDecodeChannel608Ptr	Get608ChannelDecoder (const NTV2Line21Channel inChannel) const;

	private:
		virtual void						Handle608ChangeNotification (const NTV2Caption608ChangeInfo & inChangeInfo) const;
		static void							NTV2Caption608ChangeHandler (void * pInstance, const NTV2Caption608ChangeInfo & inChangeInfo);


	//	INSTANCE DATA
	private:
		typedef std::vector <CNTV2CaptionDecodeChannel608Ptr>	ChannelDecoderArray;	/// @brief	An ordered sequence of CNTV2CaptionDecodeChannel608 instances

		NTV2Line21Channel				mDisplayChannel;		///< @brief	The captioning channel (CC1, CC2, Text1, etc.) I'm supposed to decode
		NTV2Line21Channel				mCurrXmitChannel [2];	///< @brief	The captioning channel currently being transmitted in each field (CC1 or CC2)

		ChannelDecoderArray				mChannelDecoders;		///< @brief	One of these per captioning channel (but not the XDS channel)
																///<		(i.e. I'm decoding all channels in parallel, but only displaying 1-at-a-time)

		CNTV2XDSDecodeChannel608Ptr		mXDSDecode;				///< @brief	A place to send the XDS data to

		unsigned short					mLastControlCode [2];	///< @brief	Used to remember last 16-bit control code for each field (to handle duplicate transmissions)
		UWord							mRollOffset;			///< @brief	Used to do dynamic "roll" at Carriage Return points
		UWord							mFlashCount;			///< @brief	Used to track when flash characters are displayed versus blanked
		NTV2Caption608Changed *			mpChangeSubscriber;		///< @brief	User callback for change notifications
		void *							mpSubscriberData;		///< @brief	User data for change notifications
	#if defined (AJA_DEBUG)
		public:
			static bool					ClassTest (void);
		protected:
			bool						InstanceTest (void);
	#endif	//	AJA_DEBUG

};	//	CNTV2CaptionDecoder608

#endif	// __NTV2_CEA608_DECODER_
