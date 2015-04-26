#include "QOpenALEngine.h"
#include "qdebug.h"

ALCdevice *QOpenALEngine::pDevice=NULL;
ALCcontext *QOpenALEngine::pContext=NULL;
EFXEAXREVERBPROPERTIES eaxBathroom = EFX_REVERB_PRESET_BATHROOM;
EFXEAXREVERBPROPERTIES eaxHangar = EFX_REVERB_PRESET_HANGAR;
extern double audio_clock;



QOpenALEngine::QOpenALEngine(QObject *parent) : QObject(parent)
{

    alGenBuffers( NUMBUFFERS, uiBuffers );
    alGenSources( 1, &uiSource );


    int iBuffersProcessed = 0;
    alGetSourcei(uiSource, AL_BUFFERS_QUEUED, &iBuffersProcessed);
    //qDebug()<<iBuffersProcessed;

    bb=0;
    isend=false;
    isBufInit=false;

    //openEFX();
    // timer=new QTimer();
    //  timer->setInterval(10);
    //  connect(timer,SIGNAL(timeout()),this,SLOT(timerSlot()));

}



QOpenALEngine::~QOpenALEngine()
{
    // Clean up buffers and sources
    alDeleteSources( 1, &uiSource );
    alDeleteBuffers( NUMBUFFERS, uiBuffers );
}

void QOpenALEngine::setGain(float gain)
{
    alSourcef(uiSource, AL_GAIN, gain);
}

float QOpenALEngine::getGain()
{
    float gain;
    alGetSourcef(uiSource, AL_GAIN,&gain);
    return gain;
}

bool QOpenALEngine::init()
{

    const ALCchar* pDeviceNames;

    if(alcIsExtensionPresent( NULL, "ALC_ENUMERATE_ALL_EXT"))
    {
        pDeviceNames = alcGetString( NULL, ALC_ALL_DEVICES_SPECIFIER  );

    }
    else if(alcIsExtensionPresent( NULL, "ALC_ENUMERATION_EXT" ))
    {
        pDeviceNames = alcGetString( NULL, ALC_DEVICE_SPECIFIER );

    }




    bool bReturn=false;
    pDevice = alcOpenDevice(pDeviceNames);
    if (pDevice)
    {
        pContext = alcCreateContext(pDevice, NULL);
        if (pContext)
        {
            alcMakeContextCurrent(pContext);
            bReturn = true;
        }
        else
        {
            alcCloseDevice(pDevice);
        }
    }


    return bReturn;

}

bool QOpenALEngine::deinit()
{
      qDebug("QOpenALEngine::deinit");
    if (pDevice)
    {

        return   alcCloseDevice(pDevice);

    }
    return false;
}

