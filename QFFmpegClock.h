#ifndef QFFMPEGCLOCK_H
#define QFFMPEGCLOCK_H

#include <QThread>
extern "C"{
#include "libavutil/time.h"
}
class QFFmpegAudioDecoder;

class QFFmpegClock : public QThread
{
    Q_OBJECT
public:

    enum
    {
        AudioClock,
        ExternClock,
    }ClockMasterType;

    explicit QFFmpegClock(QObject *parent = 0);
    ~QFFmpegClock();

    void setAudioDecoder(QFFmpegAudioDecoder *audioDecoder);

    double getClockTime();
signals:

public slots:
private:
    QFFmpegAudioDecoder *mAudioDecoder;

    double starttime;
    double time;
protected:
    void run();

};

#endif // QFFMPEGCLOCK_H
