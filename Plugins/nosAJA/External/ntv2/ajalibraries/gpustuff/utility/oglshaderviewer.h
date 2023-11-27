/* SPDX-License-Identifier: MIT */
#ifndef OGLSHADERVIEWER_H
#define OGLSHADERVIEWER_H

#include "gpustuff/include/oglTransfer.h"
#include "gpustuff/utility/videoprocessor.h"

#include <QtGui>
#include <QGLWidget>
//#include <QtOpenGL>
#include <GL/glu.h>

#include "ajabase/common/public.h"
#include "ajabase/common/types.h"
#include "ajabase/common/videoutilities.h"
#include "time.h"
#include <ctime>

// Also needs to be defined/undefined in ntv2qtogldpxplayback.cpp
#define DO_10BIT_OUTPUT


class COglShaderViewer : public QGLWidget, public CVideoProcessor<COglObject>
{
public:
    COglShaderViewer(QWidget *parent, QGLWidget*		sharedWidget,  IOglTransfer *gpuTransfer, AJA_PixelFormat pixelFormat);
	~COglShaderViewer();	


protected:
    void resizeEvent(QResizeEvent *evt);
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *evt);

    void resizeViewport(const QSize &size);
    virtual bool Process();
    virtual bool SetupThread();
	virtual bool CleanupThread();
    virtual bool Init();
	virtual bool Deinit();
        
private:
    bool doResize;
    int w;
    int h;

	COglObject *scratchOglObject;

	bool first;

	GLuint vertShader;

	GLuint inputShader;
	GLuint inputProgram;

	GLuint outputShader;
	GLuint outputProgram;

	GLuint computeShader;
	GLuint computeProgram;

	AJA_PixelFormat mPixelFormat;
};



#endif
