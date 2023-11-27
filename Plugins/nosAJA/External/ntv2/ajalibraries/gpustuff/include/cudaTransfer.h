/* SPDX-License-Identifier: MIT */
/*
  This software is provided by AJA Video, Inc. "AS IS"
  with no express or implied warranties.
*/

#ifndef _CUDA_TRANSFER_
#define _CUDA_TRANSFER_
#include "export_gpu.h"
#include "cudaObject.h"
#include "gpuTransferInterface.h"


class ICudaTransfer : public IGpuTransfer<CCudaObject>
{
public:
	/* In asynchronous transfer schemes where, subclasses override AcquireTexture
	   to wait (or have opengl operations wait) until the texture is available to use. */
	virtual void AcquireObject(CCudaObject* object, CUstream stream) const = 0;
	
	/* In asynchronous transfer schemes where, subclasses override ReleaseTexture
	   to signal that OpenGL is done using the texture. */
	virtual void ReleaseObject(CCudaObject* object, CUstream stream) const = 0;

};

GPU_EXPORT ICudaTransfer *CreateCudaTransfer();

#endif