/* SPDX-License-Identifier: MIT */
/*
  This software is provided by AJA Video, Inc. "AS IS"
  with no express or implied warranties.
*/


#include "cudaTransfer.h"
#include <DVPAPI.h>
#include <dvpapi_cuda.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <map>

using namespace std;

#define TIME_MEASUREMENTS


struct TimeInfo{
	uint64_t cardToSysMemStart;
	uint64_t cardToSysMemEnd;
	uint64_t sysMemToGpuStart;
	uint64_t sysMemToGpuEnd;
	uint64_t gpuToSysMemStart;	
	uint64_t gpuToSysMemEnd;	
	uint64_t sysMemToCardStart;
	uint64_t sysMemToCardEnd;
	float cardToGpuTime;
	float gpuToCardTime;
};


struct SyncInfo {
    volatile uint32_t *sem;
    volatile uint32_t *semOrg;
    volatile uint32_t releaseValue;
    volatile uint32_t acquireValue;
    DVPSyncObjectHandle syncObj;
};

struct BufferDVPInfo {
	bool bTexture;
	DVPBufferHandle handle;
	SyncInfo sysMemSyncInfo;
	SyncInfo gpuSyncInfo;
	uint32_t currentChunk;
};

class CCudaTransfer : public ICudaTransfer{
public:
	CCudaTransfer();
	virtual ~CCudaTransfer();
	
	virtual bool Init();
	virtual void Destroy();
	
	virtual void BeginTransfers(); //this has to be called in the thread where the transfers will be performed
	virtual void EndTransfers();//this has to be called in the thread where the transfers will be performed
	
	//requires a current GPU context
	virtual void RegisterTexture(CCudaObject* object);
	//requires a current GPU context
	virtual void RegisterBuffer(CCudaObject* object);


	virtual void UnregisterTexture(CCudaObject* object);
	virtual void UnregisterBuffer(CCudaObject* object);
	
	//requires a current GPU context
	virtual void RegisterTexture(CCpuObject* object);
	//requires a current GPU context
	virtual void RegisterBuffer(CCpuObject* object);


	virtual void UnregisterTexture(CCpuObject* object);
	virtual void UnregisterBuffer(CCpuObject* object);

	virtual void BeforeRecordTransfer(CCudaObject* object, CCpuObject* sysmem) const;
	virtual void AfterRecordTransfer(CCudaObject* object, CCpuObject* sysmem) const;

	virtual void BeforePlaybackTransfer(CCudaObject* object, CCpuObject* sysmem) const;
	virtual void AfterPlaybackTransfer(CCudaObject* object, CCpuObject* sysmem) const;


	virtual void AcquireObject(CCudaObject* object, CUstream stream) const;
	virtual void ReleaseObject(CCudaObject* object, CUstream stream) const;

	virtual uint32_t GetNumChunks() const;
	virtual void SetNumChunks(uint32_t numChunks);	

	virtual float GetCardToGpuTime(CCudaObject* object) const;

	virtual float GetGpuToCardTime(CCudaObject* object) const;
	
	//requires a current GPU context
	virtual void GetGpuPreferredAllocationConstants(uint32_t *alocationalignment, uint32_t *stridealignment);
private:

	uint32_t _numChunks; //specifies the number of chunks used in the transfers. Used for overlapped GPU and Video I/O transfers

	mutable std::map<CCpuObject*, BufferDVPInfo*> _dvpInfoMap;
	mutable std::map<CCudaObject *, DVPBufferHandle> _bufferHandleMap;

	mutable std::map<CCudaObject *, TimeInfo*> _bufferTimeInfoMap;

	virtual void copyCPUToGPU(CCudaObject* object, CCpuObject* sysmem) const;
	virtual void copyGPUToCPU(CCudaObject* object, CCpuObject* sysmem) const;

	virtual void copyNextChunkCPUToGPU(CCudaObject* object, CCpuObject* sysmem) const;
	virtual void copyNextChunkGPUToCPU(CCudaObject* object, CCpuObject* sysmem) const;
	

