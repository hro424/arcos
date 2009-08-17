
#ifndef ARC_AUDIO_H
#define ARC_AUDIO_H

#include <Debug.h>
#include <Ipc.h>
#include <NameService.h>
#include <Thread.h>

#include <Interrupt.h>

struct AudioBuffer
{
    // no members
};

class AudioChannelListener
{
public:
    virtual Int Handle() = 0;
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

            //DOUT("from %.8lX\n", _server.raw);
            if (L4_Label(tag) == MSG_EVENT_NOTIFY) {
                if (_listener != 0) {
                    if (_listener->Handle() != 0) {
                        return;
                    }
                }
            }
            else if (L4_Label(tag) == MSG_EVENT_TERMINATE) {
                break;
            }
            L4_Send(_server);
        }
        EXIT;
    }
};

///

class InterruptServer : public InterruptHandler
{
private:
    L4_ThreadId_t   _listener;

public:
    static const UInt   IRQ_AC97 = 12;

    InterruptServer() : _listener(L4_nilthread) {}

    stat_t EnableInterrupt()
    {
        stat_t err;
        err = InterruptManager::Instance()->Register(this, IRQ_AC97);
        return err;
    }

    void DisableInterrupt()
    { InterruptManager::Instance()->Deregister(IRQ_AC97); }

    void AddListener(L4_ThreadId_t tid) { _listener = tid; }

    void DelListener() { _listener = L4_nilthread; }

    void HandleInterrupt(L4_ThreadId_t tid, L4_Msg_t* msg)
    {
        L4_Msg_t event;

        L4_Put(&event, MSG_EVENT_NOTIFY, 0, 0, 0, 0);
        Ipc::Call(_listener, &event, &event);
    }
};


class AudioChannel
{
protected:
    L4_ThreadId_t       _server;
    L4_ThreadId_t       _int_server;
    AudioIntWrapper*    _int_thread;

    virtual L4_Word_t Id() = 0;

    InterruptServer     _ints;

public:
    AudioChannel(L4_ThreadId_t& server) : _server(server) {}

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

public:
    virtual stat_t Connect()
    {
        ENTER;
        stat_t      err;
        L4_Msg_t    msg;
        L4_Word_t   reg = Id();
        L4_Put(&msg, MSG_EVENT_CONNECT, 1, &reg, 0, 0);
        err = Ipc::Call(_server, &msg, &msg);
        /* Also see AC97/server.cc
        if (err == ERR_NONE) {
            _int_server.raw = L4_Get(&msg, 0);
        }
        */
        _ints.EnableInterrupt();
        _int_server = _ints.Id();
        EXIT;
        return err;
    }

    virtual void Disconnect()
    {
        ENTER;
        L4_Msg_t    msg;
        L4_Word_t   reg = Id();
        L4_Put(&msg, MSG_EVENT_DISCONNECT, 1, &reg, 0, 0);
        _ints.DisableInterrupt();
        _int_server = L4_nilthread;
        Ipc::Call(_server, &msg, &msg);
        EXIT;
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

        _ints.AddListener(_int_thread->Id());

        reg[0] = Id();
        _int_thread->Start();
        reg[1] = _int_thread->Id().raw;
        L4_Put(&msg, MSG_EVENT_START, 2, reg, 0, 0);
        err = Ipc::Send(_server, &msg);
        if (err != ERR_NONE && _int_thread->IsRunning()) {
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

        _ints.DelListener();

        L4_Word_t   reg[2];
        L4_Msg_t    msg;
        stat_t      err;
        reg[0] = Id();
        reg[1] = _int_thread->Id().raw;
        L4_Put(&msg, MSG_EVENT_STOP, 2, reg, 0, 0);
        err = Ipc::Send(_server, &msg);

        if (_int_thread->IsRunning()) {
            _int_thread->Cancel();
        }

        EXIT;
    }

    virtual void Join() { _int_thread->Join(); }

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

