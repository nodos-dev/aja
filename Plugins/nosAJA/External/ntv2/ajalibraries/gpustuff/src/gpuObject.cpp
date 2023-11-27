/* SPDX-License-Identifier: MIT */


#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include "gpuObject.h"

void IGpuObject::Error(const std::string& message) const
{
	_errorList.Error(message);
}
IGpuObject::IGpuObject()
{
	_size = 0;
	_width = 0;
	_height = 0;
	_stride = 0;

}
CErrorList& IGpuObject::GetErrorList() const
{
	return _errorList;
}

uint32_t IGpuObject::GetSize() const
{
	return _size;
}

uint32_t IGpuObject::GetWidth() const
{
	return _width;

}
uint32_t IGpuObject::GetHeight() const
{
	return _height;

}
uint32_t IGpuObject::GetStride() const
{
	return _stride;

}
uint32_t IGpuObject::GetFormat() const
{
	return _format;

}

uint32_t IGpuObject::GetInternalFormat() const
{
	return _internalformat;

}

uint32_t IGpuObject::GetType() const
{
	return _type;

}
uint32_t IGpuObject::GetNumChannels() const
{
	return _numChannels;

}