	TimeInfo* getTimeInfo(CCudaObject* object) const;
	void initSyncInfo(SyncInfo *si) const;
	
	BufferDVPInfo* getBufferDVPInfo(CCpuObject* object) const;	
	DVPBufferHandle getDVPHandleForObject(CCudaObject* object) const;
};

ICudaTransfer *CreateCudaTransfer()
{
	return new CCudaTransfer();
}

static void fail(DVPStatus hr)
{
    //odprintf("DVP Failed with status %X\n", hr);
	printf("DVP Failed with status %X\n", hr);
	exit(0);
}

/*odprintf("Fail on line %d\n", __LINE__); \ */
#define DVP_SAFE_CALL(cmd) { \
    DVPStatus hr = (cmd); \
    if (DVP_STATUS_OK != hr) { \
        printf("Fail on line %d\n", __LINE__); \
        fail(hr); \
    } \
}

#define MEM_RD32(a) (*(const volatile unsigned int *)(a))
#define MEM_WR32(a, d) do { *(volatile unsigned int *)(a) = (d); } while (0)



void CUDAArrayParamsToDVPParams(CUarray_format in_format, int in_numChannels, DVPBufferFormats *out_format, DVPBufferTypes *out_type)
{
		switch(in_format)
		{
		case CU_AD_FORMAT_UNSIGNED_INT8:
			*out_type  = DVP_UNSIGNED_BYTE;
				
			break;
		case CU_AD_FORMAT_SIGNED_INT8:
			*out_type  = DVP_BYTE;
						
			break;
		case CU_AD_FORMAT_UNSIGNED_INT16:
			*out_type  = DVP_UNSIGNED_SHORT;
					
			break;

		case CU_AD_FORMAT_SIGNED_INT16:
			*out_type  = DVP_SHORT;
						
			break;
		case CU_AD_FORMAT_HALF:
			*out_type  = DVP_HALF_FLOAT;
			
			break;
		case CU_AD_FORMAT_UNSIGNED_INT32:
			*out_type  = DVP_UNSIGNED_INT;
			
			break;
		case CU_AD_FORMAT_SIGNED_INT32:
			*out_type  = DVP_INT;
			
			break;
		case CU_AD_FORMAT_FLOAT:
			*out_type  = DVP_FLOAT;
			break;				
		default:
			*out_type  = DVP_UNSIGNED_BYTE;
						
		}
		switch(in_numChannels)
		{
		case 1:
			*out_format = DVP_CUDA_1_CHANNEL;
			break;
		case 2:
			*out_format = DVP_CUDA_2_CHANNELS;
			break;
		case 4:
			*out_format = DVP_CUDA_4_CHANNELS;
			break;
		default:
			*out_format = DVP_CUDA_1_CHANNEL;			
		}		
}

CCudaTransfer::CCudaTransfer() :
	_numChunks(1)
{		
}

CCudaTransfer::~CCudaTransfer()
{
}

//requires a current GPU context
void CCudaTransfer::GetGpuPreferredAllocationConstants(uint32_t *alocationalignment, uint32_t *stridealignment)
{
	uint32_t _bufferAddrAlignment;		
	uint32_t _bufferGPUStrideAlignment;	 
	uint32_t _semaphoreAddrAlignment;	
	uint32_t _semaphoreAllocSize;		
	uint32_t _semaphorePayloadOffset;
	uint32_t _semaphorePayloadSize;
	DVP_SAFE_CALL(dvpGetRequiredConstantsCUDACtx(&_bufferAddrAlignment,
		&_bufferGPUStrideAlignment,
		&_semaphoreAddrAlignment,
		&_semaphoreAllocSize,
		&_semaphorePayloadOffset,
		&_semaphorePayloadSize));
	*alocationalignment = _bufferAddrAlignment;
	*stridealignment = _bufferGPUStrideAlignment;
	

}
bool CCudaTransfer::Init()
{
	DVP_SAFE_CALL(dvpInitCUDAContext(DVP_DEVICE_FLAGS_SHARE_APP_CONTEXT));
	
	return true;
}



