/* SPDX-License-Identifier: MIT */
/*
  This software is provided by AJA Video, Inc. "AS IS"
  with no express or implied warranties.
*/

#include "gl/glew.h"
#include <DVPAPI.h>
#include <dvpapi_gl.h>
#include <assert.h>
#include <stdlib.h>	// For exit
#include <string>
#include <string.h>	// For memset
#include <map>
#include "oglTransfer.h"

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

class COglTransfer : public IOglTransfer{


public:
	COglTransfer();
	virtual ~COglTransfer();
	
	virtual bool Init();
	virtual void Destroy();

	virtual void BeginTransfers(); //this has to be called in the thread where the transfers will be performed
	virtual void EndTransfers();//this has to be called in the thread where the transfers will be performed
	
	//requires a current GPU context
	virtual void RegisterTexture(COglObject* object);
	//requires a current GPU context
	virtual void RegisterBuffer(COglObject* object);


	virtual void UnregisterTexture(COglObject* object);
	virtual void UnregisterBuffer(COglObject* object);
	
	//requires a current GPU context
	virtual void RegisterTexture(CCpuObject* object);
	//requires a current GPU context
	virtual void RegisterBuffer(CCpuObject* object);


	virtual void UnregisterTexture(CCpuObject* object);
	virtual void UnregisterBuffer(CCpuObject* object);

	virtual void BeforeRecordTransfer(COglObject* object, CCpuObject* sysmem) const;
	virtual void AfterRecordTransfer(COglObject* object, CCpuObject* sysmem) const;

	virtual void BeforePlaybackTransfer(COglObject* object, CCpuObject* sysmem) const;
	virtual void AfterPlaybackTransfer(COglObject* object, CCpuObject* sysmem) const;


	virtual void AcquireObject(COglObject* object) const;
	virtual void ReleaseObject(COglObject* object) const;

	virtual uint32_t GetNumChunks() const;
	virtual void SetNumChunks(uint32_t numChunks);	

	virtual float GetCardToGpuTime(COglObject* object) const;

	virtual float GetGpuToCardTime(COglObject* object) const;
	
	//requires a current GPU context
	virtual void GetGpuPreferredAllocationConstants(uint32_t *alocationalignment, uint32_t *stridealignment);
private:

	uint32_t _numChunks; //specifies the number of chunks used in the transfers. Used for overlapped GPU and Video I/O transfers

	mutable std::map<CCpuObject*, BufferDVPInfo*> _dvpInfoMap;
	mutable std::map<COglObject *, DVPBufferHandle> _bufferHandleMap;

	mutable std::map<COglObject *, TimeInfo*> _bufferTimeInfoMap;

	virtual void copyCPUToGPU(COglObject* object, CCpuObject* sysmem) const;
	virtual void copyGPUToCPU(COglObject* object, CCpuObject* sysmem) const;

	virtual void copyNextChunkCPUToGPU(COglObject* object, CCpuObject* sysmem) const;
	virtual void copyNextChunkGPUToCPU(COglObject* object, CCpuObject* sysmem) const;
	

	TimeInfo* getTimeInfo(COglObject* object) const;
	void initSyncInfo(SyncInfo *si) const;
	
	BufferDVPInfo* getBufferDVPInfo(CCpuObject* object) const;	
	DVPBufferHandle getDVPHandleForObject(COglObject* object) const;
	

};

IOglTransfer *CreateOglTransfer()
{
	return new COglTransfer();
}

static void fail(DVPStatus hr, char *str)
{
    //odprintf("DVP Failed with status %X\n", hr);
	printf("DVP Failed with status %X: %s\n", hr, str);
	exit(0);
}

/*odprintf("Fail on line %d\n", __LINE__); \ */
#define DVP_SAFE_CALL(cmd) { \
    DVPStatus hr = (cmd); \
	char _str[256]; \
    if (DVP_STATUS_OK != hr) { \
        sprintf(_str, "Fail on line %d\n", __LINE__); \
        fail(hr, _str); \
    } \
}

