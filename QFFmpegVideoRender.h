#ifndef QFFMPEGVIDEORENDER_H
#define QFFMPEGVIDEORENDER_H

#include <QThread>
#include "QFFmpegGLWidget.h"
#include "QFFmpegClock.h"
#include <QQueue>

typedef struct PICTURE
{
    uint8_t *data;
    double time;
}PICTURE;

class QFFmpegVideoRender : public QThread
{
    Q_OBJECT
public:
    explicit QFFmpegVideoRender(QObject *parent = 0);
    ~QFFmpegVideoRender();


    void setGLWidget(QFFmpegGLWidget *mGLWidget);
    void setClock(QFFmpegClock *mClock);
    void setSize(int w,int h);
    void addPicture(uint8_t **data,double time);
int getQueueSize();
signals:

public slots:
private:
    QFFmpegGLWidget *mGLWidget;
    QFFmpegClock *mClock;
    int width;
    int height;
    QQueue<PICTURE> queue;
    uint8_t *buffer[3];
    int bufpos;
protected:
    void run();
};

#endif // QFFMPEGVIDEORENDER_H
