
#include <Debug.h>
#include <System.h>
#include <MemoryAllocator.h>
#include <AC97.h>


class MyListener : public AudioChannelListener
{
private:
    AC97Channel*    _channel;
    AC97Buffer      _buffer;
    char*           _data_buf;

public:
    MyListener(AC97Channel* c) : _channel(c)
    {
        _data_buf = (char*)malloc(0x1000);
        _buffer.desc[0].address = (addr_t)_data_buf;
        _buffer.desc[0].length = 0x1000;
    }

    AC97Buffer* GetBuffer() { return &_buffer; }

    void Handle()
    {
        DOUT("stat: %lu\n", _channel->GetStatus());

        bool ceil = false;
        int sample = 0;
        int sample_step = 4;
        int start = 0;
        int end = 4096;
        for (int i = start; i < end; i++) {
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
                _data_buf[i] = sample;
            }
        }

        _channel->SetStatus(0);
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
    channel.SetBuffer(listener.GetBuffer());
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