void CCudaTransfer::Destroy()
{
	DVP_SAFE_CALL(dvpCloseCUDAContext());
}


//dvpMemcpy functions must be encapsulated with the dvp begin and end calls
//for optimal performance, call these once per thread instead of every frame
//using the InitTransfers and DeinitTransfers methods.

void CCudaTransfer::BeginTransfers() 
{
	DVP_SAFE_CALL(dvpBegin());
}

void CCudaTransfer::EndTransfers()
{
	DVP_SAFE_CALL(dvpEnd());
}

uint32_t CCudaTransfer::GetNumChunks() const
{
	return _numChunks;

}
void CCudaTransfer::SetNumChunks(uint32_t numChunks)
{
	_numChunks = numChunks;
}

BufferDVPInfo* CCudaTransfer::getBufferDVPInfo(CCpuObject *sysmem) const
{
	
	map<CCpuObject*, BufferDVPInfo*>::iterator itr = _dvpInfoMap.find(sysmem);
	
	if( itr == _dvpInfoMap.end() )
	{
		return NULL;
	}
	else
		return itr->second;
}


void CCudaTransfer::initSyncInfo(SyncInfo *si) const
{
	uint32_t _bufferAddrAlignment;		
	uint32_t _bufferGPUStrideAlignment;	 
	uint32_t _semaphoreAddrAlignment;	
	uint32_t _semaphoreAllocSize;		
	uint32_t _semaphorePayloadOffset;
	uint32_t _semaphorePayloadSize;
	DVP_SAFE_CALL(dvpGetRequiredConstantsCUDACtx(&_bufferAddrAlignment,
		&_bufferGPUStrideAlignment,
		&_semaphoreAddrAlignment,
		&_semaphoreAllocSize,
		&_semaphorePayloadOffset,
		&_semaphorePayloadSize));

	DVPSyncObjectDesc syncObjectDesc = {0};
    assert((_semaphoreAllocSize != 0) && (_semaphoreAddrAlignment != 0));
    si->semOrg = (uint32_t *) malloc(_semaphoreAllocSize+_semaphoreAddrAlignment-1);
	
    // Correct alignment
    uint64_t val = (uint64_t)si->semOrg;
    val += _semaphoreAddrAlignment-1;
    val &= ~(_semaphoreAddrAlignment-1);
    si->sem = (uint32_t *) val;
    
    // Initialise description
    MEM_WR32(si->sem, 0);
    si->releaseValue = 0;
    si->acquireValue = 0;
    syncObjectDesc.externalClientWaitFunc = NULL;
	syncObjectDesc.flags = 0;
    syncObjectDesc.sem = (uint32_t *)si->sem;
	
    DVP_SAFE_CALL(dvpImportSyncObject(&syncObjectDesc, &si->syncObj));
}



void CCudaTransfer::RegisterBuffer(CCudaObject* object)
{
	
	DVPBufferHandle dvpHandle;
	CUdeviceptr bufferHandle = object->GetBufferHandle();
	DVP_SAFE_CALL(dvpCreateGPUCUDADevicePtr(
		bufferHandle,
		&dvpHandle));
#ifdef TIME_MEASUREMENTS
	TimeInfo *timeInfo = new TimeInfo;
	memset(timeInfo, 0, sizeof(TimeInfo));
	_bufferTimeInfoMap[object] = timeInfo;
#endif
	_bufferHandleMap[object] = dvpHandle;
	
}

void CCudaTransfer::RegisterTexture(CCudaObject* object)
{
	
	DVPBufferHandle dvpHandle;
	CUarray textureHandle = object->GetTextureHandle();
	DVP_SAFE_CALL(dvpCreateGPUCUDAArray(
		textureHandle,
		&dvpHandle));
#ifdef TIME_MEASUREMENTS
	TimeInfo *timeInfo = new TimeInfo;
	memset(timeInfo, 0, sizeof(TimeInfo));
	_bufferTimeInfoMap[object] = timeInfo;
#endif
	_bufferHandleMap[object] = dvpHandle;
}


