/* SPDX-License-Identifier: MIT */

#ifndef CUDA_PIPELINE_H
#define CUDA_PIPELINE_H

#include <stdio.h>

#include "gpustuff/include/cudaObject.h"
#include "gpustuff/include/cudaTransfer.h"
#include "gpustuff/utility/gpuPipeline.h"


class CudaPipelineEngine: public PipelineEngine<CCudaObject>
{

public:
	CudaPipelineEngine(CUcontext		cudaContext):m_cudaContext(cudaContext)
	{
		m_GpuTransfer = CreateCudaTransfer();
		makeGpuContextCurrent();
		m_GpuTransfer->Init();
		makeGpuContextUncurrent();
	}
	~CudaPipelineEngine()
	{
		makeGpuContextCurrent();
		m_GpuTransfer->Destroy();
		makeGpuContextUncurrent();
		delete m_GpuTransfer;
	}
	CUcontext GetCudaContext(){return m_cudaContext;}
	ICudaTransfer*   GetGpuTransfer() { return (static_cast<ICudaTransfer*>(m_GpuTransfer));}
protected:
	CUcontext m_cudaContext;
	void makeGpuContextCurrent(){CUCHK(cuCtxSetCurrent(m_cudaContext));}
	void makeGpuContextUncurrent(){CUCHK(cuCtxSetCurrent(0));}
	
};


#endif
