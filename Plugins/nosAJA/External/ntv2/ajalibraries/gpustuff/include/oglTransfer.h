/* SPDX-License-Identifier: MIT */
/*
  This software is provided by AJA Video, Inc. "AS IS"
  with no express or implied warranties.
*/

#ifndef _OGL_TRANSFER_
#define _OGL_TRANSFER_
#include "oglObject.h"
#include "gpuTransferInterface.h"
#include "export_gpu.h"

class IOglTransfer : public IGpuTransfer<COglObject>
{
public:
	/* In asynchronous transfer schemes where, subclasses override AcquireTexture
	   to wait (or have opengl operations wait) until the texture is available to use. */
	virtual void AcquireObject(COglObject* object) const = 0;
	
	/* In asynchronous transfer schemes where, subclasses override ReleaseTexture
	   to signal that OpenGL is done using the texture. */
	virtual void ReleaseObject(COglObject* object) const = 0;

};

GPU_EXPORT  IOglTransfer *CreateOglTransfer();

#endif