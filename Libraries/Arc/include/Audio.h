#ifndef ARC_AUDIO_H
#define ARC_AUDIO_H

struct AudioBuffer
{
}

struct AC97BufferDescriptor
{
    addr_t  address;
    UInt    desc;
};

struct AC97Buffer : public AudioBuffer
{
    static const UInt       SIZE = 32
    AC97BufferDescriptor    desc[SIZE];
};


class AudioComponent
{
public:
    static const UByte  NUM_VOLUMES = 4;
    static const UByte  MASTER = 1;
    static const UByte  PCM_IN = 2;
    static const UByte  PCM_OUT = 3;

    /**
     * Starts audio processing.
     *
     * @param tid       the thread ID that receives interrupts
     */
    virtual void Start(L4_ThreadId_t& tid) {}

    /**
     * Stops audio processing.
     */
    virtual void Stop() {}


    virtual UInt GetVolume() = 0;
    virtual void SetVolume(UInt vol) = 0;
    virtual UShort GetVolumeLeft() = 0;
    virtual void SetVoumeLeft(UShort vol) = 0;
    virtual UShort GetVolumeRight() = 0;
    virtual void SetVolumeRight(UShort vol) = 0;
    virtual UByte GetType() = 0;

    virtual SetBuffer(const AudioBuffer& buf) {}
};

class MasterComponent : public AudioComponent
{
public:
    virtual UInt GetVolume() {}
    virtual void SetVolume(UInt vol) {}
    virtual UShort GetVolumeLeft() {}
    virtual void SetVolumeLeft(UShort vol) {}
    virtual UShort GetVolumeRight() {}
    virtual void SetVolumeRight(UShort vol) {}
    virtual UByte GetType() { return AudioVolume::MASTER; }
};

class PCMOutComponent : public AudioComponent
{
public:
    virtual void Start(L4_ThreadId_t& tid);
    virtual void Stop();
    virtual UInt GetVolume() {}
    virtual void SetVolume(UInt vol) {}
    virtual UShort GetVolumeLeft() {}
    virtual void SetVolumeLeft(UShort vol) {}
    virtual UShort GetVolumeRight() {}
    virtual void SetVolumeRight(UShort vol) {}
    virtual UByte GetType() { return AudioVolume::PCM_OUT; }

    virtual void SetBuffer(const AudioBuffer& buf);
}


class Audio
{
private:
    AudioComponent* _comp[AudioVolume::NUM_VOLUMES];

public:
    Audio() {
        _comp[AudioVolume::MASTER] = new MasterVolume();
        _comp[AudioVolume::PCM_OUT] = new PCMOutVolume();
    }

    virtual ~Audio() {
        for (int i = 0; i < AudioVolume::NUM_VOLUMES; i++) {
            if (_comp[i] != 0) {
                delete _comp[i];
                _comp[i] = 0;
            }
        }
    }

    const AudioComponent& GetComponent(UByte type)
    { return *_comp[type]; }
};

#endif // ARC_AUDIO_H

