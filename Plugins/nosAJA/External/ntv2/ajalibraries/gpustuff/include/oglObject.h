/* SPDX-License-Identifier: MIT */

#ifndef _OGL_OBJECT
#define _OGL_OBJECT

#include <stdio.h>	// fbo.h below uses printf

#include "gpuObject.h"
#include "gl/glew.h"
#include "gl/gl.h"
#include "fbo.h"
#include "export_gpu.h"

void checkOglError();

GPU_EXPORT int oglFormatToBytes(GLenum in_format, GLenum type);
GPU_EXPORT int oglTypeToFboFormat(GLenum type);


class COglObject : public IGpuObject
{
public:
	GPU_EXPORT COglObject();	
	GPU_EXPORT virtual ~COglObject();
	GPU_EXPORT virtual void Init(const GpuObjectDesc &desc);
	GPU_EXPORT GLuint GetTextureHandle() const;
	GPU_EXPORT GLuint GetBufferHandle() const;	
	// Call this function to start rendering to
	// texture.  Subsequent draw calls in OpenGL
	// will draw into the texture.
	GPU_EXPORT void Begin();
	
	// Call this function to return rendering
	// target to the screen.
	GPU_EXPORT void End();

private:

	void destroy();
	GLuint _textureHandle;
	GLuint _bufferHandle;
	CFBO	_fbo;

};

#endif

