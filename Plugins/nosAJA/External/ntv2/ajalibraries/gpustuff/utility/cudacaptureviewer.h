/* SPDX-License-Identifier: MIT */
#ifndef CUDACAPTUREVIEWER_H
#define CUDACAPTUREVIEWER_H
#include "gpustuff/include/cudaTransfer.h"
#include "gpustuff/utility/videoprocessor.h"

#include <QtGui>
#include <QGLWidget>
#include <GL/glu.h>

#include "ajabase/common/public.h"
#include "ajabase/common/types.h"

#include "time.h"
#include <ctime>

#include <cudaGL.h>


class COglWidget: public QGLWidget
{
public:
    COglWidget(QWidget *parent = NULL);
    ~COglWidget();

    void doGlResize();
    void setup();
protected:
    void paintGL();
    void resizeEvent(QResizeEvent *evt);
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *evt);

    void resizeViewport(const QSize &size);

    bool doResize;
    int w;
    int h;


};


class CCudaCaptureViewer :  public CVideoProcessor<CCudaObject>
{
public:
    CCudaCaptureViewer(QWidget *parent,COglWidget*		oglWidget, CUcontext				cudaCtx,  ICudaTransfer *gpuTransfer);
	~CCudaCaptureViewer();	
    
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
	void releaseWindowTexture();
	
	CUcontext mCudaCtx;
	CUgraphicsResource mWindowCudaResource;
	CUarray mWindowCudaArray;
	GLuint mWindowTexture;
	GLuint mTextureWidth;
	GLuint mTextureHeight;

private:
    COglWidget *m_oglWidget;
};



#endif
