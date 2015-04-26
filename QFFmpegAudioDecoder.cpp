#include "QFFmpegAudioDecoder.h"
#include <QDebug>


static uint8_t audio_buf[10240];
uint8_t *buff[]={audio_buf};



void QFFmpegAudioDecoder::setStream(AVStream *mStream)
{
    this->mStream=mStream;
    qDebug("+++++++++++++++");
}

void QFFmpegAudioDecoder::setClock(QFFmpegClock *clock)
{
    this->mClock=clock;
}
double QFFmpegAudioDecoder::getAudioTime()
{
    if(mBufferTimes.length()>0)
    {
        float tt=al->getBufferTimeOffset();
        //qDebug("--- %f %f",mBufferTimes.at(0),tt);
        return mBufferTimes.at(0)+tt;
    }
    else
        return 0;

}

void QFFmpegAudioDecoder::timerStart()
{
    timer->start();
}
void QFFmpegAudioDecoder::timerUpdate()
{
    if(mBufferTimes.length()>0)
    {
        float tt=al->getBufferTimeOffset();
        //qDebug("play:%lf",mBufferTimes.at(0)+tt);
    }

}

QFFmpegAudioDecoder::QFFmpegAudioDecoder(QObject *parent) : QThread(parent)
{
    mAudioClock=0;
    mBufferStartTime=0;

    queue.clear();
    QOpenALEngine::init();

    al=new QOpenALEngine();
    timer=new QTimer();
    timer->setInterval(20);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));

}

QFFmpegAudioDecoder::~QFFmpegAudioDecoder()
{
    qDebug("=================");
    QOpenALEngine::deinit();
}



void QFFmpegAudioDecoder::setCodecCtx(AVCodecContext *mAudioCtx)
{
    this->mAudioCtx=mAudioCtx;

    swr_ctx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
                                 mAudioCtx->sample_rate, mAudioCtx->channel_layout,
                                 mAudioCtx->sample_fmt, mAudioCtx->sample_rate, 0, NULL);
    if (swr_init(swr_ctx) < 0) {
        qDebug("Failed to initialize the resampling context\n");
    }
}


void QFFmpegAudioDecoder::run()
{
    AVFrame *frame = av_frame_alloc();

    int got_frame = 0;
    int ret=0;
    AVPacket pkt_temp,pkt;
    int packet_pending=0;
    int64_t next_pts;
    AVRational next_pts_tb;

    pkt_temp.data=0;
    pkt.data=0;

    while(true)
    {

        if(!packet_pending)
        {
            if(pkt.data)av_free_packet(&pkt);
            // qDebug("----:%d",pkt_temp.data);
            bool rr=sema.tryAcquire(1,20);
            if(!rr) continue;
            mutex.lock();
            pkt_temp =pkt=queue.dequeue().pkt;
            mutex.unlock();


            packet_pending = 1;
        }


        //解码一帧
        do {
            int ret = -1;


            ret = avcodec_decode_audio4(mAudioCtx, frame, &got_frame, &pkt_temp);
            //qDebug("%d  %d  %d",ret,got_frame,pkt_temp.pts);

            if (got_frame) {
                AVRational tb;
                tb.num=1;
                tb.den=frame->sample_rate;

                if (frame->pts != AV_NOPTS_VALUE)
                    frame->pts = av_rescale_q(frame->pts, mAudioCtx->time_base, tb);
                else if (frame->pkt_pts != AV_NOPTS_VALUE)
                    frame->pts = av_rescale_q(frame->pkt_pts, av_codec_get_pkt_timebase(mAudioCtx), tb);
                else if (next_pts != AV_NOPTS_VALUE)
                    frame->pts = av_rescale_q(next_pts, next_pts_tb, tb);
                if (frame->pts != AV_NOPTS_VALUE) {
                    next_pts = frame->pts + frame->nb_samples;
                    next_pts_tb = tb;
                }
            }




            if (ret < 0) {
                packet_pending = 0;
            } else {
                //qDebug("get===%d",pkt_temp.size);
                pkt_temp.dts = pkt_temp.pts = AV_NOPTS_VALUE;
                if (pkt_temp.data) {
                    pkt_temp.data += ret;
                    pkt_temp.size -= ret;
                    if (pkt_temp.size <= 0)
                        packet_pending = 0;
                } else {
                    if (!got_frame) {
                        packet_pending = 0;
                        // finished = pkt_serial;
                    }
                }
            }

            if(!packet_pending) break;


        }while (!got_frame);


        //qDebug("12345:%d",packet_pending);



        while(true)
        {
            int pnum=al->getProcessedNum();
            int qnum=al->getQueuedNum();
            QOpenALEngine::State state=al->getState();

            if((state!=QOpenALEngine::PLAYING) || (state==QOpenALEngine::PLAYING && pnum>1))
            {
                double dpts = NAN;
                if (frame->pts != AV_NOPTS_VALUE)
                    dpts = av_q2d(mStream->time_base) * frame->pts;

                //qDebug("play:%lf",dpts);

                int len=swr_convert(swr_ctx,buff,10240,
                                    (const uint8_t **) frame->data,frame->nb_samples);

                len=len*2*av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
                al->write(buff[0],len,mAudioCtx->sample_rate);

                if(mBufferTimes.length()>=4)
                    mBufferTimes.dequeue();
                mBufferTimes.enqueue(dpts);

                if(qnum==4 && state!=QOpenALEngine::PLAYING)
                {
                    mBufferStartTime=dpts;
                    al->play();
                    mClock->start();
                    QTimer::singleShot(0,this,SLOT(timerStart()));
                }


                break;
            }
            else QThread::msleep(20);
        }







    }

}


void QFFmpegAudioDecoder::addPacket(AVPacket pkt)
{

    FFAVPacket avpkt;
    avpkt.pkt=pkt;
    mutex.lock();
    queue.enqueue(avpkt);
    sema.release();
    mutex.unlock();

}


int QFFmpegAudioDecoder::getQueueLength()
{
    return queue.length();
}


