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

        // Register the last valid index of the buffer descriptors
        DOUT("Global status: %.8lX\n", _device.GetGlobalStatus());
        _device.SetPCMOutLastValidIndex(0x0F);

        // Register the buffer descriptor base address
        UInt* desc = (UInt*)palloc(1);
        Pager.Map((addr_t)desc, L4_ReadWriteOnly);
        _device.SetPCMOutBufDescBase(Pager.Phys((addr_t)desc));

        DOUT("current: %.2lX\n", _device.GetPCMOutCurrentIndex());
        DOUT("prefetch: %.2lX\n", _device.GetPCMOutPrefetchedIndex());
        DOUT("position: %.4lX\n", _device.GetPCMOutPosition());

        char* test_buf = (char*)palloc(1);
        Pager.Map((addr_t)test_buf, L4_ReadWriteOnly);

        bool ceil = false;
        int sample = 0;
        int sample_step = 4;
        for (int i = 0; i < 4096; i++) {
            if (!ceil) {
                sample += sample_step;
                if (sample > 0xFF) {
                    sample = 0xFF;
                    ceil = 1;
                }
                else {
                    sample -= sample_step;
                    if (sample < 0) {
                        sample = 0; 
                        ceil = 0;
                    }
                }
                test_buf[i] = sample;
            }
        }


        addr_t ptr = Pager.Phys((addr_t)test_buf);
        for (int i = 0; i < 0x10; i += 2) {
            desc[i] = ptr;
            desc[i + 1] = 0x00000100;
            ptr += 0x100;
            DOUT("desc[%d]:%.8lX desc[%d]:%.8lX\n",
                 i, desc[i], i + 1, desc[i + 1]);
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

