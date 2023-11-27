/* SPDX-License-Identifier: MIT */


#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include "cpuObject.h"

void CCpuObject::Error(const std::string& message) const
{
	_errorList.Error(message);
}
CCpuObject::CCpuObject()
{
	_size = 0;
	_width = 0;
	_height = 0;
	_stride = 0;
	videoBufferUnaligned = NULL;
	videoBuffer = NULL;

}

CCpuObject::~CCpuObject()
{
	destroy();
}

void CCpuObject::Init(const CpuObjectDesc &desc)
{
	_useTexture = desc._useTexture;	
	if (_useTexture)
	{
		_width = desc._width;
		_height = desc._height;
		_format = desc._format;
		_type = desc._type;
		_numChannels = desc._numChannels;
		_stride = desc._stride;
		_stride += desc._strideAlignment-1;
		_stride &= ~(desc._strideAlignment-1);
		_size = _height*_stride;
	}
	else
	{
		_size = desc._size;

	}
	videoBufferUnaligned = new uint8_t[_size + desc._addressAlignment - 1];
	uint64_t val = (uint64_t)(videoBufferUnaligned);
	val += desc._addressAlignment-1;
	val &= ~((uint64_t)desc._addressAlignment-1);
	videoBuffer = (uint8_t*) val;
}

void CCpuObject::destroy()
{
	if(videoBufferUnaligned)
	{
		delete [] videoBufferUnaligned;
		videoBufferUnaligned = NULL;
		videoBuffer = NULL;
	}
}
CErrorList& CCpuObject::GetErrorList() const
{
	return _errorList;
}

uint32_t CCpuObject::GetSize() const
{
	return _size;
}

uint32_t CCpuObject::GetWidth() const
{
	return _width;

}
uint32_t CCpuObject::GetHeight() const
{
	return _height;

}
uint32_t CCpuObject::GetStride() const
{
	return _stride;

}

uint32_t CCpuObject::GetFormat() const
{
	return _format;

}

uint32_t CCpuObject::GetType() const
{
	return _type;

}
uint32_t CCpuObject::GetNumChannels() const
{
	return _numChannels;

}

uint8_t *CCpuObject::GetVideoBuffer() const
{
	return videoBuffer;

}

