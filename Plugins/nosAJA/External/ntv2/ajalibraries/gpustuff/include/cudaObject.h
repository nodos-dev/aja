/* SPDX-License-Identifier: MIT */

#ifndef _CUDA_OBJECT
#define _CUDA_OBJECT

#include "gpuObject.h"
#include <cuda.h>
#include "export_gpu.h"

void GPU_EXPORT failCuda(CUresult hr);
#define CUCHK(call) do {                                         \
    CUresult status = call;                                             \
    if( CUDA_SUCCESS != status) {                                       \
        fprintf(stderr, "Cuda error in file '%s' in line %i : %d.\n",   \
                __FILE__, __LINE__, status);                            \
        failCuda(status);                                               \
    } } while (0)

int GPU_EXPORT cudaArrayFormatToBytes(CUarray_format format);

class CCudaObject : public IGpuObject
{
public:
	GPU_EXPORT CCudaObject();	
	GPU_EXPORT virtual ~CCudaObject();
	
	GPU_EXPORT virtual void Init(const GpuObjectDesc &desc);
	GPU_EXPORT CUarray GetTextureHandle() const;
	GPU_EXPORT CUdeviceptr GetBufferHandle() const;	
private:
	void destroy();
	
	CUarray _arrayHandle;
	CUdeviceptr _bufferHandle;
};

#endif

