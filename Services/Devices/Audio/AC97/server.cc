///
/// @brief  AC97 audio device driver
/// @since  July 2009
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
///

#include <Debug.h>
#include <System.h>
#include <String.h>
#include <Server.h>
#include <AC97.h>
#include "ac97_server.h"

#include <PageAllocator.h>
#include <MemoryManager.h>


class AC97Server : public SelfHealingServer
{
private:
    AC97Device  _device;

protected:
    virtual stat_t IpcHandler(const L4_ThreadId_t& gid, L4_Msg_t& msg);

public:
    virtual stat_t Initialize(Int argc, char* argv[])
    {
        ENTER;
        _device.Initialize();
        _device.EnableInterrupt();
        _device.Mixer()->SetMasterVolume(0x0808);
        _device.Mixer()->SetPCMOutVolume(0x0808);
        EXIT;
        return ERR_NONE;
    }

    virtual stat_t Recover() { return ERR_NONE; }

    virtual stat_t Exit()
    {
        ENTER;
        // Send EVENT_TERMINATE
        _device.DisableInterrupt();
        _device.Finalize();
        EXIT;
        return ERR_NONE;
    }

    virtual const char* const Name() { return "ac97"; }

    stat_t HandleConnect(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        ENTER;
        L4_Word_t reg = _device.Id().raw;
        L4_Put(&msg, ERR_NONE, 1, &reg, 0, 0);
        EXIT;
        return ERR_NONE;
    }

    stat_t HandleDisconnect(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        ENTER;
        for (UInt i = 0; i < AC97Channel::NUM_CHANNELS; i++) {
            _device.Channel(i)->Deactivate();
        }
        EXIT;
        return ERR_NONE;
    }

    stat_t HandleStart(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        ENTER;
        L4_ThreadId_t       handler;
        L4_Word_t           type;
        AC97ServerChannel*  channel;
        
        type = L4_Get(&msg, 0);
        handler.raw = L4_Get(&msg, 1);
        channel = _device.Channel(type);
        if (channel != 0) {
            _device.AddListener(handler);
            channel->Activate();
        }

        //channel->SetLastValidIndex(1);

        EXIT;
        return ERR_NONE;
    }

    stat_t HandleStop(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        ENTER;
        L4_ThreadId_t       handler;
        L4_Word_t           type;
        AC97ServerChannel*  channel;

        type = L4_Get(&msg, 0);
        handler.raw = L4_Get(&msg, 1);
        channel = _device.Channel(type);
        if (channel != 0) {
            _device.DelListener(handler);
            channel->Deactivate();
        }

        EXIT;
        return Ipc::ReturnError(&msg, ERR_NONE);
    }

    stat_t HandleConfig(const L4_ThreadId_t& tid, L4_Msg_t& msg)
    {
        ENTER;
        L4_Word_t           type = L4_Get(&msg, 0);
        L4_Word_t           op = L4_Get(&msg, 1);
        AC97ServerChannel*  channel;

        channel = _device.Channel(type);
        switch (op) {
            case AC97Channel::set_bufdesc:
            {
                channel->SetBufDescBase(L4_Get(&msg, 2));
                L4_Clear(&msg);
                break;
            }
            case AC97Channel::get_stat:
            {
                L4_Word_t stat = channel->GetStatus();
                L4_Clear(&msg);
                L4_Put(&msg, ERR_NONE, 1, &stat, 0, 0);
                break;
            }
            case AC97Channel::set_stat:
            {
                channel->SetStatus(L4_Get(&msg, 2));
                channel->SetLastValidIndex(1);
                channel->SetLastValidIndex(0);
                L4_Clear(&msg);
                break;
            }
            default:
                break;
        }

        EXIT;
        return ERR_NONE;
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

