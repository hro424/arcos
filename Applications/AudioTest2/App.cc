
#include <Debug.h>
#include <System.h>
#include <MemoryAllocator.h>
#include <PageAllocator.h>
#include <AC97.h>
#include <String.h>


class MyListener : public AudioChannelListener
{
private:
    AC97Channel*    _channel;
    AC97DescriptorList*     _buffer;
    UShort*         _data_buf;

public:
    MyListener(AC97Channel* c) : _channel(c)
    {
        _buffer = (AC97DescriptorList*)palloc(1);
        Pager.Map((addr_t)_buffer, L4_ReadWriteOnly);

        for (size_t i = 0; i < AC97DescriptorList::SIZE; i++) {
            memset(&_buffer->desc[i], 0, sizeof(AC97BufferDescriptor));
        }

        _data_buf = (UShort*)palloc(1);
        Pager.Map((addr_t)_data_buf, L4_ReadWriteOnly);
        memset(_data_buf, 0, PAGE_SIZE);
        _buffer->desc[0].address = Pager.Phys((addr_t)_data_buf);
        _buffer->desc[0].length = PAGE_SIZE;
        _buffer->desc[0].ioc = 1;

        DOUT("size: %d\n", sizeof(AC97BufferDescriptor));
        DOUT("virt %.8lX phys %.8lX\n",
             (addr_t)_data_buf, Pager.Phys((addr_t)_data_buf));
        DOUT("addr %.8lX\n", _buffer->desc[0].raw[0]);
        DOUT("desc %.8lX\n", _buffer->desc[0].raw[1]);
    }

    virtual ~MyListener()
    {
        pfree((addr_t)_data_buf, 1);
        pfree((addr_t)_buffer, 1);
    }

    AC97DescriptorList* GetBuffer() { return _buffer; }

    /*
    UInt factorial(UInt n)
    {
        UInt ret = n;
        while (n > 2) {
            n--;
            ret = ret * n;
        }
        return ret;
    }

    Int power(Int x, UInt n)
    {
        if (n == 0) {
            return 1;
        }

        Int ret = x;
        for (UInt i = 1; i < n; i++) {
            ret *= x;
        }
        return ret;
    }

    Int sin(Int x, UInt n)
    {
        Int ret = 0;
        for (int i = 0; i < n; i++) {
            UInt j = 2 * i + 1;
            Int p = power(x, j) / factorial(j);
            if (i % 2 == 0) {
                ret += p;
            }
            else {
                ret -= p;
            }
        }    
        return ret;
    }

    Int sin(Int x) { return sin(x, 4); }

    void FillBuffer(UShort* buf, size_t len)
    {
    }
    */

    void Handle()
    {
        UInt stat = _channel->GetStatus();
        DOUT("stat: %.8lX\n", stat);
        UByte lvi = (stat >> 8) & 0xFF;

        Bool ceil = false;
        Short sample = 0;
        Short sample_step = 4;
        int start = 0;
        int end = PAGE_SIZE / sizeof(UShort);
        for (int i = start; i < end; i++) {
            if (!ceil) {
                sample += sample_step;
                if (sample > 0xFF) {
                    sample = 0xFF;
                    ceil = 1;
                }
            }
            else {
                sample -= sample_step;
                if (sample < 0) {
                    sample = 0; 
                    ceil = 0;
                }
            }
            _data_buf[i] = sample;
        }

        lvi = (lvi == AC97DescriptorList::SIZE - 1) ? 0 : lvi + 1;
        _buffer->desc[lvi].address = Pager.Phys((addr_t)_data_buf);
        _buffer->desc[lvi].length = 0x1000;
        _buffer->desc[lvi].ioc = 1;
        stat = (stat & ~0xFF00) | (lvi << 8);
        _channel->SetStatus(stat);
    }
};


int
main(int argc, char* argv[])
{
    AC97Audio           audio;
    stat_t              err;

    err = audio.Initialize();
    if (err != ERR_NONE) {
        return err;
    }

    AC97Channel& channel = (AC97Channel&)audio.GetChannel(AC97Channel::pcm_out);

    MyListener listener(&channel);

    err = channel.Connect();
    if (err != ERR_NONE) {
        System.Print("Disconnected. Channel is busy.\n");
        return -1;
    }

    //channel.SetVolume(0x0A0A);
    AC97DescriptorList* buf = listener.GetBuffer();
    DOUT("user bufdesc list @ %p\n", buf);
    channel.SetBuffer(buf);
    channel.SetListener((AudioChannelListener*)&listener);
    err = channel.Start();
    if (err != ERR_NONE) {
        System.Print("Channel is busy.\n");
        return -1;
    }

    L4_Sleep(L4_TimePeriod(10000000));

    channel.Stop();
    channel.Disconnect();

    return 0;
}