void CCudaTransfer::UnregisterTexture(CCudaObject* object)
{
	
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	DVP_SAFE_CALL(dvpFreeBuffer(dvpHandle));
	_bufferHandleMap.erase(object);
	_bufferTimeInfoMap.erase(object);
}
void CCudaTransfer::UnregisterBuffer(CCudaObject* object)
{
	
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	DVP_SAFE_CALL(dvpFreeBuffer(dvpHandle));
	_bufferHandleMap.erase(object);
	_bufferTimeInfoMap.erase(object);
}



void CCudaTransfer::RegisterBuffer(CCpuObject* sysmem)
{
	
	BufferDVPInfo* info = new BufferDVPInfo;
	
	DVPSysmemBufferDesc desc;
	
	desc.size = sysmem->GetSize();
	
	desc.format = DVP_BUFFER;
	desc.type = DVP_UNSIGNED_BYTE;
	desc.bufAddr = sysmem->GetVideoBuffer();
	
	DVP_SAFE_CALL(dvpCreateBuffer( &desc, &(info->handle) ));
	DVP_SAFE_CALL(dvpBindToCUDACtx( info->handle ));
	
	initSyncInfo(&(info->sysMemSyncInfo));
	initSyncInfo(&(info->gpuSyncInfo));
	
	info->currentChunk = 0;
	info->bTexture = false;
	_dvpInfoMap[sysmem] = info;			
}

void CCudaTransfer::RegisterTexture(CCpuObject* object)
{
	
	BufferDVPInfo* info = new BufferDVPInfo;
	
	DVPSysmemBufferDesc desc;
		
	desc.width = object->GetWidth();
	desc.height = object->GetHeight();
	desc.stride = object->GetStride();
	desc.size = object->GetSize();
	
	CUDAArrayParamsToDVPParams((CUarray_format)object->GetFormat(),object->GetNumChannels(),&desc.format, &desc.type);
	desc.bufAddr = object->GetVideoBuffer();
	
	DVP_SAFE_CALL(dvpCreateBuffer( &desc, &(info->handle) ));
	DVP_SAFE_CALL(dvpBindToCUDACtx( info->handle ));
	
	initSyncInfo(&(info->sysMemSyncInfo));
	initSyncInfo(&(info->gpuSyncInfo));
	
	info->currentChunk = 0;
	info->bTexture = true;
	_dvpInfoMap[object] = info;		
}


void CCudaTransfer::UnregisterTexture(CCpuObject* object)
{
	
	BufferDVPInfo* info = getBufferDVPInfo( object );
	DVP_SAFE_CALL(dvpUnbindFromCUDACtx(info->handle));
	DVP_SAFE_CALL(dvpDestroyBuffer(info->handle));
	DVP_SAFE_CALL(dvpFreeSyncObject(info->gpuSyncInfo.syncObj));
	DVP_SAFE_CALL(dvpFreeSyncObject(info->sysMemSyncInfo.syncObj));
		
	free((void*)(info->gpuSyncInfo.semOrg));
	free((void*)(info->sysMemSyncInfo.semOrg));
	_dvpInfoMap.erase(object);
	delete info;

}
void CCudaTransfer::UnregisterBuffer(CCpuObject* object)
{
	
	BufferDVPInfo* info = getBufferDVPInfo( object );
	DVP_SAFE_CALL(dvpUnbindFromCUDACtx(info->handle));
	DVP_SAFE_CALL(dvpDestroyBuffer(info->handle));
	DVP_SAFE_CALL(dvpFreeSyncObject(info->gpuSyncInfo.syncObj));
	DVP_SAFE_CALL(dvpFreeSyncObject(info->sysMemSyncInfo.syncObj));
		
	free((void*)(info->gpuSyncInfo.semOrg));
	free((void*)(info->sysMemSyncInfo.semOrg));
	_dvpInfoMap.erase(object);
	delete info;
}

