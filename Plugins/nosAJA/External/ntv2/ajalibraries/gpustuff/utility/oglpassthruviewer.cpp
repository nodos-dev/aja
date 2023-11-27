/* SPDX-License-Identifier: MIT */
//---------------------------------------------------------------------------------------------------------------------
//  oglpassthuviewer.cpp
//
//	Copyright (C) 2012 AJA Video Systems, Inc.  Proprietary and Confidential information.  All rights reserved.
//---------------------------------------------------------------------------------------------------------------------
#include "oglpassthruviewer.h"
#include "ajabase/system/systemtime.h"
#include "ajabase/common/videoutilities.h"

#include <QDate>
#include <QThread>
#include <string>
#include <sstream>
#include <iomanip>


#include "teapot.h"

#define TEAPOT
COglWidget::COglWidget(QWidget *parent)
{
    w=320;
    h=540;
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
    glClearColor(1,0,0,0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    assert(glGetError() == GL_NO_ERROR);

}

void COglWidget::process(GLuint textureHandle)
{
    // Reset view parameters for background image
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);

    // Bind texture object
    glBindTexture(GL_TEXTURE_2D,textureHandle );

    // Draw the background as the source
    glBegin(GL_QUADS);
    glTexCoord2f(1.0, 1.0); glVertex2f(1, 1);
    glTexCoord2f(1.0, 0.0);  glVertex2f(1, -1);
    glTexCoord2f(0.0, 0.0);  glVertex2f(-1, -1);
    glTexCoord2f(0.0, 1.0); glVertex2f(-1, 1);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

#ifdef TEAPOT
    glClear(GL_DEPTH_BUFFER_BIT);
    // draw the teapot
    static 	float angle = 0.0;
    static int frame_count_per_rotation = 360;
    static int frame_count = 0;

    // Increment rotation angle
    frame_count += 1;
    angle = (frame_count % frame_count_per_rotation);
    angle /= (GLfloat) frame_count_per_rotation;///NUM_FAKE_VIDEOS;
    angle = 359 - angle*360;

    double halfWinWidth = w / 2.0;
    double halfWinHeight = h / 2.0;
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-halfWinWidth, halfWinWidth, halfWinHeight, -halfWinHeight, -1000.0, 1000.0),
    gluLookAt(0, 0.5, 1, 0, 0, 0, 0, 1, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScaled(75.0, 75.0, 75.0);
    glTranslated(0.0, -1.0, 0.0);
    glRotated(angle, 0.0, 1.0, 0.0);

    glEnable(GL_LIGHTING);
    drawTeapot();
    glDisable(GL_LIGHTING);
    //finished drawing the teapot
    glEnable(GL_TEXTURE_2D);
#endif

    assert(glGetError() == GL_NO_ERROR);
}

COglPassthruViewer::COglPassthruViewer(QWidget *parent,  COglWidget*		oglWidget, IOglTransfer *gpuTransfer)
    : CVideoProcessor<COglObject>(gpuTransfer)
{
    m_oglWidget = oglWidget;
}

COglPassthruViewer::~COglPassthruViewer()
{
}

bool COglPassthruViewer::Process()
{
	//odprintf("COglPassthruViewer::Process()");

    srand(QTime::currentTime().msec());
    m_oglWidget->doGlResize();
	COglObject *inGpuObject = NULL;	
	if(mGpuQueue[INQ])
	{		
		inGpuObject = mGpuQueue[INQ]->StartConsumeNextBuffer();
		//odprintf("inGpuObject Start");
	}

	COglObject *outGpuObject = NULL;	
	if(mGpuQueue[OUTQ])
	{
		outGpuObject = mGpuQueue[OUTQ]->StartProduceNextBuffer();
		//odprintf("outGpuObject Start");

	}		
	if(inGpuObject && outGpuObject) 
	{					
		assert(glGetError() == GL_NO_ERROR);
		(static_cast<IOglTransfer*>(mGpuTransfer))->AcquireObject(inGpuObject);
		assert(glGetError() == GL_NO_ERROR);
		(static_cast<IOglTransfer*>(mGpuTransfer))->AcquireObject(outGpuObject);
		assert(glGetError() == GL_NO_ERROR);

		//copy the input texture to the output texture
		outGpuObject->Begin();		

        m_oglWidget->process(inGpuObject->GetTextureHandle());

		outGpuObject->End();

        // Reset view parameters to blit to window
        glViewport(0, 0, m_oglWidget->width(), m_oglWidget->height());
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glEnable(GL_TEXTURE_2D);

        // Disable depth test
        glDisable(GL_DEPTH_TEST);
        assert(glGetError() == GL_NO_ERROR);

        QRectF target;
		target.setRect(-1.0, -1.0, 2.0, 2.0);		
        m_oglWidget->drawTexture(target,outGpuObject->GetTextureHandle(),GL_TEXTURE_2D);

        (static_cast<IOglTransfer*>(mGpuTransfer))->ReleaseObject(inGpuObject);
		(static_cast<IOglTransfer*>(mGpuTransfer))->ReleaseObject(outGpuObject);
	}
	else if(inGpuObject)
	{
		(static_cast<IOglTransfer*>(mGpuTransfer))->AcquireObject(inGpuObject);
		QRectF target;		
		target.setRect(-1.0, -1.0, 2.0, 2.0);		
        m_oglWidget->drawTexture(target,inGpuObject->GetTextureHandle(),GL_TEXTURE_2D);
		(static_cast<IOglTransfer*>(mGpuTransfer))->ReleaseObject(inGpuObject);
	}      
	if(mGpuQueue[INQ])
	{		
		mGpuQueue[INQ]->EndConsumeNextBuffer();
		//odprintf("outGpuObject End");
	}	
	if(mGpuQueue[OUTQ])
	{
		mGpuQueue[OUTQ]->EndProduceNextBuffer();	
		//odprintf("outGpuObject End");		
    }

    m_oglWidget->updateGL();

	return true;
}

bool COglPassthruViewer::Init()
{
	return true;
}

bool COglPassthruViewer::Deinit()
{	
	return true;
}

bool COglPassthruViewer::SetupThread()
{	
	//this is called inside the thread
	//make OGL context current

    m_oglWidget->makeCurrent();
    m_oglWidget->setup();

#ifdef TEAPOT	
	//init teapot
	glEnable(GL_DEPTH_TEST); 
	
	// Initialize lighting for render to texture
	GLfloat spot_ambient[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat spot_position[] = {0.0, 3.0, 3.0, 0.0};
	GLfloat local_view[] = {0.0};
	GLfloat ambient[] = {0.01175, 0.01175, 0.1745};
	GLfloat diffuse[] = {0.04136, 0.04136, 0.61424};
	GLfloat specular[] = {0.626959, 0.626959, 0.727811};

	glLightfv(GL_LIGHT0, GL_POSITION, spot_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, spot_ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

	glFrontFace(GL_CCW);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 0.6*128.0);

	glDisable(GL_LIGHTING);
	

	glColor3f(1.0, 1.0, 1.0);
	
	glClearColor(.5, .5, .5, 1.0);
	glClearDepth( 1.0 ); 
	// Create teapot
	createTeapot(10);
#endif
	return true;
}

bool COglPassthruViewer::CleanupThread()
{
    //this is called inside the thread
    m_oglWidget->doneCurrent();
    m_oglWidget->context()->moveToThread(QGuiApplication::instance()->thread());
    return true;
}

