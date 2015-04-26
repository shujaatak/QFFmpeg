#ifndef QFFMPEGPLAYER_H
#define QFFMPEGPLAYER_H

#include <QThread>
#include "QFFmpegAudioDecoder.h"
#include "QFFmpegVideoDecoder.h"
#include "QFFmpegGLWidget.h"
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


class QFFmpegPlayer : public QThread
{
    Q_OBJECT
public:
    explicit QFFmpegPlayer(QObject *parent = 0);
    ~QFFmpegPlayer();


    static void Init();
    static int DecodeInterruptCallback(void *ctx);

    void play();

    void destroy();


    void setGLWidget(QFFmpegGLWidget *gl);
signals:

public slots:



protected:
    void run();

private:
    QFFmpegAudioDecoder *mAudioDecoder;
    QFFmpegVideoDecoder *mVideoDecoder;
    QFFmpegClock *mClock;
    bool abort;

};

#endif // QFFMPEGPLAYER_H
