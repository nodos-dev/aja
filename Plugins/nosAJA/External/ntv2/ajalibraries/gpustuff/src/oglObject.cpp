/* SPDX-License-Identifier: MIT */

#include "gl/glew.h"
#include "oglObject.h"


#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>




void checkOglError()
{
	GLuint error = glGetError();

	if( error != GL_NO_ERROR )
	{
#if _DEBUG
		char* errString = (char*)gluErrorString(error);
		fprintf(stderr,"%s",errString);
		assert(!"GL_ERROR");
#else
		exit(0);
#endif
	}
}
int oglTypeToFboFormat(GLenum type)
{
	int numBits;
	switch(type)
	{ 
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			numBits = 8;
			break;
		case GL_HALF_FLOAT:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
			numBits = 16;
			break; 
		case GL_FLOAT:
		case GL_INT:
		case GL_UNSIGNED_INT: 
			numBits = 32;
			break;
		case GL_UNSIGNED_BYTE_3_3_2:
		case GL_UNSIGNED_BYTE_2_3_3_REV:
		case GL_UNSIGNED_INT_8_8_8_8:
		case GL_UNSIGNED_INT_8_8_8_8_REV:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_5_6_5_REV:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_1_5_5_5_REV: 
			numBits = 8;
			break;
		case GL_UNSIGNED_INT_10_10_10_2:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			numBits = 10;
			break;
		default:
			numBits = 4; 
	}
	return numBits;

}
int oglFormatToBytes(GLenum in_format, GLenum type)
{
	int numChannels;
	switch(in_format)
	{
		case GL_RGBA:
		case GL_BGRA:
		case GL_RGBA_INTEGER_EXT:
		case GL_BGRA_INTEGER_EXT:
			numChannels = 4;
			break;
		case GL_DEPTH_COMPONENT:
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
		case GL_LUMINANCE:
		case GL_RED_INTEGER_EXT:
		case GL_GREEN_INTEGER_EXT:
		case GL_BLUE_INTEGER_EXT:
		case GL_ALPHA_INTEGER_EXT:
		case GL_LUMINANCE_INTEGER_EXT:
			numChannels = 1;
			break;
		case GL_RGB:
		case GL_BGR:
		case GL_RGB_INTEGER_EXT:
		case GL_BGR_INTEGER_EXT:
			numChannels = 3;
			break;
		case GL_LUMINANCE_ALPHA:
		case GL_LUMINANCE_ALPHA_INTEGER_EXT:
			numChannels = 2;
			break;
		default:
			numChannels = 4; 
	}
	int numBytes;
	switch(type)
	{ 
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			numBytes = numChannels;
			break;
		case GL_HALF_FLOAT:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
			numBytes = 2*numChannels;
			break; 
		case GL_FLOAT:
		case GL_INT:
		case GL_UNSIGNED_INT: 
			numBytes = 4*numChannels;
			break;
		case GL_UNSIGNED_BYTE_3_3_2:
		case GL_UNSIGNED_BYTE_2_3_3_REV:
			numBytes = 1;
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_5_6_5_REV:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_1_5_5_5_REV: 
			numBytes = 2;
			break;
		case GL_UNSIGNED_INT_8_8_8_8:
		case GL_UNSIGNED_INT_8_8_8_8_REV:
		case GL_UNSIGNED_INT_10_10_10_2:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			numBytes = 4;
			break;
		default:
			numBytes = 4; 
	}
	return numBytes;
}


GLuint COglObject::GetTextureHandle() const
{
	return _textureHandle;
}
GLuint COglObject::GetBufferHandle() const
{
	return _bufferHandle;
}
COglObject::COglObject()
{

}

COglObject::~COglObject()
{
	destroy();
}

void COglObject::Init(const GpuObjectDesc &desc)
{
	_useTexture = desc._useTexture;	
	_useRenderToTexture = desc._useRenderToTexture;
	
	if(_useTexture)
	{
		
		_width = desc._width;
		_height = desc._height;
		_internalformat = desc._internalformat;
		_format = desc._format;
		_type = desc._type;
		//ignore the incoming size and calulate your own based on the optimal GPU stride
		int numBytesPerPixel = oglFormatToBytes(_format,_type);
		_stride = _width*numBytesPerPixel;
		_size = _height*_stride;

		glGenTextures(1, &_textureHandle);		
		glBindTexture(GL_TEXTURE_2D, _textureHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, _internalformat,_width,_height, 0, _format, _type, NULL);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
		glBindTexture(GL_TEXTURE_2D, 0);
		checkOglError();
		if(_useRenderToTexture)
		{
			// Init glew
			glewInit();
			if (!glewIsSupported("GL_VERSION_2_0 "
				"GL_ARB_pixel_buffer_object "
				"GL_EXT_framebuffer_object "
				)) {
					fprintf(stderr, "Support for necessary OpenGL extensions missing.\n");
			}
			_fbo.create(_width,_height, oglTypeToFboFormat(_type), 1, GL_TRUE, GL_TRUE, _textureHandle);		
			_fbo.unbind();
		}
	}
	else
	{
		_size = desc._size;
		glGenBuffers(1,&_bufferHandle);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _bufferHandle);
		assert(glGetError() == GL_NO_ERROR);		
		glBufferData(GL_PIXEL_UNPACK_BUFFER, _size, NULL, GL_STREAM_COPY);
	}
	//initSysmem(cpuAddressAlignment);
}

void COglObject::destroy()
{
	if(_useTexture)
	{
		glDeleteTextures(1, &_textureHandle);	
		if(_useRenderToTexture)
		{
			_fbo.destroy();
		}
	}
	else
	{
		glDeleteBuffers(1, &_bufferHandle);
	}

}

void COglObject::Begin()
{
	if(_useRenderToTexture)
	{
		_fbo.bind(_width, _height);
	}
}

void COglObject::End()
{
	if(_useRenderToTexture)
	{
		_fbo.unbind();
	}
}
