/* SPDX-License-Identifier: MIT */

#include "cudaObject.h"


#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

void failCuda(CUresult hr)
{
    fprintf(stderr, "CUDA Failed with status %X\n", hr);
}


int cudaArrayFormatToBytes(CUarray_format format)
{
	int numBytes;
		switch(format)
		{
		case CU_AD_FORMAT_UNSIGNED_INT8:
		case CU_AD_FORMAT_SIGNED_INT8:
			numBytes  = 1;						
			break;
		case CU_AD_FORMAT_UNSIGNED_INT16:
		case CU_AD_FORMAT_SIGNED_INT16:
		case CU_AD_FORMAT_HALF:
			numBytes  = 2;						
			break;
		case CU_AD_FORMAT_UNSIGNED_INT32:
		case CU_AD_FORMAT_SIGNED_INT32:
		case CU_AD_FORMAT_FLOAT:
			numBytes  = 4;			
			break;

		default:
			numBytes  = 1;
						
		}
	return numBytes;
}

CUarray CCudaObject::GetTextureHandle() const
{
	return _arrayHandle;
}
CUdeviceptr CCudaObject::GetBufferHandle() const
{
	return _bufferHandle;
}
CCudaObject::CCudaObject()
{
}

CCudaObject::~CCudaObject()
{
	destroy();
}

void CCudaObject::Init(const GpuObjectDesc &desc)
{
	_useTexture = desc._useTexture;	
	_useRenderToTexture = desc._useRenderToTexture;
	if(_useTexture)
	{
		_width = desc._width;
		_height = desc._height;
		_format = desc._format;
		_numChannels = desc._numChannels;		
		//ignore the incoming size and calulate your own based on the optimal GPU stride
		_stride = _width*_numChannels*cudaArrayFormatToBytes((CUarray_format)_format);		
		_size = _height*_stride;

		CUDA_ARRAY_DESCRIPTOR desc;
		desc.Format = (CUarray_format) _format;//CU_AD_FORMAT_UNSIGNED_INT8;
		desc.Height = _height;
		desc.Width = _width;
		desc.NumChannels = _numChannels;//4

		CUCHK(cuArrayCreate(&_arrayHandle, &desc));
		
	}
	else
	{
		_size = desc._size;
		CUCHK(cuMemAlloc(&_bufferHandle,_size));
	}	
}

void CCudaObject::destroy()
{
	if(_useTexture)
	{
		cuArrayDestroy(_arrayHandle);
	}
	else
	{
		cuMemFree(_bufferHandle);
	}

}

