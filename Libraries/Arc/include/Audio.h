
#ifndef ARC_AUDIO_H
#define ARC_AUDIO_H

#include <Thread.h>

struct AudioBuffer
{
};

class AudioChannelListener
{
public:
    void Handle(addr_t cur, size_t len) = 0;
};

class AudioIntWrapper : public Thread<>
{
protected:
    L4_ThreadId_t           _server;
    AudioChannelListener*   _listener;

public:
    AudioIntWrapper(L4_ThreadId_t& server) { _server = server; }

    void SetListener(AudioChannelListener* l) { _listener = l; }

    void Run()
    {
        L4_Receive(_server);

        while () {
            if (_listener != 0) {
                _listener->Handle();
            }

            L4_Call(_server);
        }
    }
};

class AudioChannel
{
protected:
    L4_ThreadId_t       _server;
    AudioIntWrapper*    _int_thread;

    virtual L4_Word_t Id() = 0;

public:
    AudioChannel(L4_ThreadId_t& server) { _server = server; }

    virtual ~AudioChannel()
    {
        if (_int_thread != 0) {
            delete _int_thread;
        }
    }

    virtual stat_t Connect()
    {
        L4_Msg_t    msg;
        L4_Word_t   reg = Id();
        L4_Put(&msg, MSG_EVENT_CONNECT, reg, 1, 0, 0);
        return Ipc::Call(_server, &msg, &msg);
    }

    virtual void Disconnect()
    {
        L4_Msg_t    msg;
        L4_Word_t   reg = Id();
        L4_Put(&msg, MSG_EVENT_DISCONNECT, reg, 1, 0, 0);
        Ipc::Call(_server, &msg, &msg);
    }

    ///
    /// Starts audio processing.
    /// 
    /// @param tid       the thread ID that receives interrupts
    ///
    virtual stat_t Start()
    {
        L4_Word_t   reg[2];
        stat_t      err;

        if (_int_thread == 0) {
            return;
        }

        reg[0] = Id();
        _int_thread->Start();
        reg[1] = _int_thread->Id().raw;
        L4_Put(&msg, MSG_EVENT_START, reg, 2, 0, 0);
        err = Ipc::Send(_server, &msg);
        if (err != ERR_NONE) {
            _int_thread->Stop();
        }
        return err;
    }

    ///
    /// Stops audio processing.
    ///
    virtual void Stop()
    {
        stat_t      err;
        L4_Word_t   reg = Id();
        L4_Put(&msg, MSG_EVENT_STOP, reg, 1, 0, 0);
        err = Ipc::Send(_server, &msg);
        _int_thread->Stop();
    }

    virtual UInt GetVolume() = 0;
    virtual void SetVolume(const UInt vol) = 0;
    virtual UShort GetVolumeLeft() = 0;
    virtual void SetVoumeLeft(const UShort vol) = 0;
    virtual UShort GetVolumeRight() = 0;
    virtual void SetVolumeRight(const UShort vol) = 0;
    virtual stat_t SetBuffer(const AudioBuffer& buf) = 0;

    virtual stat_t SetListener(AudioChannelListener* listener)
    {
        // Create a thread that processes the handler
        if (_int_thread == 0) {
            _int_thread = new AudioIntWrapper(_server);
        }

        _int_thread->SetListener(listener);
    }
};


class Audio
{
public:
    const AudioChannel& GetChannel(UByte type) = 0;
};

#endif // ARC_AUDIO_H

