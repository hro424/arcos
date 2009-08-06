#ifndef ARC_AUDIO_AC97_H
#define ARC_AUDIO_AC97_H

#include <Assert.h>
#include <Debug.h>
#include <PageAllocator.h>
#include <MemoryManager.h>
#include <Ipc.h>
#include <System.h>
#include <Audio.h>

struct AC97BufferDescriptor
{
    union {
        UInt raw[2];
        struct {
            /// Physical address of the buffer
            addr_t  address;
            /// Number of samples (not number of bytes)
            UShort  length;

            UInt    _reserved:14;

            UInt    bup:1;
            /// Interrupt on completion
            UInt    ioc:1;
        };
    };
};

struct AC97DescriptorList : public AudioBuffer
{
    // Limited to the size of L4 message registers
    static const size_t     SIZE = 32; 
    AC97BufferDescriptor    desc[SIZE];
};


class AC97Channel : public AudioChannel
{
public:
    enum channel_e {
        pcm_in = 0,
        pcm_out = 1,
        mic_in = 2,
        mic_2 = 3,
        pcm_in_2 = 4,
        spdif_out = 5,
    };

    enum op_e {
        set_bufdesc,
        get_bufdesc,
        current_index,
        set_lvi,
        get_lvi,
        set_stat,
        get_stat,
        position,
        prefetch,
        set_control,
        get_control
    };

    static const UInt NUM_CHANNELS = 6;

protected:
    stat_t Config(int config, UInt* arg)
    {
        L4_Msg_t    msg;
        L4_Word_t   regs[3];
        stat_t      err;

        regs[0] = GetType();
        assert(0 <= regs[0] && regs[0] < NUM_CHANNELS);
        regs[1] = config;
        regs[2] = *arg;

        L4_Put(&msg, MSG_EVENT_CONFIG, 3, regs, 0, 0);
        err = Ipc::Call(_server, &msg, &msg);
        if (err == ERR_NONE) {
            *arg = L4_Get(&msg, 0);
        }

        return err;
    }

    L4_Word_t Id() { return static_cast<L4_Word_t>(GetType()); }

public:
    // Because we don't use RTTI support.
    virtual AC97Channel::channel_e GetType() = 0;

    AC97Channel(L4_ThreadId_t& server) : AudioChannel(server) {}

    virtual stat_t SetBuffer(const AudioBuffer& buf)
    { return SetBuffer((const AC97DescriptorList*)&buf); }

    virtual stat_t SetBuffer(const AC97DescriptorList* buf)
    {
        L4_Word_t reg = Pager.Phys((addr_t)buf);
        return Config(AC97Channel::set_bufdesc, &reg);
    }

    /*
    virtual UInt GetVolume()
    {
        int arg;
        Config(GET_VOLUME, &arg);
        return arg;
    }

    virtual UShort GetVolumeLeft() { return (UShort)(GetVolume() >> 8); }

    virtual UShort GetVolumeRight() { return (UShort)(GetVolume() & 0xFF); }

    virtual void SetVolume(UInt vol) { Config(SET_VOLUME, &vol); }

    virtual void SetVolumeLeft(UShort vol)
    { SetVolume(GetVolumeRight() & ((UInt)vol << 8)); }

    virtual void SetVolumeRight(UShort vol)
    { SetVolume(GetVolumeLeft() & ((UInt)vol)); }
    */

    virtual UInt GetStatus()
    {
        UInt arg;
        Config(AC97Channel::get_stat, &arg);
        return arg;
    }

    virtual void SetStatus(UInt arg)
    {
        Config(AC97Channel::set_stat, &arg);
    }

    virtual UInt GetIndex()
    {
        UInt arg;
        Config(AC97Channel::get_lvi, &arg);
        return arg;
    }

    virtual void SetIndex(UInt arg)
    { Config(AC97Channel::set_lvi, &arg); }
};


class AC97PCMIn : public AC97Channel
{
public:
    AC97PCMIn(L4_ThreadId_t& server) : AC97Channel(server) {}
    virtual AC97Channel::channel_e GetType() { return AC97Channel::pcm_in; }
};


class AC97PCMOut : public AC97Channel
{
public:
    AC97PCMOut(L4_ThreadId_t& server) : AC97Channel(server) {}
    virtual AC97Channel::channel_e GetType() { return AC97Channel::pcm_out; }
};


class AC97MicIn : public AC97Channel
{
public:
    AC97MicIn(L4_ThreadId_t& server) : AC97Channel(server) {}
    virtual AC97Channel::channel_e GetType() { return AC97Channel::mic_in; }
};


class AC97Mic2 : public AC97Channel
{
public:
    AC97Mic2(L4_ThreadId_t& server) : AC97Channel(server) {}
    virtual AC97Channel::channel_e GetType() { return AC97Channel::mic_2; }
};


class AC97PCMIn2 : public AC97Channel
{
public:
    AC97PCMIn2(L4_ThreadId_t& server) : AC97Channel(server) {}
    virtual AC97Channel::channel_e GetType() { return AC97Channel::pcm_in_2; }
};


class AC97SPDIFOut : public AC97Channel
{
public:
    AC97SPDIFOut(L4_ThreadId_t& server) : AC97Channel(server) {}
    virtual AC97Channel::channel_e GetType() { return AC97Channel::spdif_out; }
};


class AC97Audio : public Audio
{
private:
    AC97Channel*    _channels[AC97Channel::NUM_CHANNELS];

public:
    AC97Audio()
    {
        for (size_t i = 0; i < AC97Channel::NUM_CHANNELS; i++) {
            _channels[i] = 0;
        }
    }

    virtual ~AC97Audio()
    {
        for (UInt i = 0; i < AC97Channel::NUM_CHANNELS; i++) {
            if (_channels[i] != 0) {
                delete _channels[i];
            }
        }
    }

    stat_t Initialize()
    {
        ENTER;
        L4_ThreadId_t   server;
        stat_t          err;

        err = NameService::Get("ac97", &server);
        if (err != ERR_NONE) {
            return err;
        }

        _channels[AC97Channel::pcm_in] = new AC97PCMIn(server);
        _channels[AC97Channel::pcm_out] = new AC97PCMOut(server);
        _channels[AC97Channel::mic_in] = new AC97MicIn(server);
        _channels[AC97Channel::mic_2] = new AC97Mic2(server);
        _channels[AC97Channel::pcm_in_2] = new AC97PCMIn2(server);
        _channels[AC97Channel::spdif_out] = new AC97SPDIFOut(server);

        EXIT;
        return ERR_NONE;
    }

    virtual AudioChannel& GetChannel(UByte type)
    {
        return *_channels[type];
    }
};

#endif // ARC_AUDIO_AC97_H
