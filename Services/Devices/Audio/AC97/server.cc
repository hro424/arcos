#include <Debug.h>
#include <System.h>
#include <String.h>
#include <Server.h>
#include "ac97.h"

#include <PageAllocator.h>
#include <MemoryManager.h>


class AC97Server : public SelfHealingServer
{
private:
    AC97    _device;

protected:
    virtual stat_t IpcHandler(const L4_ThreadId_t& gid, L4_Msg_t& msg);

public:
    virtual stat_t Initialize(Int argc, char* argv[])
    {
        _device.Initialize();
        _device.EnableInterrupt();
        return ERR_NONE;
    }

    virtual stat_t Recover() { return ERR_NONE; }

    virtual stat_t Exit()
    {
        // Send EVENT_TERMINATE
        _device.DisableInterrupt();
        _device.Finalize();
        return ERR_NONE;
    }

    virtual const char* const Name() { return "ac97"; }

    stat_t HandleConnect(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    { return ERR_NONE; }

    stat_t HandleDisconnect(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        for (UInt i = 0; i < AC97Channel::NUM_CHANNELS; i++) {
            _device.Channel(i)->Deactivate();
        }
        return ERR_NONE;
    }

    stat_t HandleStart(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        L4_ThreadId_t   handler;
        L4_Word_t       type;
        AC97Channel*    channel;
        
        type = L4_Get(&msg, 0);
        handler.raw = L4_Get(&msg, 1);
        channel = _device.Channel(type);
        if (channel != 0) {
            channel->SetHandler(handler);
            channel->Activate();
        }

        return ERR_NONE;
    }

    stat_t HandleStop(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        L4_Word_t       type;
        AC97Channel*    channel;

        type = L4_Get(&msg, 0);
        channel = _device.Channel(type);
        if (channel != 0) {
            channel->Deactivate();
        }

        return ERR_NONE;
    }

    stat_t HandleConfig(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        L4_Word_t       type = L4_Get(&msg, 0);
        L4_Word_t       op = L4_Get(&msg, 1);
        stat_t          err = ERR_NONE;
        AC97Channel*    channel;

        channel = _device.Channel(type);
        switch (op) {
            case SET_BUFDESC:
                channel->SetBufDescBase(L4_Get(&msg, 2));
                break;
            default:
                break;
        }

        return err;
    }
};

SERVER_HANDLER_BEGIN(AC97Server)
SERVER_HANDLER_CONNECT(MSG_EVENT_CONNECT, HandleConnect)
SERVER_HANDLER_CONNECT(MSG_EVENT_DISCONNECT, HandleDisconnect)
SERVER_HANDLER_CONNECT(MSG_EVENT_START, HandleStart)
SERVER_HANDLER_CONNECT(MSG_EVENT_STOP, HandleStop)
SERVER_HANDLER_CONNECT(MSG_EVENT_CONFIG, HandleConfig)
SERVER_HANDLER_END

ARC_SH_SERVER(AC97Server)

