/* SPDX-License-Identifier: MIT */

#ifndef PIPELINE_ENGINE
#define PIPELINE_ENGINE

#include "videoprocessor.h"
#include "ajabase/common/circularbuffer.h"
#include "gpustuff/include/cpuObject.h"
#include "gpustuff/include/gpuObject.h"
#include "gpustuff/include/gpuTransferInterface.h"

template<class T>
class PipelineEngine 
{
public:
	PipelineEngine();
	~PipelineEngine();
	void InitEngine();
	void DeinitEngine();
	void StartEngine();
	void StopEngine();
	void ConnectViaCpuQueue(CVideoProcessor<T> *processor1, CVideoProcessor<T> *processor2);
	void ConnectViaGpuQueue(CVideoProcessor<T> *processor1, CVideoProcessor<T> *processor2);
	IGpuTransfer<T>*   GetGpuTransfer() { return m_GpuTransfer;}
protected:
	virtual void										     makeGpuContextCurrent() = 0;
	virtual void											 makeGpuContextUncurrent() = 0;
	void addToVec(CVideoProcessor<T> *processor);
	bool mAbortFlag;
	std::vector<CVideoProcessor<T> *>	mProcessorVec;
	//the connection queues
	std::vector<AJACircularBuffer<T*> *>		mGpuVec;	
	std::vector<AJACircularBuffer<CCpuObject*> *>		mCpuVec;	

	IGpuTransfer<T>*										m_GpuTransfer;
};



template<class T>
PipelineEngine<T>::PipelineEngine()
{		
	mAbortFlag = false;
}
template<class T>
PipelineEngine<T>::~PipelineEngine()
{			
	for(size_t i = 0; i < mGpuVec.size(); i ++)
	{
		delete mGpuVec[i];
	}

	for(size_t i = 0; i < mCpuVec.size(); i ++)
	{
		delete mCpuVec[i];
	}
	mGpuVec.clear();
	mCpuVec.clear();
	mProcessorVec.clear();
}


template<class T>
void PipelineEngine<T>::StartEngine()
{		
	for(size_t i = 0; i < mProcessorVec.size(); i ++)
	{
		mProcessorVec[i]->Start();
	}
}

template<class T>
void PipelineEngine<T>::StopEngine()
{	
	mAbortFlag = true;
	for(size_t i = 0; i < mProcessorVec.size(); i ++)
	{
		mProcessorVec[i]->Stop();
	}
	for(size_t i = 0; i < mProcessorVec.size(); i ++)
	{
		mProcessorVec[i]->wait();
	}
}

template<class T>
void PipelineEngine<T>::InitEngine()
{		
	makeGpuContextCurrent();
	// Do Init before starting Thread to avoid race condition.
	for(size_t i = 0; i < mProcessorVec.size(); i ++)
	{
		mProcessorVec[i]->Init();
	}
	makeGpuContextUncurrent();
}

template<class T>
void PipelineEngine<T>::DeinitEngine()
{
	makeGpuContextCurrent();

	for(size_t i = 0; i < mProcessorVec.size(); i ++)
	{
		mProcessorVec[i]->Deinit();
	}
	makeGpuContextUncurrent();
}


template<class T>
void PipelineEngine<T>::addToVec(CVideoProcessor<T> *processor)
{
	//add the processors to the vector
	bool add = true;
	for(size_t i = 0;i < mProcessorVec.size(); i++)
	{
		if(processor == mProcessorVec[i])
		{
			add = false;
			break;
		}
	}
	if(add)
		mProcessorVec.push_back(processor);
}

template<class T>
void PipelineEngine<T>::ConnectViaGpuQueue(CVideoProcessor<T> *processor1,CVideoProcessor<T> *processor2)
{
	if(processor1 == NULL || processor2 == NULL)
		return;
	AJACircularBuffer<T*> *gpuq = new AJACircularBuffer<T*>();
	gpuq->SetAbortFlag(&mAbortFlag);
	processor1->SetGpuQueue(OUTQ, gpuq);	
	processor2->SetGpuQueue(INQ, gpuq);	
	mGpuVec.push_back(gpuq);		
	//add the processors to the vector
	addToVec(processor1);
	addToVec(processor2);
}

template<class T>
void PipelineEngine<T>::ConnectViaCpuQueue(CVideoProcessor<T> *processor1,CVideoProcessor<T> *processor2)
{
	if(processor1 == NULL || processor2 == NULL)
		return;
	AJACircularBuffer<CCpuObject*> *cpuq = new AJACircularBuffer<CCpuObject*>();
	cpuq->SetAbortFlag(&mAbortFlag);
	processor1->SetCpuQueue(OUTQ, cpuq);	
	processor2->SetCpuQueue(INQ, cpuq);	
	mCpuVec.push_back(cpuq);		
	//add the processors to the vector
	addToVec(processor1);
	addToVec(processor2);
}

#endif 

