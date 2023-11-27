/* SPDX-License-Identifier: MIT */
//---------------------------------------------------------------------------------------------------------------------
//  cudacaptureviewer.cpp
//
//	Copyright (C) 2012 AJA Video Systems, Inc.  Proprietary and Confidential information.  All rights reserved.
//---------------------------------------------------------------------------------------------------------------------
#include "cudacaptureviewer.h"
#include "ajabase/system/systemtime.h"
#include "ajabase/common/videoutilities.h"

#include <QDate>
#include <string>
#include <sstream>
#include <iomanip>
 
using std::string;

COglWidget::COglWidget(QWidget *parent)
{
    w=0;
    h=0;
    resize(320, 240);
}
COglWidget::~COglWidget()
{


}

void COglWidget::resizeViewport(const QSize &size)
{
    w = size.width();
    h = size.height();
    doResize = true;
}

void COglWidget::resizeEvent(QResizeEvent *evt)
{
    resizeViewport(evt->size());
}

void COglWidget::paintEvent(QPaintEvent *)
{
    // Handled by the GLThread.
    //swapBuffers();
}

void COglWidget::closeEvent(QCloseEvent *evt)
{
    QGLWidget::closeEvent(evt);

}

void COglWidget::paintGL()
{

    swapBuffers();

}
void COglWidget::doGlResize()
{
    if (doResize) {
        glViewport(0, 0, w, h);
        doResize = false;
    }

}
void COglWidget::setup()
{
    setAutoBufferSwap(false);
    glClearColor(0,0,0,0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    assert(glGetError() == GL_NO_ERROR);

}

CCudaCaptureViewer::CCudaCaptureViewer(QWidget *parent,COglWidget*		oglWidget,  CUcontext				cudaCtx, ICudaTransfer *gpuTransfer)
: CVideoProcessor<CCudaObject>(gpuTransfer), mCudaCtx(cudaCtx)
{
    m_oglWidget = oglWidget;
	mWindowTexture = GL_ZERO;	
	mTextureWidth = 0;
	mTextureHeight = 0;
}


void CCudaCaptureViewer::releaseWindowTexture()
{
	if(mWindowTexture)
	{
		CUCHK(cuGraphicsUnregisterResource(mWindowCudaResource));
        glDeleteTextures(1,&mWindowTexture);
	}
}

CCudaCaptureViewer::~CCudaCaptureViewer()
{
	wait();
	Deinit();
	releaseWindowTexture();
}

    
void CCudaCaptureViewer::resizeViewport(const QSize &size)
{
    w = size.width();
    h = size.height();
    doResize = true;
}    

bool CCudaCaptureViewer::Init()
{


	return true;
}
bool CCudaCaptureViewer::Deinit()
{	
	return true;
}

bool CCudaCaptureViewer::Process()
{
    srand(QTime::currentTime().msec());
    if (doResize) {
        glViewport(0, 0, w, h);
        doResize = false;
    }
	CCudaObject *inGpuObject = NULL;	
	if(mGpuQueue[INQ])
	{		
		inGpuObject = mGpuQueue[INQ]->StartConsumeNextBuffer();
		if(inGpuObject == NULL) return false;
		(static_cast<ICudaTransfer*>(mGpuTransfer))->AcquireObject(inGpuObject,0);		
		
		uint32_t width = inGpuObject->GetWidth();
		uint32_t height = inGpuObject->GetHeight();
		if(mWindowTexture == GL_ZERO || mTextureWidth != width || mTextureHeight != height)
		{
			mTextureWidth = width;
			mTextureHeight = height;
			if(mWindowTexture)
				releaseWindowTexture();			
			glGenTextures(1, &mWindowTexture);
			glBindTexture(GL_TEXTURE_2D, mWindowTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
								width, height,
								0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST );
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
			glBindTexture(GL_TEXTURE_2D, 0);
			CUCHK(cuGraphicsGLRegisterImage(&mWindowCudaResource, mWindowTexture, GL_TEXTURE_2D, CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD));
		}
		////
		//you can call the cuda kernel on the cuda object here
		////

		//copy the object into the window texture using CUDA-OpenGL interoperability
		//map the window texture into CUDA
		CUCHK(cuGraphicsMapResources(1, &mWindowCudaResource, 0));
		CUCHK(cuGraphicsSubResourceGetMappedArray(&mWindowCudaArray, mWindowCudaResource,0,0));
		CUDA_MEMCPY2D desc;
		desc.srcArray = inGpuObject->GetTextureHandle();
		desc.srcMemoryType = CU_MEMORYTYPE_ARRAY;
		desc.srcXInBytes = 0;
		desc.srcY = 0;
		
		desc.dstArray = mWindowCudaArray;
		desc.dstMemoryType = CU_MEMORYTYPE_ARRAY;
		desc.dstXInBytes = 0;
		desc.dstY = 0;
		desc.WidthInBytes = inGpuObject->GetStride();
		desc.Height = inGpuObject->GetHeight();
		CUCHK(cuMemcpy2D(&desc));
		//unmap the window texture from CUDA
		CUCHK(cuGraphicsUnmapResources(1, &mWindowCudaResource, 0));
		QRectF target;		
		target.setRect(-1.0, -1.0, 2.0, 2.0);		
        m_oglWidget->drawTexture(target,mWindowTexture,GL_TEXTURE_2D);
		(static_cast<ICudaTransfer*>(mGpuTransfer))->ReleaseObject(inGpuObject,0);
		mGpuQueue[INQ]->EndConsumeNextBuffer();
		
	}	
    m_oglWidget->makeCurrent();
    m_oglWidget->swapBuffers();

	return true;
}

bool CCudaCaptureViewer::SetupThread()
{	
	//this is called inside the thread
	//make OGL context current
    m_oglWidget->makeCurrent();
    m_oglWidget->setup();
    CUCHK(cuCtxSetCurrent(mCudaCtx));
	return true;
}


bool CCudaCaptureViewer::CleanupThread()
{
	//this is called inside the thread
    //this is called inside the thread
    m_oglWidget->doneCurrent();
    m_oglWidget->context()->moveToThread(QGuiApplication::instance()->thread());
    return true;
}


