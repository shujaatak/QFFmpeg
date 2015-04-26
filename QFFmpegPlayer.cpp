#include "QFFmpegPlayer.h"
#include <QDebug>

QFFmpegPlayer::QFFmpegPlayer(QObject *parent) : QThread(parent)
{
    mAudioDecoder=new QFFmpegAudioDecoder();
    mVideoDecoder=new QFFmpegVideoDecoder();
    mClock=new QFFmpegClock();
    mClock->setAudioDecoder(mAudioDecoder);

    mVideoDecoder->setClock(mClock);
    mAudioDecoder->setClock(mClock);


    abort=false;
}

QFFmpegPlayer::~QFFmpegPlayer()
{
    delete mAudioDecoder;
}

///static
void QFFmpegPlayer::Init()
{
    av_register_all();
    avformat_network_init();
}

void QFFmpegPlayer::destroy()
{
    qDebug("[][][][");
    abort=true;

}

void QFFmpegPlayer::play()
{

    this->start();
}

void QFFmpegPlayer::setGLWidget(QFFmpegGLWidget *gl)
{
    mVideoDecoder->setGLWidget(gl);

}

int is_realtime(AVFormatContext *s)
{
    if(   !strcmp(s->iformat->name, "rtp")
          || !strcmp(s->iformat->name, "rtsp")
          || !strcmp(s->iformat->name, "sdp")
          )
        return 1;

    if(s->pb && (   !strncmp(s->filename, "rtp:", 4)
                    || !strncmp(s->filename, "udp:", 4)
                    )
            )
        return 1;
    return 0;
}


static const char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};


