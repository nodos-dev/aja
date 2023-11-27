/* SPDX-License-Identifier: MIT */

#ifndef OGL_PIPELINE_ENGINE_H
#define OGL_PIPELINE_ENGINE_H

#include "gpustuff/include/oglObject.h"
#include "gpustuff/include/oglTransfer.h"
#include "gpustuff/utility/gpuPipeline.h"


#include <QGLWidget>





class OglPipelineEngine: public PipelineEngine<COglObject>
{

public:
	OglPipelineEngine(QGLWidget *sharedWidget):m_GLContext(sharedWidget)
	{
		m_GpuTransfer = CreateOglTransfer();
		makeGpuContextCurrent();
		m_GpuTransfer->Init();
		makeGpuContextUncurrent();
	}
	~OglPipelineEngine()
	{
		makeGpuContextCurrent();
		m_GpuTransfer->Destroy();
		makeGpuContextUncurrent();
		delete m_GpuTransfer;
	}
	QGLWidget *GetGLContext(){return m_GLContext;}
	IOglTransfer*   GetGpuTransfer() { return (static_cast<IOglTransfer*>(m_GpuTransfer));}
protected:
	QGLWidget *m_GLContext;
	void makeGpuContextCurrent(){m_GLContext->makeCurrent();}
	void makeGpuContextUncurrent(){m_GLContext->doneCurrent();}
	
};


#endif