float CCudaTransfer::GetCardToGpuTime( CCudaObject* object) const
{
	TimeInfo *info = getTimeInfo(object);
	if(info == 0)
	{
		return 0;
	}
	return info->cardToGpuTime*1000;
}

float CCudaTransfer::GetGpuToCardTime(CCudaObject* object) const
{
	TimeInfo *info = getTimeInfo(object);
	if(info == 0)
	{
		return 0;
	}
	return info->gpuToCardTime*1000;
}

TimeInfo* CCudaTransfer::getTimeInfo(CCudaObject* object) const
{
	map<CCudaObject*, TimeInfo*>::iterator itr = _bufferTimeInfoMap.find(object);
	if( itr == _bufferTimeInfoMap.end() )
	{
		assert(false);
		return 0;
	}
	
	return itr->second;
}

DVPBufferHandle CCudaTransfer::getDVPHandleForObject(CCudaObject* object) const
{
	map<CCudaObject*, DVPBufferHandle>::iterator itr = _bufferHandleMap.find(object);
	if( itr == _bufferHandleMap.end() )
	{
		assert(false);
		return 0;
	}
	
	return itr->second;
}


void CCudaTransfer::copyNextChunkCPUToGPU(CCudaObject* object, CCpuObject *sysmem) const
{
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	BufferDVPInfo* info = getBufferDVPInfo( sysmem );
	
	if(info->currentChunk == 0)
	{
		// Make sure the rendering API is finished using the buffer and block further usage
		DVP_SAFE_CALL(dvpMapBufferWaitDVP(dvpHandle));

#ifdef TIME_MEASUREMENTS		
		//TimeInfo *timeinfo = getTimeInfo(object);
		//glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&timeinfo->sysMemToGpuStart);
		//assert(glGetError() == GL_NO_ERROR);
#endif

	}
	info->sysMemSyncInfo.acquireValue++;
	info->gpuSyncInfo.releaseValue++;
	if(info->bTexture)
	{
		const uint32_t numLinesPerCopy = (uint32_t)((float)object->GetHeight()/(float)_numChunks);
		uint32_t copiedLines = info->currentChunk*numLinesPerCopy;
		
		// Initiate the system memory to GPU copy

		uint32_t linesRemaining = object->GetHeight()-copiedLines;
		uint32_t linesToCopy = (linesRemaining > numLinesPerCopy ? numLinesPerCopy : linesRemaining);
			

		
		DVP_SAFE_CALL(
			dvpMemcpyLined(
			info->handle,
			info->sysMemSyncInfo.syncObj,
			info->sysMemSyncInfo.acquireValue,
			DVP_TIMEOUT_IGNORED,
			dvpHandle,
			info->gpuSyncInfo.syncObj,
			info->gpuSyncInfo.releaseValue,
			copiedLines,
			linesToCopy));
	}
	else
	{
		uint32_t chunkSize = object->GetSize()/_numChunks;
		uint32_t copiedSize = info->currentChunk*chunkSize;
		uint32_t copiedChunkSize = (object->GetSize()-copiedSize > chunkSize ? chunkSize : object->GetSize()-copiedSize);

		DVP_SAFE_CALL(dvpMemcpy(		info->handle,
			info->sysMemSyncInfo.syncObj,
			info->sysMemSyncInfo.acquireValue,
			DVP_TIMEOUT_IGNORED,
			dvpHandle,
			info->gpuSyncInfo.syncObj,
			info->gpuSyncInfo.releaseValue,copiedSize,copiedSize,					   
					   copiedChunkSize));
					

	}
		
	info->currentChunk++;
	
	if(info->currentChunk == _numChunks)
	{
		DVP_SAFE_CALL(dvpMapBufferEndDVP(dvpHandle));
		info->currentChunk = 0;
	}
}
void CCudaTransfer::copyCPUToGPU(CCudaObject* object, CCpuObject *sysmem) const
{
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	BufferDVPInfo* info = getBufferDVPInfo( sysmem );
	
	// Make sure the rendering API is finished using the buffer and block further usage
	DVP_SAFE_CALL(dvpMapBufferWaitDVP(dvpHandle));

#ifdef TIME_MEASUREMENTS
	//TimeInfo *timeinfo = getTimeInfo(object);	
	//glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&timeinfo->sysMemToGpuStart);
	//assert(glGetError() == GL_NO_ERROR);
#endif


	// Initiate the system memory to GPU copy
	
	info->sysMemSyncInfo.acquireValue++;
	info->gpuSyncInfo.releaseValue++;
	if(info->bTexture)
	{	
		DVP_SAFE_CALL(
			dvpMemcpyLined(
			info->handle,
			info->sysMemSyncInfo.syncObj,
			info->sysMemSyncInfo.acquireValue,
			DVP_TIMEOUT_IGNORED,
			dvpHandle,
			info->gpuSyncInfo.syncObj,
			info->gpuSyncInfo.releaseValue,
			0,
			object->GetHeight()));
	}	
	else
	{
		DVP_SAFE_CALL(dvpMemcpy(		info->handle,
			info->sysMemSyncInfo.syncObj,
			info->sysMemSyncInfo.acquireValue,
			DVP_TIMEOUT_IGNORED,
			dvpHandle,
			info->gpuSyncInfo.syncObj,
			info->gpuSyncInfo.releaseValue,0,0,					   
					   object->GetSize()));
	}
	//while (MEM_RD32(info->gpuSyncInfo.sem) < info->gpuSyncInfo.acquireValue) {};
	DVP_SAFE_CALL(dvpMapBufferEndDVP(dvpHandle));	
	
}

