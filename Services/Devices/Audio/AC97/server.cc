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

        _device.ResetPCMOut();

        _device.SetMasterVolume(0x0A0A);
        _device.SetPCMOutVolume(0x0A0A);

        DOUT("Global status: %.8lX\n", _device.GetGlobalStatus());
        _device.SetPCMOutLastValidIndex(0x0F);

        UInt* buf = (UInt*)palloc(1);
        Pager.Map((addr_t)buf, L4_ReadWriteOnly);
        _device.SetPCMOutBuffer(Pager.Phys((addr_t)buf));

        DOUT("current: %.2lX\n", _device.GetPCMOutCurrentIndex());
        DOUT("prefetch: %.2lX\n", _device.GetPCMOutPrefetchedIndex());
        DOUT("position: %.4lX\n", _device.GetPCMOutPosition());

        for (int i = 0; i < 0x10; i += 2) {
            buf[i] = 0x5000000;
            buf[i + 1] = 0x80000100;
        }

        _device.SetPCMOutControl(0x1D);
        DOUT("PCM out status: %.4lX\n", _device.GetPCMOutStatus());
        DOUT("current: %.2lX\n", _device.GetPCMOutCurrentIndex());
        DOUT("prefetch: %.2lX\n", _device.GetPCMOutPrefetchedIndex());
        DOUT("position: %.4lX\n", _device.GetPCMOutPosition());

        DOUT("current: %.2lX\n", _device.GetPCMOutCurrentIndex());
        DOUT("prefetch: %.2lX\n", _device.GetPCMOutPrefetchedIndex());
        DOUT("position: %.4lX\n", _device.GetPCMOutPosition());
        DOUT("PCM out status: %.4lX\n", _device.GetPCMOutStatus());

        return ERR_NONE;
    }

    virtual stat_t Recover() { return ERR_NONE; }

    virtual stat_t Exit() { return ERR_NONE; }

    virtual const char* const Name() { return "ac97"; }

    virtual stat_t HandleConnect(L4_ThreadId_t& tid, L4_Msg_t& msg)
    { return ERR_NONE; }

    virtual stat_t HandleDisconnect(L4_ThreadId_t& tid, L4_Msg_t& msg)
    { return ERR_NONE; }

    virtual stat_t HandlePut(L4_ThreadId_t& tid, L4_Msg_t& msg)
    { return ERR_NONE; }
};

SERVER_HANDLER_BEGIN(AC97Server)
SERVER_HANDLER_END

ARC_SH_SERVER(AC97Server)

