
#ifndef ARC_AUDIO_H
#define ARC_AUDIO_H

#include <Debug.h>
#include <Ipc.h>
#include <NameService.h>
#include <Thread.h>

struct AudioBuffer
{
    // no members
};

class AudioChannelListener
{
public:
    virtual void Handle() = 0;
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
        ENTER;
        L4_MsgTag_t tag;

        for (;;) {
            tag = L4_Receive(_server);
            if (L4_IpcFailed(tag)) {
                break;
            }

            if (L4_Label(tag) == MSG_EVENT_NOTIFY) {
                if (_listener != 0) {
                    _listener->Handle();
                }
            }
            else if (L4_Label(tag) == MSG_EVENT_TERMINATE) {
                break;
            }
        }
        EXIT;
    }
};

class AudioChannel
{
protected:
    L4_ThreadId_t       _server;
    L4_ThreadId_t       _int_server;
    AudioIntWrapper*    _int_thread;

    virtual L4_Word_t Id() = 0;

public:
    AudioChannel(L4_ThreadId_t& server) : _server(server) { }

    virtual ~AudioChannel()
    {
        ENTER;
        if (_int_thread != 0) {
            if (_int_thread->IsRunning()) {
                _int_thread->Cancel();
            }
            delete _int_thread;
        }
        EXIT;
    }

    virtual stat_t Connect()
    {
        ENTER;
        stat_t      err;
        L4_Msg_t    msg;
        L4_Word_t   reg = Id();
        L4_Put(&msg, MSG_EVENT_CONNECT, 1, &reg, 0, 0);
        err = Ipc::Call(_server, &msg, &msg);
        if (err == ERR_NONE) {
            _int_server.raw = L4_Get(&msg, 0);
        }
        EXIT;
        return err;
    }

    virtual void Disconnect()
    {
        ENTER;
        L4_Msg_t    msg;
        L4_Word_t   reg = Id();
        L4_Put(&msg, MSG_EVENT_DISCONNECT, 1, &reg, 0, 0);
        EXIT;
        Ipc::Call(_server, &msg, &msg);
    }

    ///
    /// Starts audio processing.
    /// 
    /// @param tid       the thread ID that receives interrupts
    ///
    virtual stat_t Start()
    {
        ENTER;
        L4_Word_t   reg[2];
        L4_Msg_t    msg;
        stat_t      err;

        if (_int_thread == 0) {
            return ERR_NOT_FOUND;
        }

        reg[0] = Id();
        _int_thread->Start();
        reg[1] = _int_thread->Id().raw;
        L4_Put(&msg, MSG_EVENT_START, 2, reg, 0, 0);
        err = Ipc::Send(_server, &msg);
        if (err != ERR_NONE) {
            _int_thread->Cancel();
        }
        EXIT;
        return err;
    }

    ///
    /// Stops audio processing.
    ///
    virtual void Stop()
    {
        ENTER;
        L4_Word_t   reg[2];
        L4_Msg_t    msg;
        stat_t      err;
        reg[0] = Id();
        reg[1] = _int_thread->Id().raw;
        L4_Put(&msg, MSG_EVENT_STOP, 2, reg, 0, 0);
        err = Ipc::Send(_server, &msg);
        _int_thread->Cancel();
        EXIT;
    }

    /*
    virtual UInt GetVolume() = 0;
    virtual void SetVolume(const UInt vol) = 0;
    virtual UShort GetVolumeLeft() = 0;
    virtual void SetVoumeLeft(const UShort vol) = 0;
    virtual UShort GetVolumeRight() = 0;
    virtual void SetVolumeRight(const UShort vol) = 0;
    */
    virtual stat_t SetBuffer(const AudioBuffer& buf) = 0;

    virtual void SetListener(AudioChannelListener* listener)
    {
        ENTER;
        // Create a thread that processes the handler
        if (_int_thread == 0) {
            _int_thread = new AudioIntWrapper(_int_server);
        }

        _int_thread->SetListener(listener);
        EXIT;
    }
};


class Audio
{
public:
    virtual AudioChannel& GetChannel(UByte type) = 0;
};

#endif // ARC_AUDIO_H