void QOpenALEngine::write(void *data, unsigned long size,int samplerate)
{
    /*  alSourceUnqueueBuffers(uiSource, 1, &uiBuffer);

    alBufferData(uiBuffer, AL_FORMAT_STEREO16, data, size, 44100);
    alSourceQueueBuffers(uiSource, 1, &uiBuffer);
    */


    if(data==0 || size<=0)
    {
        return;
    }

    //qDebug()<<QString("%1").arg(bufsize.length());


    int iBuffers = 0;
    int iBuffersProcessed = 0;


    alGetSourcei(uiSource, AL_BUFFERS_QUEUED, &iBuffers);

    alGetSourcei(uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
    //qDebug()<<QString("%1 %2").arg(iBuffers).arg(iBuffersProcessed);


    if(iBuffers==NUMBUFFERS)
    {




        //

        if(iBuffersProcessed)
        {
            bufsize.pop_front();
            // audio_clock+=(double)bufsize.at(0)/
            //        (2*44100*av_get_bytes_per_sample(AV_SAMPLE_FMT_S16));
            // qDebug()<<QString("%1").arg(audio_clock);
            alSourceUnqueueBuffers(uiSource, 1, &uiBuffer);
            alBufferData(uiBuffer, AL_FORMAT_STEREO16, data, size, samplerate);
            alSourceQueueBuffers(uiSource, 1, &uiBuffer);
            bufsize.append(size);
        }
    }
    else
    {
        alBufferData(uiBuffers[iBuffers], AL_FORMAT_STEREO16, data, size, samplerate);
        alSourceQueueBuffers(uiSource, 1, &uiBuffers[iBuffers]);
        bufsize.append(size);
    }
    //mutex.lock();
    if(bb>0)bb--;
    // mutex.unlock();


}


void QOpenALEngine::end()
{

    isend=true;

}


void QOpenALEngine::seek()
{
    timer->stop();
    alSourceStop(uiSource);
    alSourcei(uiSource, AL_BUFFER, 0);

}

void QOpenALEngine::replay()
{

    //state=Idle;
    //timer->start();
}


void QOpenALEngine::openEFX(){
    if (ALFWIsEFXSupported())
    {
        if (CreateAuxEffectSlot(&uiEffectSlot))
        {
            if (CreateEffect(&uiEffect, AL_EFFECT_EAXREVERB))
            {
                // ConvertReverbParameters(&eaxBathroom, &efxReverb);
                efxReverb=eaxBathroom;

                // Set the Effect parameters
                if (!SetEFXEAXReverbProperties(&efxReverb, uiEffect))
                    return;
                // Load Effect into Auxiliary Effect Slot
                alAuxiliaryEffectSloti(uiEffectSlot, AL_EFFECTSLOT_EFFECT, uiEffect);
                // Enable (non-filtered) Send from Source to Auxiliary Effect Slot
                alSource3i(uiSource, AL_AUXILIARY_SEND_FILTER, uiEffectSlot, 0, AL_FILTER_NULL);
            }
        }
    }
}


void QOpenALEngine::play()
{
    alSourcePlay(uiSource);
}

void QOpenALEngine::stop()
{
    alSourceStop(uiSource);
}
void QOpenALEngine::pause()
{
    alSourcePause(uiSource);
}
void QOpenALEngine::rewind()
{
    alSourceRewind(uiSource);
}

int QOpenALEngine::getQueuedNum()
{

    int iQueuedBuffers = 0;
    alGetSourcei(uiSource, AL_BUFFERS_QUEUED, &iQueuedBuffers);
    return iQueuedBuffers;
}
int QOpenALEngine::getProcessedNum()
{

    int iBuffersProcessed = 0;
    alGetSourcei(uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);
    return iBuffersProcessed;
}

float QOpenALEngine::getBufferTimeOffset()
{
    float time=0;
    alGetSourcef(uiSource, AL_SEC_OFFSET, &time);
    return time;
}

float QOpenALEngine::getOffset()
{
    //AL_SAMPLE_OFFSET
    //AL_BYTE_OFFSET

    float iOffset = 0;
    alGetSourcef(uiSource, AL_BYTE_OFFSET, &iOffset);
    return iOffset;
}

void QOpenALEngine::clear()
{
    alSourcei(uiSource, AL_BUFFER, 0);
}

QOpenALEngine::State QOpenALEngine::getState()
{
    int iState;
    alGetSourcei(uiSource, AL_SOURCE_STATE, &iState);

    return (State)iState;
}



ALboolean QOpenALEngine::CreateAuxEffectSlot(ALuint *puiAuxEffectSlot)
{
    ALboolean bReturn = AL_FALSE;

    // Clear AL Error state
    alGetError();

    // Generate an Auxiliary Effect Slot
    alGenAuxiliaryEffectSlots(1, puiAuxEffectSlot);
    if (alGetError() == AL_NO_ERROR)
        bReturn = AL_TRUE;

    return bReturn;
}

ALboolean QOpenALEngine::CreateEffect(ALuint *puiEffect, ALenum eEffectType)
{
    ALboolean bReturn = AL_FALSE;

    if (puiEffect)
    {
        // Clear AL Error State
        alGetError();

        // Generate an Effect
        alGenEffects(1, puiEffect);
        if (alGetError() == AL_NO_ERROR)
        {
            // Set the Effect Type
            alEffecti(*puiEffect, AL_EFFECT_TYPE, eEffectType);
            if (alGetError() == AL_NO_ERROR)
                bReturn = AL_TRUE;
            else
                alDeleteEffects(1, puiEffect);
        }
    }

    return bReturn;
}



ALboolean QOpenALEngine::SetEFXEAXReverbProperties(EFXEAXREVERBPROPERTIES *pEFXEAXReverb, ALuint uiEffect)
{
    ALboolean bReturn = AL_FALSE;

    if (pEFXEAXReverb)
    {
        // Clear AL Error code
        alGetError();

        alEffectf(uiEffect, AL_EAXREVERB_DENSITY, pEFXEAXReverb->flDensity);
        alEffectf(uiEffect, AL_EAXREVERB_DIFFUSION, pEFXEAXReverb->flDiffusion);
        alEffectf(uiEffect, AL_EAXREVERB_GAIN, pEFXEAXReverb->flGain);
        alEffectf(uiEffect, AL_EAXREVERB_GAINHF, pEFXEAXReverb->flGainHF);
        alEffectf(uiEffect, AL_EAXREVERB_GAINLF, pEFXEAXReverb->flGainLF);
        alEffectf(uiEffect, AL_EAXREVERB_DECAY_TIME, pEFXEAXReverb->flDecayTime);
        alEffectf(uiEffect, AL_EAXREVERB_DECAY_HFRATIO, pEFXEAXReverb->flDecayHFRatio);
        alEffectf(uiEffect, AL_EAXREVERB_DECAY_LFRATIO, pEFXEAXReverb->flDecayLFRatio);
        alEffectf(uiEffect, AL_EAXREVERB_REFLECTIONS_GAIN, pEFXEAXReverb->flReflectionsGain);
        alEffectf(uiEffect, AL_EAXREVERB_REFLECTIONS_DELAY, pEFXEAXReverb->flReflectionsDelay);
        alEffectfv(uiEffect, AL_EAXREVERB_REFLECTIONS_PAN, pEFXEAXReverb->flReflectionsPan);
        alEffectf(uiEffect, AL_EAXREVERB_LATE_REVERB_GAIN, pEFXEAXReverb->flLateReverbGain);
        alEffectf(uiEffect, AL_EAXREVERB_LATE_REVERB_DELAY, pEFXEAXReverb->flLateReverbDelay);
        alEffectfv(uiEffect, AL_EAXREVERB_LATE_REVERB_PAN, pEFXEAXReverb->flLateReverbPan);
        alEffectf(uiEffect, AL_EAXREVERB_ECHO_TIME, pEFXEAXReverb->flEchoTime);
        alEffectf(uiEffect, AL_EAXREVERB_ECHO_DEPTH, pEFXEAXReverb->flEchoDepth);
        alEffectf(uiEffect, AL_EAXREVERB_MODULATION_TIME, pEFXEAXReverb->flModulationTime);
        alEffectf(uiEffect, AL_EAXREVERB_MODULATION_DEPTH, pEFXEAXReverb->flModulationDepth);
        alEffectf(uiEffect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, pEFXEAXReverb->flAirAbsorptionGainHF);
        alEffectf(uiEffect, AL_EAXREVERB_HFREFERENCE, pEFXEAXReverb->flHFReference);
        alEffectf(uiEffect, AL_EAXREVERB_LFREFERENCE, pEFXEAXReverb->flLFReference);
        alEffectf(uiEffect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, pEFXEAXReverb->flRoomRolloffFactor);
        alEffecti(uiEffect, AL_EAXREVERB_DECAY_HFLIMIT, pEFXEAXReverb->iDecayHFLimit);

        if (alGetError() == AL_NO_ERROR)
            bReturn = AL_TRUE;
    }

    return bReturn;
}



ALboolean QOpenALEngine::ALFWIsEFXSupported()
{
    ALCdevice *pDevice = NULL;
    ALCcontext *pContext = NULL;
    ALboolean bEFXSupport = AL_FALSE;

    pContext = alcGetCurrentContext();
    pDevice = alcGetContextsDevice(pContext);

    if (alcIsExtensionPresent(pDevice, (ALCchar*)ALC_EXT_EFX_NAME))
    {
        // Get function pointers
        alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
        alDeleteEffects = (LPALDELETEEFFECTS )alGetProcAddress("alDeleteEffects");
        alIsEffect = (LPALISEFFECT )alGetProcAddress("alIsEffect");
        alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
        alEffectiv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
        alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
        alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
        alGetEffecti = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
        alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
        alGetEffectf = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
        alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");
        alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
        alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
        alIsFilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
        alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
        alFilteriv = (LPALFILTERIV)alGetProcAddress("alFilteriv");
        alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");
        alFilterfv = (LPALFILTERFV)alGetProcAddress("alFilterfv");
        alGetFilteri = (LPALGETFILTERI )alGetProcAddress("alGetFilteri");
        alGetFilteriv= (LPALGETFILTERIV )alGetProcAddress("alGetFilteriv");
        alGetFilterf = (LPALGETFILTERF )alGetProcAddress("alGetFilterf");
        alGetFilterfv= (LPALGETFILTERFV )alGetProcAddress("alGetFilterfv");
        alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
        alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
        alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
        alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
        alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
        alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
        alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
        alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
        alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
        alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
        alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetAuxiliaryEffectSlotfv");

        if (alGenEffects &&	alDeleteEffects && alIsEffect && alEffecti && alEffectiv &&	alEffectf &&
                alEffectfv && alGetEffecti && alGetEffectiv && alGetEffectf && alGetEffectfv &&	alGenFilters &&
                alDeleteFilters && alIsFilter && alFilteri && alFilteriv &&	alFilterf && alFilterfv &&
                alGetFilteri &&	alGetFilteriv && alGetFilterf && alGetFilterfv && alGenAuxiliaryEffectSlots &&
                alDeleteAuxiliaryEffectSlots &&	alIsAuxiliaryEffectSlot && alAuxiliaryEffectSloti &&
                alAuxiliaryEffectSlotiv && alAuxiliaryEffectSlotf && alAuxiliaryEffectSlotfv &&
                alGetAuxiliaryEffectSloti && alGetAuxiliaryEffectSlotiv && alGetAuxiliaryEffectSlotf &&
                alGetAuxiliaryEffectSlotfv)
            bEFXSupport = AL_TRUE;
    }

    return bEFXSupport;
}
