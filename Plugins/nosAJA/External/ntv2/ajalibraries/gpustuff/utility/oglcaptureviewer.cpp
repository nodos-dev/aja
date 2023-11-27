/* SPDX-License-Identifier: MIT */
//---------------------------------------------------------------------------------------------------------------------
//  OglCaptureViewer.cpp
//
//	Copyright (C) 2012 AJA Video Systems, Inc.  Proprietary and Confidential information.  All rights reserved.
//---------------------------------------------------------------------------------------------------------------------
#include "oglcaptureviewer.h"
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

//COglCaptureViewer::COglCaptureViewer(QWidget *parent, QGLContext*	oglContext, IOglTransfer *gpuTransfer)
//:CVideoProcessor<COglObject>(gpuTransfer),  QGLWidget(oglContext, parent)
COglCaptureViewer::COglCaptureViewer(QWidget *parent,  COglWidget*		oglWidget, IOglTransfer *gpuTransfer)
: CVideoProcessor<COglObject>(gpuTransfer)
{
    m_oglWidget = oglWidget;

    //resize(320, 240);
    //doResize = false;
}



COglCaptureViewer::~COglCaptureViewer()
{
	wait();
	Deinit();

}

    

bool COglCaptureViewer::Init()
{	
	return true;
}
bool COglCaptureViewer::Deinit()
{		
	return true;
}

bool COglCaptureViewer::Process()
{
    srand(QTime::currentTime().msec());
    m_oglWidget->doGlResize();
	COglObject *inGpuObject = NULL;	
	if(mGpuQueue[INQ])
	{		
		inGpuObject = mGpuQueue[INQ]->StartConsumeNextBuffer();
		if(inGpuObject)
		{
			(static_cast<IOglTransfer*>(mGpuTransfer))->AcquireObject(inGpuObject);
			QRectF target;		
			target.setRect(-1.0, -1.0, 2.0, 2.0);		
            m_oglWidget->drawTexture(target,inGpuObject->GetTextureHandle(),GL_TEXTURE_2D);
			(static_cast<IOglTransfer*>(mGpuTransfer))->ReleaseObject(inGpuObject);
			mGpuQueue[INQ]->EndConsumeNextBuffer();
            m_oglWidget->updateGL();
            //m_oglWidget->swapBuffers();
		}		
	}	

	return true;
}

bool COglCaptureViewer::SetupThread()
{	
	//this is called inside the thread
	//make OGL context current  
    //m_sharedWidget->makeCurrent();
	//m_sharedWidget->setAutoBufferSwap(false);
    m_oglWidget->makeCurrent();
    m_oglWidget->setup();

	return true;
}


bool COglCaptureViewer::CleanupThread()
{
	//this is called inside the thread
	m_oglWidget->doneCurrent();
	m_oglWidget->context()->moveToThread(QGuiApplication::instance()->thread());
	return true;
}

