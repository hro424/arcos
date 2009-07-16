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

        //_device.ResetPCMOut();

        _device.SetMasterVolume(0x0A0A);
        _device.SetPCMOutVolume(0x0A0A);

        _device.SetPCMOutControl(0x1D);

        return ERR_NONE;
    }

    virtual stat_t Recover() { return ERR_NONE; }

    virtual stat_t Exit()
    {
        _device.Finalize();
        return ERR_NONE;
    }

    virtual const char* const Name() { return "ac97"; }

    virtual stat_t HandleConnect(L4_ThreadId_t& tid, L4_Msg_t& msg)
    { return ERR_NONE; }

    virtual stat_t HandleDisconnect(L4_ThreadId_t& tid, L4_Msg_t& msg)
    { return ERR_NONE; }

    virtual stat_t HandlePut(L4_ThreadId_t& tid, L4_Msg_t& msg)
    { return ERR_NONE; }

    SetBufferBase();

    Start();

    Stop();
};

SERVER_HANDLER_BEGIN(AC97Server)
SERVER_HANDLER_END

ARC_SH_SERVER(AC97Server)

