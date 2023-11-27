/* SPDX-License-Identifier: MIT */
//
// Copyright (C) 2012 AJA Video Systems, Inc.
// Proprietary and Confidential information.
//
#ifndef _GPUCIRCULARBUFFER_H
#define _GPUCIRCULARBUFFER_H

#include "ajabase/common/circularbuffer.h"

#include "gpuObject.h"

template<class T>
struct AVGpuBuffer {
	T*						gpuObject;
	uint32_t*					audioBuffer;
	uint32_t					audioBufferSize;
	uint32_t					audioRecordSize;
};

template<class T>
class CGpuCircularBuffer
{
public:
	CGpuCircularBuffer();
	virtual ~CGpuCircularBuffer();
	
	void Init(int numObjects, T* gpuObjects, bool withAudio);
	void Abort();
	
	AVGpuBuffer<T>* StartProduceNextBuffer();
	void EndProduceNextBuffer();
	AVGpuBuffer<T>* StartConsumeNextBuffer();
	void EndConsumeNextBuffer();
	bool IsEmpty();

	AVGpuBuffer<T>* _avGpuBuffers;
private:
	uint32_t _numFrames;
	bool _abort;
	AJACircularBuffer<AVGpuBuffer<T>*> _avCircularBuffer;
	
};

#endif 