void CCudaTransfer::copyNextChunkGPUToCPU(CCudaObject* object, CCpuObject *sysmem) const
{
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	BufferDVPInfo* info = getBufferDVPInfo( sysmem );
	if(info->currentChunk == 0)
	{
		// Make sure the rendering API is finished using the buffer and block further usage
		DVP_SAFE_CALL(dvpMapBufferWaitDVP(dvpHandle));


#ifdef TIME_MEASUREMENTS
		TimeInfo *timeinfo = getTimeInfo(object);
		DVP_SAFE_CALL(dvpSyncObjCompletion(info->gpuSyncInfo.syncObj,&timeinfo->gpuToSysMemEnd));
		timeinfo->gpuToCardTime = (timeinfo->sysMemToCardEnd - timeinfo->gpuToSysMemStart)*.000000001;
		//glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&timeinfo->gpuToSysMemStart);
		//assert(glGetError() == GL_NO_ERROR);
#endif

	}

	info->gpuSyncInfo.releaseValue++;
	if(info->bTexture)
	{	
		const uint32_t numLinesPerCopy = (uint32_t)((float)object->GetHeight()/(float)_numChunks);
		uint32_t copiedLines = info->currentChunk*numLinesPerCopy;
		
		// Initiate the GPU to system memory copy

		uint32_t linesRemaining = object->GetHeight()-copiedLines;
		uint32_t linesToCopy = (linesRemaining > numLinesPerCopy ? numLinesPerCopy : linesRemaining);
	
		DVP_SAFE_CALL(
				dvpMemcpyLined(
					dvpHandle,
					info->sysMemSyncInfo.syncObj,
					info->sysMemSyncInfo.acquireValue,
					DVP_TIMEOUT_IGNORED,
					info->handle,
					info->gpuSyncInfo.syncObj,
					info->gpuSyncInfo.releaseValue,
					copiedLines,
					linesToCopy));
	}
	else
	{
		uint32_t chunkSize = object->GetSize()/_numChunks;
		uint32_t copiedSize = info->currentChunk*chunkSize;
		uint32_t copiedChunkSize = (object->GetSize()-copiedSize > chunkSize ? chunkSize : object->GetSize()-copiedSize);


		DVP_SAFE_CALL(
				dvpMemcpy(
					dvpHandle,
					info->sysMemSyncInfo.syncObj,
					info->sysMemSyncInfo.acquireValue,
					DVP_TIMEOUT_IGNORED,
					info->handle,
					info->gpuSyncInfo.syncObj,
					info->gpuSyncInfo.releaseValue,
					copiedSize,
					copiedSize,					   
					copiedChunkSize));
		

	}
	info->sysMemSyncInfo.acquireValue++;
	info->currentChunk++;
	if(info->currentChunk == _numChunks)
	{
		DVP_SAFE_CALL(dvpMapBufferEndDVP(dvpHandle));
		info->currentChunk = 0;
	}

}
void CCudaTransfer::copyGPUToCPU(CCudaObject* object, CCpuObject *sysmem) const
{
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	BufferDVPInfo* info = getBufferDVPInfo( sysmem );
	
    
	// Make sure the rendering API is finished using the buffer and block further usage
    DVP_SAFE_CALL(dvpMapBufferWaitDVP(dvpHandle));

#ifdef TIME_MEASUREMENTS
	TimeInfo *timeinfo = getTimeInfo(object);
	DVP_SAFE_CALL(dvpSyncObjCompletion(info->gpuSyncInfo.syncObj,&timeinfo->gpuToSysMemEnd));
	timeinfo->gpuToCardTime = (timeinfo->sysMemToCardEnd - timeinfo->gpuToSysMemStart)*.000000001;
	//glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&timeinfo->gpuToSysMemStart);
	//assert(glGetError() == GL_NO_ERROR);	
#endif

	// Initiate the GPU to system memory copy
 	
    info->gpuSyncInfo.releaseValue++;
	if(info->bTexture)
	{	
		DVP_SAFE_CALL(
			dvpMemcpyLined(
				dvpHandle,
				info->sysMemSyncInfo.syncObj,
				info->sysMemSyncInfo.acquireValue,
				DVP_TIMEOUT_IGNORED,
				info->handle,
				info->gpuSyncInfo.syncObj,
				info->gpuSyncInfo.releaseValue,
				0,
				object->GetHeight()));
	}
	else
	{
		DVP_SAFE_CALL(
			dvpMemcpy(
				dvpHandle,
				info->sysMemSyncInfo.syncObj,
				info->sysMemSyncInfo.acquireValue,
				DVP_TIMEOUT_IGNORED,
				info->handle,
				info->gpuSyncInfo.syncObj,
				info->gpuSyncInfo.releaseValue,
				0, 0,
				object->GetSize()));
	}
	info->sysMemSyncInfo.acquireValue++;
	
    DVP_SAFE_CALL(dvpMapBufferEndDVP(dvpHandle));    
}

