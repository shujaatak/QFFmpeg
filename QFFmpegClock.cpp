#include "QFFmpegClock.h"
#include "QFFmpegAudioDecoder.h"
#include <QDebug>

QFFmpegClock::QFFmpegClock(QObject *parent) : QThread(parent)
{
    starttime=0;
    mAudioDecoder=NULL;

}

QFFmpegClock::~QFFmpegClock()
{

}


void QFFmpegClock::setAudioDecoder(QFFmpegAudioDecoder *audioDecoder)
{
    this->mAudioDecoder=audioDecoder;

}


double QFFmpegClock::getClockTime()
{
   // qDebug("---  %f ----  %f",time,mAudioDecoder->getAudioTime());
   // if(mAudioDecoder)
        //return mAudioDecoder->getAudioTime();
    return time;
}

void QFFmpegClock::run()
{
    starttime=av_gettime_relative()/1000000.0;
    while(true)
    {
        time=av_gettime_relative()/1000000.0-starttime;
        //qDebug("%f",time-starttime);
        QThread::msleep(10);
    }

}
