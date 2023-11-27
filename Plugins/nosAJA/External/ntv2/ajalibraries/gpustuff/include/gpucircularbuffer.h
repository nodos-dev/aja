/* SPDX-License-Identifier: MIT */
//
// Copyright (C) 2012 AJA Video Systems, Inc.
// Proprietary and Confidential information.
//
#ifndef _GPU_CIRCULAR_BUFFER
#define _GPU_CIRCULAR_BUFFER

#include "gpucircularbufferInterface.h"

#include <assert.h>

template<class T>
CGpuCircularBuffer<T>::CGpuCircularBuffer() :
	_abort(false), _avGpuBuffers(NULL), _numFrames(0)
{
}
template<class T>
CGpuCircularBuffer<T>::~CGpuCircularBuffer()
{
	if ( _avGpuBuffers )
	{
		for ( uint32_t i=0; i<_numFrames; i++ )
		{
			if(_avGpuBuffers[i].audioBuffer)
				delete [] _avGpuBuffers[i].audioBuffer;
		}
		
		if ( _avGpuBuffers )
			delete [] _avGpuBuffers;
		_avGpuBuffers = NULL;
	}
}
template<class T>
void CGpuCircularBuffer<T>::Init(int numObjects, T* gpuObjects, bool withAudio)
{
	assert( _numFrames == 0 );
	assert( numObjects > 0 );

	
	_numFrames = numObjects;
	
	_avCircularBuffer.SetAbortFlag(&_abort);
	
	_avGpuBuffers = new AVGpuBuffer<T>[_numFrames];
	memset(_avGpuBuffers, 0, sizeof(AVGpuBuffer<T>)*_numFrames);
	
	for ( uint32_t i=0; i<_numFrames; i++ )
	{	
		_avGpuBuffers[i].gpuObject = &gpuObjects[i];			
		
		if ( withAudio )
		{
			//_avGpuBuffers[i].audioBuffer = new uint32_t[NTV2_AUDIOSIZE_MAX/sizeof(uint32_t)];
			//_avGpuBuffers[i].audioBufferSize = NTV2_AUDIOSIZE_MAX;			// this will change each frame
		} 
		else
		{
			_avGpuBuffers[i].audioBuffer = NULL;
			_avGpuBuffers[i].audioBufferSize = 0; // this will change each frame
		}
		_avCircularBuffer.Add(&_avGpuBuffers[i]);
		
	}
}

template<class T>
void CGpuCircularBuffer<T>::Abort()
{
	_abort = true;
}
template<class T>
AVGpuBuffer<T>* CGpuCircularBuffer<T>::StartProduceNextBuffer()
{
	return _avCircularBuffer.StartProduceNextBuffer();
}
template<class T>
void CGpuCircularBuffer<T>::EndProduceNextBuffer()
{
	_avCircularBuffer.EndProduceNextBuffer();
}
template<class T>
AVGpuBuffer<T>* CGpuCircularBuffer<T>::StartConsumeNextBuffer()
{
	return _avCircularBuffer.StartConsumeNextBuffer();
}
template<class T>
void CGpuCircularBuffer<T>::EndConsumeNextBuffer()
{
	_avCircularBuffer.EndConsumeNextBuffer();
}

template<class T>
bool CGpuCircularBuffer<T>::IsEmpty()
{
	if(_avCircularBuffer.GetCircBufferCount())
		return false;
	return true;

}

#endif