void CCudaTransfer::BeforeRecordTransfer(CCudaObject* object, CCpuObject *sysmem) const
{
	// Before TransferWithAutoCirculate that records to main memory,
	// we have to wait for any DMA from main memory to the GPU that
	// might need that same piece of main memory.
	BufferDVPInfo* info = getBufferDVPInfo( sysmem );
	if ( info->gpuSyncInfo.acquireValue)
	{

		//while (MEM_RD32(info->gpuSyncInfo.sem) < info->gpuSyncInfo.acquireValue) {};
		DVP_SAFE_CALL(dvpSyncObjClientWaitPartial(
			info->gpuSyncInfo.syncObj, info->gpuSyncInfo.acquireValue, DVP_TIMEOUT_IGNORED));

#ifdef TIME_MEASUREMENTS
		if(info->currentChunk == 0)
		{
			TimeInfo *timeinfo = getTimeInfo(object);
			DVP_SAFE_CALL(dvpSyncObjCompletion(info->gpuSyncInfo.syncObj,&timeinfo->sysMemToGpuEnd));
			timeinfo->cardToGpuTime = (timeinfo->sysMemToGpuEnd - timeinfo->cardToSysMemStart)*.000000001;
			//glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&timeinfo->cardToSysMemStart);
			//assert(glGetError() == GL_NO_ERROR);
		}
#endif

	}
}