#define MEM_RD32(a) (*(const volatile unsigned int *)(a))
#define MEM_WR32(a, d) do { *(volatile unsigned int *)(a) = (d); } while (0)


void OGLTextureParamsToDVPParams(GLenum in_format, GLenum in_type, DVPBufferFormats *out_format, DVPBufferTypes *out_type)
{
	switch(in_type)
	{
		case GL_UNSIGNED_BYTE:
			*out_type = DVP_UNSIGNED_BYTE;
			break;
		case GL_BYTE:
			*out_type = DVP_BYTE;
			break;
		case GL_UNSIGNED_BYTE_3_3_2:
			*out_type = DVP_UNSIGNED_BYTE_3_3_2;
			break;
		case GL_UNSIGNED_BYTE_2_3_3_REV:
			*out_type = DVP_UNSIGNED_BYTE_3_3_2;
			break;
		case GL_UNSIGNED_SHORT:
			*out_type = DVP_UNSIGNED_SHORT;
			break;
		case GL_SHORT:
			*out_type = DVP_SHORT;
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
			*out_type = DVP_UNSIGNED_SHORT_5_6_5;
			break;
		case GL_UNSIGNED_SHORT_5_6_5_REV:
			*out_type = DVP_UNSIGNED_SHORT_5_6_5_REV;
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
			*out_type = DVP_UNSIGNED_SHORT_4_4_4_4;
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
			*out_type = DVP_UNSIGNED_SHORT_4_4_4_4_REV;
			break;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			*out_type = DVP_UNSIGNED_SHORT_5_5_5_1;
			break;
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
			*out_type = DVP_UNSIGNED_SHORT_1_5_5_5_REV;
			break;
		case GL_HALF_FLOAT:
			*out_type = DVP_HALF_FLOAT;
			break;
		case GL_UNSIGNED_INT:
			*out_type = DVP_UNSIGNED_INT;
			break;
		case GL_INT:
			*out_type = DVP_INT;
			break;
		case GL_FLOAT:
			*out_type = DVP_FLOAT;
			break;
		case GL_UNSIGNED_INT_8_8_8_8:
			*out_type = DVP_UNSIGNED_INT_8_8_8_8;
			break;
		case GL_UNSIGNED_INT_8_8_8_8_REV:
			*out_type = DVP_UNSIGNED_INT_8_8_8_8_REV;
			break;
		case GL_UNSIGNED_INT_10_10_10_2:
			*out_type = DVP_UNSIGNED_INT_10_10_10_2;
			break;

		case GL_UNSIGNED_INT_2_10_10_10_REV:
			*out_type = DVP_UNSIGNED_INT_2_10_10_10_REV;
			break;
		default:
			*out_type = DVP_UNSIGNED_BYTE;		
	}
	switch(in_format)
	{
		case GL_DEPTH_COMPONENT:
			*out_format = DVP_DEPTH_COMPONENT;
			break;
		case GL_RGBA:
			*out_format = DVP_RGBA;
			break;
		case GL_BGRA:
			*out_format = DVP_BGRA;
			break;
		case GL_RED:
			*out_format = DVP_RED;
			break;
		case GL_GREEN:
			*out_format = DVP_GREEN;
			break;
		case GL_BLUE:
			*out_format = DVP_BLUE;
			break;
 		case GL_ALPHA:
			*out_format = DVP_ALPHA;
			break;
 		case GL_RGB:
			*out_format = DVP_RGB;
			break;
 		case GL_BGR:
			*out_format = DVP_BGR;
			break;
 		case GL_LUMINANCE:
			*out_format = DVP_LUMINANCE;
			break;
 		case GL_LUMINANCE_ALPHA:
			*out_format = DVP_LUMINANCE_ALPHA;
			break;
		case GL_RGB_INTEGER:
			*out_format = DVP_RGB_INTEGER;
			break;
 		case GL_BGR_INTEGER:
			*out_format = DVP_BGR_INTEGER;
			break;
		case GL_RGBA_INTEGER:
			*out_format = DVP_RGBA_INTEGER;
			break;
		case GL_BGRA_INTEGER:
			*out_format = DVP_BGRA_INTEGER;
			break;
		case GL_RED_INTEGER:
			*out_format = DVP_RED_INTEGER;
			break;
		case GL_GREEN_INTEGER:
			*out_format = DVP_GREEN_INTEGER;
			break;
		case GL_BLUE_INTEGER:
			*out_format = DVP_BLUE_INTEGER;
			break;
 		case GL_ALPHA_INTEGER:
			*out_format = DVP_ALPHA_INTEGER;
		default:
			*out_format = DVP_RGBA;		
	}
}

