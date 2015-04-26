#include "QFFmpegVideoRender.h"


QFFmpegVideoRender::QFFmpegVideoRender(QObject *parent) : QThread(parent)
{
    bufpos=0;
}

QFFmpegVideoRender::~QFFmpegVideoRender()
{

}

void QFFmpegVideoRender::setGLWidget(QFFmpegGLWidget *mGLWidget)
{
    this->mGLWidget=mGLWidget;
}
void QFFmpegVideoRender::setClock(QFFmpegClock *mClock)
{
    this->mClock=mClock;
}
void QFFmpegVideoRender::run()
{

    while(true)
    {
        if(queue.length()>0)
        {
            PICTURE pic=queue.front();

            if(pic.time>=mClock->getClockTime())
            {

                mGLWidget->updateData(pic.data);
                queue.dequeue();
            }
        }
        QThread::msleep(10);

    }
}


void QFFmpegVideoRender::setSize(int w,int h)
{
    width=w;
    height=h;
    int size=w*h*3/2;
    if(size>0){
        buffer[0]=(uint8_t*)malloc(size);
        buffer[1]=(uint8_t*)malloc(size);
        buffer[2]=(uint8_t*)malloc(size);

        mGLWidget->initShader(w,h);
    }
    else{

    }

}

int QFFmpegVideoRender::getQueueSize()
{

    return queue.length();
}

void QFFmpegVideoRender::addPicture(uint8_t **data,double time)
{
    PICTURE picture;
    picture.data=buffer[bufpos];
    picture.time=time;
    memcpy(picture.data,data[0],width*height);
    memcpy(picture.data+width*height,data[1],width*height/4);
    memcpy(picture.data+width*height*5/4,data[2],width*height/4);

    queue.enqueue(picture);
    bufpos++;
    if(bufpos>2)
        bufpos=0;

}

