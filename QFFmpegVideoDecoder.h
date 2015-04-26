#ifndef QFFMPEGVIDEODECODER_H
#define QFFMPEGVIDEODECODER_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>

#include "QFFmpegGLWidget.h"
#include "QFFmpegClock.h"
#include "QFFmpegVideoRender.h"

extern "C"{
#include "libavutil/avstring.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavutil/time.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"
}

class QFFmpegVideoDecoder : public QThread
{
    Q_OBJECT
public:
    explicit QFFmpegVideoDecoder(QObject *parent = 0);
    ~QFFmpegVideoDecoder();


    void setCodecCtx(AVCodecContext *mVideoCtx);
    void addPacket(AVPacket pkt);
    int getQueueLength();

    void setFormatCtx(AVFormatContext *mFormatCtx);
    void setStream(AVStream *mStream);

    void setGLWidget(QFFmpegGLWidget *gl);
    void setClock(QFFmpegClock *clock);
signals:

public slots:
private:
    QQueue<AVPacket> queue;
    QMutex *mutex;
    QSemaphore *sema;
    AVCodecContext *mVideoCtx;
    AVFormatContext *mFormatCtx;
    AVStream *mStream;

    QFFmpegGLWidget *mGLWidget;

    SwsContext *convert_ctx;
    AVPicture picture;
    bool picInit;
    QFFmpegClock *mClock;
    QFFmpegVideoRender *mVideoRender;
protected:
    void run();
};

#endif // QFFMPEGVIDEODECODER_H
