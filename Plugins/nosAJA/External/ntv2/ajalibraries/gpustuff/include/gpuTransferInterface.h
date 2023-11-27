/* SPDX-License-Identifier: MIT */
/*
  This software is provided by AJA Video, Inc. "AS IS"
  with no express or implied warranties.
*/

#ifndef _GPU_TRANSFER_INTERFACE_
#define _GPU_TRANSFER_INTERFACE_
#include "ajabase/common/types.h"
#include <assert.h>
#include <string>
#include "cpuObject.h"
/* An abstract class representing the method by which a frame of video is
   transferred from the AJA hardware to a GPU object.*/
template <class T>
class IGpuTransfer {
public:		
	virtual ~IGpuTransfer(){};
	
	/* Subclasses should override to do any initialization.
	   Caller should call Init() before anything else.*/
	virtual bool Init() = 0;	
	
	virtual void GetGpuPreferredAllocationConstants(uint32_t *alocationalignment, uint32_t *stridealignment) = 0;

	/* Subclasses override to undo the work of Init().
	   Caller should call Destroy() before the end of
	   the life of the object.*/
	virtual void Destroy() = 0;
	
	/* Subclasses override this function to do any transfer initialization that are execution thread specific 
		and which should be done once before any transfers are performed.
	   Caller should call ThreadCleanup() before the same thread exits. */	   
	virtual void BeginTransfers() = 0; 

	/* Subclasses override this function to undo any transfer initialization that are execution thread specific.
	   and which should be done once after all transfers are performed.
	   Caller should call ThreadCleanup() in the same thread as ThreadPrep() is called. */
	virtual void EndTransfers() = 0;

	virtual void RegisterTexture(T* object) = 0;
	virtual void RegisterBuffer(T* object) = 0;

	virtual void UnregisterTexture(T* object) = 0;
	virtual void UnregisterBuffer(T* object) = 0;

	virtual void RegisterTexture(CCpuObject* sysmem) = 0;
	virtual void RegisterBuffer(CCpuObject* sysmem) = 0;

	virtual void UnregisterTexture(CCpuObject* sysmem) = 0;
	virtual void UnregisterBuffer(CCpuObject* sysmem) = 0;

	/* Caller should call BeforeRecordTransfer before every AJA transfer call to prep the
	   buffer texture and render-target for transfer from the AJA hardware to the GPU. */
	virtual void BeforeRecordTransfer(T* object, CCpuObject* sysmem) const = 0;
	
	/* Caller should call AfterRecordTransfer after every call to AJA transfer meant for copying from AJA hardware
	   to the GPU.  Subclasses should override to handle the buffer, texture and render-target.
	   For instance, this function could copy from the buffer to the texture. */
	virtual void AfterRecordTransfer(T* object, CCpuObject* sysmem) const = 0;
	
	/* Caller should call BeforePlaybackTransfer before every call to AJA transfer to prep the
	   buffer texture and render-target for the transfer from the GPU to the AJA hardware.  For instance, this function
	   could copy from the texture to the buffer. */
	virtual void BeforePlaybackTransfer(T* object, CCpuObject* sysmem) const = 0;

	/* Caller should call AfterPlaybackTransfer after every call to AJA transfer meant for copying from the GPU to
	   AJA hardware.  Subclasses should override to handle the buffer, texture and render-target after playback. */
	virtual void AfterPlaybackTransfer(T* object, CCpuObject* sysmem) const = 0;

	/* Caller should use this method to set the number of chunks used in GPU transfers.
		Multiple chunks are used when overlapping GPU transfers with the I/O DMAs */
	virtual void SetNumChunks(uint32_t numChunks) = 0;

	/* Caller should use this method to match the number of chunks used with AJA transfers with the number of chunks 
		used by this class. Multiple chunks are used when overlapping gpu transfers with the I/O DMAs */
	virtual uint32_t GetNumChunks() const = 0;
	
	/* Should return time in milliseconds to transfer the video frame from AJA hardware to the GPU*/
	virtual float GetCardToGpuTime(T* object) const = 0;

	/* Should return time in milliseconds to transfer the video frame from GPU to AJA hardware*/
	virtual float GetGpuToCardTime(T* object) const = 0;
};

#endif


