/* SPDX-License-Identifier: MIT */
#ifndef OGLCAPTUREVIEWER_H
#define OGLCAPTUREVIEWER_H

#include "gpustuff/include/oglTransfer.h"
#include "videoprocessor.h"

#include <QtGui>
#include <QGLWidget>
#include <GL/glu.h>

#include "ajabase/common/public.h"
#include "ajabase/common/types.h"

#include "time.h"
#include <ctime>

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


class COglCaptureViewer :  public CVideoProcessor<COglObject>
{
public:
    COglCaptureViewer(QWidget *parent, COglWidget*		oglWidget,  IOglTransfer *gpuTransfer);
	~COglCaptureViewer();	
    
protected:
    virtual bool Process();
    virtual bool SetupThread();
	virtual bool CleanupThread();
    virtual bool Init();
	virtual bool Deinit();
private:
	COglWidget *m_oglWidget;

};



#endif
