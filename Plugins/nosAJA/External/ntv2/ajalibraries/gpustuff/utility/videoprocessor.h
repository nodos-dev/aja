/* SPDX-License-Identifier: MIT */
#ifndef NTVGPUPROCESSOR_H
#define NTVGPUPROCESSOR_H

#include <QThread>

#include "ajabase/common/types.h"


#include "time.h"
#include <ctime>

#include "ajabase/common/circularbuffer.h"
#include "gpustuff/include/cpuObject.h"
#include "gpustuff/include/gpuTransferInterface.h"

enum {INQ, OUTQ};

template<class T>
class CVideoProcessor : public QThread
{

public:
	CVideoProcessor(IGpuTransfer<T>	*gpuTransfer);
	virtual ~CVideoProcessor();
	void SetGpuQueue(int type, AJACircularBuffer<T*> *q);
	void SetCpuQueue(int type, AJACircularBuffer<CCpuObject*> *q);
	
	virtual void Stop();	
	virtual void Start();	
	virtual bool Init() = 0;	//{return true;}   // Moved here to prevent race condition.
	virtual bool Deinit() = 0;	//{return true;}
	
protected:	
	virtual void run();		
	virtual bool Process() = 0;			//{return true;}	
	virtual bool SetupThread() = 0;		//{return true;}
	virtual bool CleanupThread() = 0;	//{return true;}	
	bool											mAbortFlag;
	AJACircularBuffer<T*>*							mGpuQueue[2];
	AJACircularBuffer<CCpuObject *>*				mCpuQueue[2];	
	IGpuTransfer<T>									*mGpuTransfer;
};

template<class T>
CVideoProcessor<T>::CVideoProcessor(IGpuTransfer<T>	*gpuTransfer)
:mGpuTransfer(gpuTransfer)
{
	mGpuQueue[0] = NULL_PTR;
	mGpuQueue[1] = NULL_PTR;
	mCpuQueue[0] = NULL_PTR;
	mCpuQueue[1] = NULL_PTR;
	mAbortFlag = false;

}
template<class T>
CVideoProcessor<T>::~CVideoProcessor()
{
	
}

template<class T>
void CVideoProcessor<T>::SetGpuQueue(int type, AJACircularBuffer<T*> *q)
{
	if(type <0 || type > 1)
		return;

	mGpuQueue[type] = q;
}

template<class T>
void CVideoProcessor<T>::SetCpuQueue(int type, AJACircularBuffer<CCpuObject*> *q)
{
	if(type <0 || type > 1)
		return;

	mCpuQueue[type] = q;

}


template<class T>
void CVideoProcessor<T>::run()
{
	if(SetupThread() == false)
		return;
	while(!mAbortFlag)
	{
		Process();
	}

	CleanupThread();
}

template<class T>
void CVideoProcessor<T>::Stop()
{	
	mAbortFlag = true;	
}


template<class T>
void CVideoProcessor<T>::Start()
{	
	start();
}



#endif