COglTransfer::COglTransfer() :
	_numChunks(1)
{		
	 
}

COglTransfer::~COglTransfer()
{
	
}

bool COglTransfer::Init()
{	
	DVP_SAFE_CALL(dvpInitGLContext(0));
	
	return true;
}

//requires a current GPU context
void COglTransfer::GetGpuPreferredAllocationConstants(uint32_t *alocationalignment, uint32_t *stridealignment)
{
	uint32_t _bufferAddrAlignment;		
	uint32_t _bufferGPUStrideAlignment;	 
	uint32_t _semaphoreAddrAlignment;	
	uint32_t _semaphoreAllocSize;		
	uint32_t _semaphorePayloadOffset;
	uint32_t _semaphorePayloadSize;
	DVP_SAFE_CALL(dvpGetRequiredConstantsGLCtx(&_bufferAddrAlignment,
		&_bufferGPUStrideAlignment,
		&_semaphoreAddrAlignment,
		&_semaphoreAllocSize,
		&_semaphorePayloadOffset,
		&_semaphorePayloadSize));
	*alocationalignment = _bufferAddrAlignment;
	*stridealignment = _bufferGPUStrideAlignment;
	

}

void COglTransfer::Destroy()
{
	DVP_SAFE_CALL(dvpCloseGLContext());
}


//dvpMemcpy functions must be encapsulated with the dvp begin and end calls
//for optimal performance, call these once per thread instead of every frame
//using the InitTransfers and DeinitTransfers methods.

void COglTransfer::BeginTransfers() 
{
	DVP_SAFE_CALL(dvpBegin());
}

void COglTransfer::EndTransfers()
{
	DVP_SAFE_CALL(dvpEnd());
}

uint32_t COglTransfer::GetNumChunks() const
{
	return _numChunks;

}
void COglTransfer::SetNumChunks(uint32_t numChunks)
{
	_numChunks = numChunks;
}


BufferDVPInfo* COglTransfer::getBufferDVPInfo(CCpuObject* sysmem) const
{
	
	map<CCpuObject*, BufferDVPInfo*>::iterator itr = _dvpInfoMap.find(sysmem);
	
	if( itr == _dvpInfoMap.end() )
	{
		return NULL;
	}
	else
		return itr->second;
}


