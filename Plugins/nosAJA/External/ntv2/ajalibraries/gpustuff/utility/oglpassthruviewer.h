/* SPDX-License-Identifier: MIT */
#ifndef OGLPASSTHRUVIEWER_H
#define OGLPASSTHRUVIEWER_H

#include "gpustuff/include/oglTransfer.h"
#include "gpustuff/utility/videoprocessor.h"

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
    void process(GLuint textureHandle);

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

class COglPassthruViewer : public CVideoProcessor<COglObject>
{
public:
    COglPassthruViewer(QWidget *parent,  COglWidget*		oglWidget, IOglTransfer *gpuTransfer);
	~COglPassthruViewer();	

protected:

    virtual bool Process();
    virtual bool SetupThread();
	virtual bool CleanupThread();
    virtual bool Init();
	virtual bool Deinit();

private:
    COglWidget *m_oglWidget;

private:

};



#endif