void CCudaTransfer::AfterRecordTransfer(CCudaObject* object, CCpuObject *sysmem) const
{
	// After TransferWithAutoCirculate call to record to main memory,
	// we have to signal that transfer is complete so that code that
	// waits for frame to complete can continue.
	BufferDVPInfo* info = getBufferDVPInfo( sysmem );

#ifdef TIME_MEASUREMENTS
	if ( info->gpuSyncInfo.acquireValue )
	{
		//TimeInfo *timeinfo = getTimeInfo(object);
		//glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&timeinfo->cardToSysMemEnd);
		//assert(glGetError() == GL_NO_ERROR);
	}
#endif

	info->sysMemSyncInfo.releaseValue++;
	info->gpuSyncInfo.acquireValue++;
	MEM_WR32(info->sysMemSyncInfo.sem, info->sysMemSyncInfo.releaseValue);

	// Also we copy within the current frame of the circular buffer,
	// from main memory to the texture.
	if(_numChunks > 1)
	{
		copyNextChunkCPUToGPU( object, sysmem); //this is a partial copy
	}
	else
	{
		copyCPUToGPU(object, sysmem); 
	}
	
}

void CCudaTransfer::BeforePlaybackTransfer(CCudaObject* object, CCpuObject *sysmem) const
{
	// Before TransferWithAutoCirculate to playback, we need to copy
	// the texture from the receiving texture to main memory.
	if(_numChunks > 1)
	{
		copyNextChunkGPUToCPU(object, sysmem); //this is a partial copy
	}
	else
	{
		copyGPUToCPU(object, sysmem);
	}
	
	// Then wait for the buffer to become available.
	BufferDVPInfo* info = getBufferDVPInfo( sysmem );
	if ( info->gpuSyncInfo.acquireValue)
	{
		//while (MEM_RD32(*(info->gpuSyncInfo.sem)) < info->gpuSyncInfo.acquireValue) {};
		DVP_SAFE_CALL(dvpSyncObjClientWaitPartial(
			info->gpuSyncInfo.syncObj, info->gpuSyncInfo.acquireValue, DVP_TIMEOUT_IGNORED));

#ifdef TIME_MEASUREMENTS
		if(info->currentChunk == 0)
		{
			//TimeInfo *timeinfo = getTimeInfo(object);
			//glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&timeinfo->sysMemToCardStart);
			//assert(glGetError() == GL_NO_ERROR);
		}
#endif

	}
	
}

void CCudaTransfer::AfterPlaybackTransfer(CCudaObject* object,CCpuObject *sysmem) const
{
	// After TransferWithAutoCirculate call to playback from main memory,
	// we have to signal that transfer is complete so that code that
	// waits for frame to complete can continue.
	BufferDVPInfo* info = getBufferDVPInfo( sysmem );

#ifdef TIME_MEASUREMENTS
	if ( info->gpuSyncInfo.acquireValue)
	{
		//TimeInfo *timeinfo = getTimeInfo(object);
		//glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&timeinfo->sysMemToCardEnd);
		//assert(glGetError() == GL_NO_ERROR);
	}
#endif

	info->sysMemSyncInfo.releaseValue++;
	info->gpuSyncInfo.acquireValue++;
	MEM_WR32(info->sysMemSyncInfo.sem, info->sysMemSyncInfo.releaseValue);
}

// Functions below to be called in this fashion:
// _dvpTransfer->AcquireTexture(texture);
// ...GL code that uses texture...
// _dvpTransfer->ReleaseTexture(texture);

void CCudaTransfer::AcquireObject(CCudaObject* object, CUstream stream) const
{
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	DVP_SAFE_CALL(dvpMapBufferWaitCUDAStream(dvpHandle, stream));
}

void CCudaTransfer::ReleaseObject(CCudaObject* object, CUstream stream) const
{
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	DVP_SAFE_CALL(dvpMapBufferEndCUDAStream(dvpHandle, stream));
}