void COglTransfer::initSyncInfo(SyncInfo *si) const
{
	uint32_t _bufferAddrAlignment;		
	uint32_t _bufferGPUStrideAlignment;	 
	uint32_t _semaphoreAddrAlignment;	
	uint32_t _semaphoreAllocSize;		
	uint32_t _semaphorePayloadOffset;
	uint32_t _semaphorePayloadSize;
	DVP_SAFE_CALL(dvpGetRequiredConstantsGLCtx(&_bufferAddrAlignment,
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
	uint64_t alignmentValue = _semaphoreAddrAlignment-1;
    val +=  alignmentValue;
    val &= ~alignmentValue;	// Must be 64 bits to avoid zeroing high order 32 bits
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

void COglTransfer::RegisterBuffer(COglObject* object)
{
	
	DVPBufferHandle dvpHandle;
	GLuint bufferHandle = object->GetBufferHandle();
	DVP_SAFE_CALL(dvpCreateGPUBufferGL(
		bufferHandle,
		&dvpHandle));
#ifdef TIME_MEASUREMENTS
	TimeInfo *timeInfo = new TimeInfo;
	memset(timeInfo, 0, sizeof(TimeInfo));
	_bufferTimeInfoMap[object] = timeInfo;
#endif
	_bufferHandleMap[object] = dvpHandle;
	
}

void COglTransfer::RegisterTexture(COglObject* object)
{
	
	DVPBufferHandle dvpHandle;
	GLuint textureHandle = object->GetTextureHandle();
	DVP_SAFE_CALL(dvpCreateGPUTextureGL(
		textureHandle,
		&dvpHandle));
#ifdef TIME_MEASUREMENTS
	TimeInfo *timeInfo = new TimeInfo;
	memset(timeInfo, 0, sizeof(TimeInfo));
	_bufferTimeInfoMap[object] = timeInfo;
#endif
	_bufferHandleMap[object] = dvpHandle;
}


void COglTransfer::UnregisterTexture(COglObject* object)
{
	
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	TimeInfo *timeinfo = getTimeInfo(object);
	DVP_SAFE_CALL(dvpFreeBuffer(dvpHandle));
	_bufferHandleMap.erase(object);
	_bufferTimeInfoMap.erase(object);
	delete	timeinfo;
}
void COglTransfer::UnregisterBuffer(COglObject* object)
{
	
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	TimeInfo *timeinfo = getTimeInfo(object);
	DVP_SAFE_CALL(dvpFreeBuffer(dvpHandle));
	_bufferHandleMap.erase(object);
	_bufferTimeInfoMap.erase(object);
	delete	timeinfo;
}


void COglTransfer::RegisterBuffer(CCpuObject* sysmem)
{
	
	BufferDVPInfo* info = new BufferDVPInfo;
	
	DVPSysmemBufferDesc desc;
	
	desc.size = sysmem->GetSize();
	
	desc.format = DVP_BUFFER;
	desc.type = DVP_UNSIGNED_BYTE;
	desc.bufAddr = sysmem->GetVideoBuffer();
	
	DVP_SAFE_CALL(dvpCreateBuffer( &desc, &(info->handle) ));
	DVP_SAFE_CALL(dvpBindToGLCtx( info->handle ));
	
	initSyncInfo(&(info->sysMemSyncInfo));
	initSyncInfo(&(info->gpuSyncInfo));
	
	info->currentChunk = 0;
	info->bTexture = false;
	_dvpInfoMap[sysmem] = info;			
}

void COglTransfer::RegisterTexture(CCpuObject* object)
{
	
	BufferDVPInfo* info = new BufferDVPInfo;
	
	DVPSysmemBufferDesc desc;
		
	desc.width = object->GetWidth();
	desc.height = object->GetHeight();
	desc.stride = object->GetStride();
	desc.size = object->GetSize();
	
	OGLTextureParamsToDVPParams((GLenum)object->GetFormat(),(GLenum)object->GetType(),&desc.format, &desc.type);
	desc.bufAddr = object->GetVideoBuffer();
	
	DVP_SAFE_CALL(dvpCreateBuffer( &desc, &(info->handle) ));
	DVP_SAFE_CALL(dvpBindToGLCtx( info->handle ));
	
	initSyncInfo(&(info->sysMemSyncInfo));
	initSyncInfo(&(info->gpuSyncInfo));
	
	info->currentChunk = 0;
	info->bTexture = true;
	_dvpInfoMap[object] = info;		
}


void COglTransfer::UnregisterTexture(CCpuObject* object)
{
	
	BufferDVPInfo* info = getBufferDVPInfo( object );
	DVP_SAFE_CALL(dvpUnbindFromGLCtx(info->handle));
	DVP_SAFE_CALL(dvpDestroyBuffer(info->handle));
	DVP_SAFE_CALL(dvpFreeSyncObject(info->gpuSyncInfo.syncObj));
	DVP_SAFE_CALL(dvpFreeSyncObject(info->sysMemSyncInfo.syncObj));
		
	free((void*)(info->gpuSyncInfo.semOrg));
	free((void*)(info->sysMemSyncInfo.semOrg));
	_dvpInfoMap.erase(object);
	delete info;

}
void COglTransfer::UnregisterBuffer(CCpuObject* object)
{
	
	BufferDVPInfo* info = getBufferDVPInfo( object );
	DVP_SAFE_CALL(dvpUnbindFromGLCtx(info->handle));
	DVP_SAFE_CALL(dvpDestroyBuffer(info->handle));
	DVP_SAFE_CALL(dvpFreeSyncObject(info->gpuSyncInfo.syncObj));
	DVP_SAFE_CALL(dvpFreeSyncObject(info->sysMemSyncInfo.syncObj));
		
	free((void*)(info->gpuSyncInfo.semOrg));
	free((void*)(info->sysMemSyncInfo.semOrg));
	_dvpInfoMap.erase(object);
	delete info;
}


float COglTransfer::GetCardToGpuTime( COglObject* object) const
{
	TimeInfo *info = getTimeInfo(object);
	if(info == 0)
	{
		return 0;
	}
	return info->cardToGpuTime*1000;
}

float COglTransfer::GetGpuToCardTime(COglObject* object) const
{
	TimeInfo *info = getTimeInfo(object);
	if(info == 0)
	{
		return 0;
	}
	return info->gpuToCardTime*1000;
}

TimeInfo* COglTransfer::getTimeInfo(COglObject* object) const
{
	map<COglObject*, TimeInfo*>::iterator itr = _bufferTimeInfoMap.find(object);
	if( itr == _bufferTimeInfoMap.end() )
	{
		assert(false);
		return 0;
	}
	
	return itr->second;
}

DVPBufferHandle COglTransfer::getDVPHandleForObject(COglObject* object) const
{
	map<COglObject*, DVPBufferHandle>::iterator itr = _bufferHandleMap.find(object);
	if( itr == _bufferHandleMap.end() )
	{
		assert(false);
		return 0;
	}
	
	return itr->second;
}

void COglTransfer::copyNextChunkCPUToGPU(COglObject* object, CCpuObject *sysmem) const
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
void COglTransfer::copyCPUToGPU(COglObject* object, CCpuObject *sysmem) const
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

void COglTransfer::copyNextChunkGPUToCPU(COglObject* object, CCpuObject *sysmem) const
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
		timeinfo->gpuToCardTime = float((timeinfo->sysMemToCardEnd - timeinfo->gpuToSysMemStart)*.000000001);
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
void COglTransfer::copyGPUToCPU(COglObject* object, CCpuObject *sysmem) const
{
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	BufferDVPInfo* info = getBufferDVPInfo( sysmem );
	
    
	// Make sure the rendering API is finished using the buffer and block further usage
    DVP_SAFE_CALL(dvpMapBufferWaitDVP(dvpHandle));

#ifdef TIME_MEASUREMENTS
	TimeInfo *timeinfo = getTimeInfo(object);
	DVP_SAFE_CALL(dvpSyncObjCompletion(info->gpuSyncInfo.syncObj,&timeinfo->gpuToSysMemEnd));
	timeinfo->gpuToCardTime = float((timeinfo->sysMemToCardEnd - timeinfo->gpuToSysMemStart)*.000000001);
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

void COglTransfer::BeforeRecordTransfer(COglObject* object, CCpuObject *sysmem) const
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
			timeinfo->cardToGpuTime = float((timeinfo->sysMemToGpuEnd - timeinfo->cardToSysMemStart)*.000000001);
			//glGetInteger64v(GL_CURRENT_TIME_NV,(GLint64 *)&timeinfo->cardToSysMemStart);
			//assert(glGetError() == GL_NO_ERROR);
		}
#endif

	}
}

void COglTransfer::AfterRecordTransfer(COglObject* object, CCpuObject *sysmem) const
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

void COglTransfer::BeforePlaybackTransfer(COglObject* object, CCpuObject *sysmem) const
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

void COglTransfer::AfterPlaybackTransfer(COglObject* object,CCpuObject *sysmem) const
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

void COglTransfer::AcquireObject(COglObject* object) const
{
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	DVP_SAFE_CALL(dvpMapBufferWaitAPI(dvpHandle));
}

void COglTransfer::ReleaseObject(COglObject* object) const
{
	DVPBufferHandle dvpHandle = getDVPHandleForObject(object);
	DVP_SAFE_CALL(dvpMapBufferEndAPI(dvpHandle));
}