void QFFmpegPlayer::run()
{


    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];
    AVPacket pkt1, *pkt = &pkt1;
    int64_t stream_start_time;
    int pkt_in_play_range = 0;
    AVDictionaryEntry *t;
    AVDictionary **opts;
    int orig_nb_streams;
    QMutex *wait_mutex = new QMutex();
    int scan_all_pmts_set = 0;
    int64_t pkt_ts;
    float max_frame_duration;
    int realtime;

    AVFormatContext *formatCtx=avformat_alloc_context();
    AVFrame *frame=av_frame_alloc();


    int video_stream = -1;
    int audio_stream = -1;
    int subtitle_stream = -1;
    int eof = 0;



    formatCtx->interrupt_callback.callback = DecodeInterruptCallback;
    formatCtx->interrupt_callback.opaque = this;

    memset(st_index, -1, sizeof(st_index));
    ret = avformat_open_input(&formatCtx, "D:/Music/mww.mp4",NULL, NULL);
    if (ret < 0) {
        qDebug("avformat_open_input fail!");
        return;
    }

    // av_dump_format(formatCtx, 0, 0, 0);

    av_format_inject_global_side_data(formatCtx);


    orig_nb_streams = formatCtx->nb_streams;


    if (formatCtx->pb)
        formatCtx->pb->eof_reached = 0;


    max_frame_duration = (formatCtx->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;



    realtime = is_realtime(formatCtx);




    for (i = 0; i < formatCtx->nb_streams; i++) {
        AVStream *st = formatCtx->streams[i];
        AVMediaType type = st->codec->codec_type;
        //st->discard = AVDISCARD_ALL;
        if (st_index[type] == -1)
            st_index[type] = i;
    }

    for (i = 0; i < AVMEDIA_TYPE_NB; i++) {
        if (st_index[i] == -1) {
            st_index[i] = INT_MAX;
        }
    }


    st_index[AVMEDIA_TYPE_VIDEO] =
            av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO,
                                st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);

    st_index[AVMEDIA_TYPE_AUDIO] =
            av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO,
                                st_index[AVMEDIA_TYPE_AUDIO],
                                st_index[AVMEDIA_TYPE_VIDEO],
                                NULL, 0);

    st_index[AVMEDIA_TYPE_SUBTITLE] =
            av_find_best_stream(formatCtx, AVMEDIA_TYPE_SUBTITLE,
                                st_index[AVMEDIA_TYPE_SUBTITLE],
                                (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                     st_index[AVMEDIA_TYPE_AUDIO] :
                                     st_index[AVMEDIA_TYPE_VIDEO]),
                                NULL, 0);
    video_stream=st_index[AVMEDIA_TYPE_VIDEO];
    audio_stream=st_index[AVMEDIA_TYPE_AUDIO];

    qDebug("video:%d audio:%d subtitle:%d",st_index[AVMEDIA_TYPE_VIDEO],st_index[AVMEDIA_TYPE_AUDIO],st_index[AVMEDIA_TYPE_SUBTITLE]);


    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVStream *st = formatCtx->streams[st_index[AVMEDIA_TYPE_VIDEO]];
        AVCodecContext *avctx = st->codec;
        AVRational sar = av_guess_sample_aspect_ratio(formatCtx, st, NULL);
        if (avctx->width)
        {
            qDebug("window size:%d*%d SAR:%d/%d",avctx->width,avctx->height,sar.num,sar.den);
        }

    }


    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
        AVCodecContext *mAudioCtx = formatCtx->streams[st_index[AVMEDIA_TYPE_AUDIO]]->codec;

        AVCodec *mAudioCodec = avcodec_find_decoder(mAudioCtx->codec_id);
        if(mAudioCodec->capabilities & CODEC_CAP_DR1)
            mAudioCtx->flags |= CODEC_FLAG_EMU_EDGE;

        ret = avcodec_open2(mAudioCtx, mAudioCodec, NULL);
        if(ret<0)
            return;

        formatCtx->streams[st_index[AVMEDIA_TYPE_AUDIO]]->discard = AVDISCARD_DEFAULT;

        //sample_rate    = mAudioCtx->sample_rate;
        //nb_channels    = mAudioCtx->channels;
        //channel_layout = mAudioCtx->channel_layout;

        //开启音频解码
        mAudioDecoder->setCodecCtx(mAudioCtx);
        mAudioDecoder->setStream(formatCtx->streams[st_index[AVMEDIA_TYPE_AUDIO]]);
        mAudioDecoder->start();
    }


    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVCodecContext *mVideoCtx = formatCtx->streams[st_index[AVMEDIA_TYPE_VIDEO]]->codec;

        AVCodec *mVideoCodec = avcodec_find_decoder(mVideoCtx->codec_id);
        //if(mVideoCodec->capabilities & CODEC_CAP_DR1)
        //    mVideoCodec->flags |= CODEC_FLAG_EMU_EDGE;

        ret = avcodec_open2(mVideoCtx, mVideoCodec, NULL);
        if(ret<0)
            return;
        mVideoDecoder->setFormatCtx(formatCtx);
        mVideoDecoder->setCodecCtx(mVideoCtx);
        mVideoDecoder->setStream(formatCtx->streams[st_index[AVMEDIA_TYPE_VIDEO]]);
        mVideoDecoder->start();

    }


    if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {


    }


    while(true)
    {


        if(abort)
        {
            qDebug("++++++++++");
            //this->deleteLater();

            return;
        }

        if(mAudioDecoder->getQueueLength() >10 && mVideoDecoder->getQueueLength() >10)
        {
            QThread::msleep(20);
            continue;
        }


        ret = av_read_frame(formatCtx, pkt);
        //qDebug("av_read_frame:%d",pkt->stream_index);



        if (ret < 0)
        {
            if ((ret == AVERROR_EOF || avio_feof(formatCtx->pb)) && !eof)
            {
                /*if (mVideoState->video_stream >= 0)
                mVideoState->videoq.packet_queue_put_nullpacket(mVideoState->video_stream);
            if (mVideoState->audio_stream >= 0)
                mVideoState->audioq.packet_queue_put_nullpacket(mVideoState->audio_stream);
            if (mVideoState->subtitle_stream >= 0)
                mVideoState->subtitleq.packet_queue_put_nullpacket(mVideoState->subtitle_stream);*/
                eof = 1;
            }
            if (formatCtx->pb && formatCtx->pb->error)
                return;

        }
        else
        {

            eof = 0;

            int64_t stream_start_time = formatCtx->streams[pkt->stream_index]->start_time;
            pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;
            //qDebug(".............:%d",pkt->stream_index);

            if (pkt->stream_index == audio_stream )
            {
                mAudioDecoder->addPacket(*pkt);
            }
            else if (pkt->stream_index == video_stream)
                //&& !(formatCtx->streams[pkt->stream_index]->disposition & AV_DISPOSITION_ATTACHED_PIC))
            {
                mVideoDecoder->addPacket(*pkt);


            }
            else if (pkt->stream_index == subtitle_stream)
            {
                av_free_packet(pkt);
            }
            else
            {
                av_free_packet(pkt);
            }

            //qDebug("else end");
        }

    }

}


int QFFmpegPlayer::DecodeInterruptCallback(void *ctx)
{

    QFFmpegPlayer *rt = (QFFmpegPlayer *)ctx;

    return rt->abort;

}

