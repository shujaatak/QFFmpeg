#include "QFFmpegVideoDecoder.h"


QFFmpegVideoDecoder::QFFmpegVideoDecoder(QObject *parent) : QThread(parent)
{
    queue.clear();

    mutex=new QMutex();
    sema=new QSemaphore(0);
    convert_ctx=0;
    picInit=false;
    mVideoRender=new QFFmpegVideoRender();
}

QFFmpegVideoDecoder::~QFFmpegVideoDecoder()
{
    delete mutex;
    delete sema;
}
void QFFmpegVideoDecoder::setClock(QFFmpegClock *clock)
{
    this->mClock=clock;
    mVideoRender->setClock(clock);
}
void QFFmpegVideoDecoder::addPacket(AVPacket pkt)
{
    //qDebug(".............");
    mutex->lock();
    queue.enqueue(pkt);
    sema->release();
    mutex->unlock();


}


void QFFmpegVideoDecoder::setGLWidget(QFFmpegGLWidget *gl)
{
    this->mGLWidget=gl;
mVideoRender->setGLWidget(gl);
}


static uint8_t video_buf[1024000000];

void QFFmpegVideoDecoder::setCodecCtx(AVCodecContext *mVideoCtx)
{
    this->mVideoCtx=mVideoCtx;


}

int QFFmpegVideoDecoder::getQueueLength()
{
    return queue.length();
}


void QFFmpegVideoDecoder::setFormatCtx(AVFormatContext *mFormatCtx)
{
    this->mFormatCtx=mFormatCtx;

}
void QFFmpegVideoDecoder::setStream(AVStream *mStream)
{
    this->mStream=mStream;
}

int linesize[AV_NUM_DATA_POINTERS];

void QFFmpegVideoDecoder::run()
{
    qDebug("QFFmpegVideoDecoder");
    AVPacket pkt_temp,pkt;
    AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    AVRational tb = mVideoCtx->time_base;
    AVRational frame_rate = av_guess_frame_rate(mFormatCtx, mStream, NULL);
    int packet_pending=0;
    int got_frame=0;

    //qDebug("-- %d",this->mVideoCtx->width);


    //avpicture_fill((AVPicture *)frameA, video_buf, AV_PIX_FMT_RGB565LE,mVideoCtx->width, mVideoCtx->height);


    for (;;) {


        int got_picture;



        if(!packet_pending)
        {

            // qDebug("----:%d",pkt_temp.data);
            bool rr=sema->tryAcquire(1,20);
            if(!rr) continue;
            mutex->lock();
            pkt_temp =pkt=queue.dequeue();
            mutex->unlock();


            packet_pending = 1;
        }


        //解码一帧
        do {
            int ret = -1;


            ret = avcodec_decode_video2(mVideoCtx, frame, &got_frame, &pkt_temp);
            if (got_frame)
            {
                frame->pts = av_frame_get_best_effort_timestamp(frame);
            }


            if (ret < 0) {
                packet_pending = 0;
            } else {

                pkt_temp.dts = pkt_temp.pts = AV_NOPTS_VALUE;
                if (pkt_temp.data)
                {
                    packet_pending = 0;
                }
                else  if (!got_frame)
                {
                    packet_pending = 0;
                }
            }

            if(!packet_pending) break;


        }while (!got_frame);




        //

        av_free_packet(&pkt);

        if (got_frame) {
            double dpts = NAN;

            if (frame->pts != AV_NOPTS_VALUE)
                dpts = av_q2d(mStream->time_base) * frame->pts;

            //qDebug("---  %lf",dpts);


            frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(mFormatCtx, mStream, frame);

            /* if (framedrop>0 || (framedrop && get_master_sync_type() != AV_SYNC_VIDEO_MASTER))
            {
                if (frame->pts != AV_NOPTS_VALUE) {
                    double diff = dpts - get_master_clock();
                    if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
                            diff - mVideoState->frame_last_filter_delay < 0 &&
                            mVideoState->viddec.pkt_serial == mVideoState->vidclk.serial &&
                            mVideoState->videoq.size()) {
                        mVideoState->frame_drops_early++;
                        av_frame_unref(frame);
                        got_picture = 0;
                    }
                }
            }
        */



            //qDebug("==== %d",frame->width);

            AVRational tmp;
            tmp.num=frame_rate.den;
            tmp.den=frame_rate.num;
            duration = (frame_rate.num && frame_rate.den ? av_q2d(tmp) : 0);
            pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);

            //
            /*
            convert_ctx = sws_getCachedContext(convert_ctx,
                frame->width, frame->height, (AVPixelFormat)frame->format, frame->width, frame->height,
                AV_PIX_FMT_YUV420P, 0, NULL, NULL, NULL);*/
            int width = frame->width;
            int height= frame->height;


            if(!picInit)
            {
                int ret=avpicture_alloc(&picture,AV_PIX_FMT_YUV420P,width,height);
                mVideoRender->setSize(width,height);
                //mVideoRender->start();
                if(ret==0)
                    picInit=true;

            }

            //avpicture_fill(&picture, video_buf, AV_PIX_FMT_YUV420P,width, height);


            convert_ctx = sws_getCachedContext(convert_ctx,width,height,mVideoCtx->pix_fmt,width,height,AV_PIX_FMT_YUV420P,SWS_FAST_BILINEAR, NULL,NULL,NULL);

            if (!convert_ctx) {
                qDebug("Cannot initialize the conversion context\n");

            }


            int rr=sws_scale(convert_ctx, frame->data, frame->linesize,
                             0, frame->height, picture.data,  picture.linesize);

            //qDebug("==== %f %f",dpts,mClock->getClockTime());
            if(dpts > mClock->getClockTime())
            {
                QThread::msleep((dpts-mClock->getClockTime())*1000);
            }

           mGLWidget->updateData(picture.data);
            /*while(mVideoRender->getQueueSize()>=3)
            {
                QThread::msleep(10);
            }

            mVideoRender->addPicture(picture.data,dpts);
*/
            av_frame_unref(frame);

        }
        // QThread::msleep(10);




    }
}
