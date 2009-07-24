
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
    AC97Buffer*     _buffer;
    UShort*         _data_buf;

public:
    MyListener(AC97Channel* c) : _channel(c)
    {
        _buffer = (AC97Buffer*)palloc(1);
        Pager.Map((addr_t)_buffer, L4_ReadWriteOnly);

        for (size_t i = 0; i < AC97Buffer::SIZE; i++) {
            memset(&_buffer->desc[i], 0, sizeof(AC97BufferDescriptor));
        }

        _data_buf = (UShort*)palloc(1);
        Pager.Map((addr_t)_data_buf, L4_ReadWriteOnly);
        memset(_data_buf, 0, 0x1000);
        _buffer->desc[0].address = Pager.Phys((addr_t)_data_buf);
        _buffer->desc[0].length = 0x1000;
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

    AC97Buffer* GetBuffer() { return _buffer; }

    void Handle()
    {
        UShort stat = _channel->GetStatus();
        DOUT("stat: %.8lX\n", stat);

        Bool ceil = false;
        Short sample = 0;
        Short sample_step = 4;
        int start = 0;
        int end = 2048;
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

        _buffer->desc[0].length = 0x1000;
        _buffer->desc[0].ioc = 1;
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
    AC97Buffer* buf = listener.GetBuffer();
    DOUT("user bufdesc list @ %p\n", buf);
    channel.SetBuffer(buf);
    channel.SetListener((AudioChannelListener*)&listener);
    err = channel.Start();
    if (err != ERR_NONE) {
        System.Print("Channel is busy.\n");
        return -1;
    }

    /*
    buf->desc[0].length = 0x1000;
    buf->desc[0].ioc = 1;
    */

    L4_Sleep(L4_TimePeriod(10000000));

    channel.Stop();
    channel.Disconnect();

    return 0;
}


