/* SPDX-License-Identifier: MIT */

#ifndef VIDEO_PROCESSING_ENGINE
#define VIDEO_PROCESSING_ENGINE

#include "videoprocessor.h"
#include "ajabase/common/circularbuffer.h"
#include "gpustuff/include/cpuObject.h"

template<class T>
class CVideoProcessingEngine 
{
public:
	CVideoProcessingEngine();
	~CVideoProcessingEngine();
	void StartEngine();
	void StopEngine();
	void ConnectViaCpuQueue(CVideoProcessor<T> *processor1, CVideoProcessor<T> *processor2);
	void ConnectViaGpuQueue(CVideoProcessor<T> *processor1, CVideoProcessor<T> *processor2);
	
private:
	void addToVec(CVideoProcessor<T> *processor);
	bool mAbortFlag;
	std::vector<CVideoProcessor<T> *>	mProcessorVec;
	//the connection queues
	std::vector<AJACircularBuffer<T*> *>		mGpuVec;	
	std::vector<AJACircularBuffer<CCpuObject*> *>		mCpuVec;	


};



template<class T>
CVideoProcessingEngine<T>::CVideoProcessingEngine()
{		
	mAbortFlag = false;
}
template<class T>
CVideoProcessingEngine<T>::~CVideoProcessingEngine()
{			
	for(int i = 0; i < mGpuVec.size(); i ++)
	{
		delete mGpuVec[i];
	}

	for(int i = 0; i < mCpuVec.size(); i ++)
	{
		delete mCpuVec[i];
	}
	mGpuVec.clear();
	mCpuVec.clear();
	mProcessorVec.clear();
}


template<class T>
void CVideoProcessingEngine<T>::StartEngine()
{		
	// Do Init before starting Thread to avoid race condition.
	for(int i = 0; i < mProcessorVec.size(); i ++)
	{
		mProcessorVec[i]->Init();
	}
	for(int i = 0; i < mProcessorVec.size(); i ++)
	{
		mProcessorVec[i]->Start();
	}
}

template<class T>
void CVideoProcessingEngine<T>::StopEngine()
{	
	mAbortFlag = true;
	for(int i = 0; i < mProcessorVec.size(); i ++)
	{
		mProcessorVec[i]->Stop();
	}
	for(int i = 0; i < mProcessorVec.size(); i ++)
	{
		mProcessorVec[i]->wait();
	}
}

template<class T>
void CVideoProcessingEngine<T>::addToVec(CVideoProcessor<T> *processor)
{
	//add the processors to the vector
	bool add = true;
	for(int i = 0;i < mProcessorVec.size(); i++)
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
void CVideoProcessingEngine<T>::ConnectViaGpuQueue(CVideoProcessor<T> *processor1,CVideoProcessor<T> *processor2)
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
void CVideoProcessingEngine<T>::ConnectViaCpuQueue(CVideoProcessor<T> *processor1,CVideoProcessor<T> *processor2)
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

