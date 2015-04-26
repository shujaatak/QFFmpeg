#ifndef OPENALENGINE_H
#define OPENALENGINE_H

#include "al.h"
#include "alc.h"
#include "efx.h"
#include "alext.h"
#include "efx-creative.h"
#include "efx-presets.h"

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/avstring.h"
#include "libavutil/time.h"
}

#define NUMBUFFERS              (4)
#define	SERVICE_UPDATE_PERIOD	(20)

class QOpenALEngine : public QObject
{
    Q_OBJECT

public:

    Q_ENUMS(State)
    enum State
    {
        INITIAL=0x1011,
        PLAYING=0x1012,
        PAUSED=0x1013,
        STOPPED=0x1014
    };

    explicit QOpenALEngine(QObject *parent = 0);
    ~QOpenALEngine();

    static bool init();
    static bool deinit();

    void end();
    void write(void *data, unsigned long size,int samplerate);
    void replay();
    void seek();



    void play();
    void pause();
    void stop();
    void rewind();
    int getQueuedNum();
    int getProcessedNum();
    float getOffset();
float getBufferTimeOffset();

    void clear();
    void openEFX();
    State getState();

    void setGain(float gain);
    float getGain();

signals:

    void needData(int );

public slots:
   // void timerSlot();
private:
    QList<long> bufsize;
    QTimer *timer;

    bool isend;
    bool isBufInit;
    QThread *thread;

    static ALCdevice *pDevice;
    static ALCcontext *pContext;


    ALuint		    uiBuffers[NUMBUFFERS];
    ALuint		    uiSource;
    ALuint			uiBuffer;

    QMutex mutex;
    int bb;


    EFXEAXREVERBPROPERTIES efxReverb;
    ALuint		uiEffectSlot, uiEffect;

    ALboolean CreateAuxEffectSlot(ALuint *puiAuxEffectSlot);
    ALboolean CreateEffect(ALuint *puiEffect, ALenum eEffectType);
    ALboolean SetEFXEAXReverbProperties(EFXEAXREVERBPROPERTIES *pEFXEAXReverb, ALuint uiEffect);
    ALboolean ALFWIsEFXSupported();
    /////////


    // Effect objects
    LPALGENEFFECTS alGenEffects;
    LPALDELETEEFFECTS alDeleteEffects;
    LPALISEFFECT alIsEffect;
    LPALEFFECTI alEffecti;
    LPALEFFECTIV alEffectiv;
    LPALEFFECTF alEffectf;
    LPALEFFECTFV alEffectfv;
    LPALGETEFFECTI alGetEffecti;
    LPALGETEFFECTIV alGetEffectiv;
    LPALGETEFFECTF alGetEffectf;
    LPALGETEFFECTFV alGetEffectfv;

    //Filter objects
    LPALGENFILTERS alGenFilters;
    LPALDELETEFILTERS alDeleteFilters;
    LPALISFILTER alIsFilter;
    LPALFILTERI alFilteri;
    LPALFILTERIV alFilteriv;
    LPALFILTERF alFilterf;
    LPALFILTERFV alFilterfv;
    LPALGETFILTERI alGetFilteri;
    LPALGETFILTERIV alGetFilteriv;
    LPALGETFILTERF alGetFilterf;
    LPALGETFILTERFV alGetFilterfv;

    // Auxiliary slot object
    LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
    LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
    LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
    LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
    LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
    LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
    LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
    LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
    LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
    LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
    LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;
protected:


};

#endif // OPENALENGINE_H
