#ifndef QFFMPEGAUDIODECODER_H
#define QFFMPEGAUDIODECODER_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>
#include "QOpenALEngine.h"
#include <QTimer>
#include <QQueue>
#include "QFFmpegClock.h"

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

typedef struct FFAVPacket {
    AVPacket pkt;
    int serial;
} FFAVPacket;


class QFFmpegAudioDecoder : public QThread
{
    Q_OBJECT
public:
    explicit QFFmpegAudioDecoder(QObject *parent = 0);
    ~QFFmpegAudioDecoder();


    void setCodecCtx(AVCodecContext *mAudioCtx);
    void setStream(AVStream *mStream);
    void addPacket(AVPacket pkt);
    int getQueueLength();

    void setClock(QFFmpegClock *clock);
    double getAudioTime();
signals:

public slots:
private:
    AVCodecContext *mAudioCtx;
    QQueue<FFAVPacket> queue;
    QMutex mutex;
    QSemaphore sema;
    SwrContext *swr_ctx;
    QOpenALEngine *al;
    QTimer *timer;
    AVStream *mStream;

    double mAudioClock;
    double mBufferStartTime;
    QQueue<double> mBufferTimes;

    QFFmpegClock *mClock;
protected:
    void run();
private slots:
    void timerUpdate();
    void timerStart();
};

#endif // QFFMPEGAUDIODECODER_H
