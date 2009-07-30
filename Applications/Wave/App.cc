
#include <Debug.h>
#include <System.h>
#include <MemoryAllocator.h>
#include <PageAllocator.h>
#include <AC97.h>
#include <String.h>
#include <FileStream.h>
#include <NameService.h>
#include <Ipc.h>
#include <Wave.h>


static inline void
ReadTSC(unsigned long* hi, unsigned long* lo)
{
    asm volatile ("rdtsc" : "=a" (*lo), "=d" (*hi));
}


class MyListener : public AudioChannelListener
{
private:
    static const size_t NUM_SLOT = 8;
    WaveStream*         _stream;
    AC97Channel*        _channel;
    AC97DescriptorList* _list;
    UByte*              _data_buf;
    size_t              _data_length;

public:
    static const size_t BUFFER_SIZE = PAGE_SIZE / 2;

    MyListener(WaveStream* f, AC97Channel* c) : _stream(f), _channel(c)
    {
        // Allocate the space for the buffer descriptor list
        _list = (AC97DescriptorList*)palloc(1);
        Pager.Map((addr_t)_list, L4_ReadWriteOnly);

        for (size_t i = 0; i < AC97DescriptorList::SIZE; i++) {
            memset(&_list->desc[i], 0, sizeof(AC97BufferDescriptor));
        }

        // Allocate the buffer where audio data is stored.
        _data_buf = (UByte*)palloc(BUFFER_SIZE * AC97DescriptorList::SIZE / PAGE_SIZE);
        for (int i = 0; i < 8; i++) {
            Pager.Map(((addr_t)_data_buf) + PAGE_SIZE * i, L4_ReadWriteOnly);
        }

        memset(_data_buf, 0, BUFFER_SIZE * AC97DescriptorList::SIZE);

        // Initialize the buffer descriptors
        for (size_t i = 0; i < AC97DescriptorList::SIZE; i++) {
            _list->desc[i].address =
                    Pager.Phys(((addr_t)_data_buf) + BUFFER_SIZE * i);
            // Number of samples to be processed
            _list->desc[i].length = BUFFER_SIZE / _stream->GetNumChannels();

            // Get interrupted by every NUM_SLOT
            if (i % NUM_SLOT == NUM_SLOT - 1) {
                _list->desc[i].ioc = 1;
            }
        }

        // Initialize the LVI register
        _channel->SetStatus((NUM_SLOT * 2 - 1) << 8);

        _data_length = _stream->Size();
    }

    virtual ~MyListener()
    {
        pfree((addr_t)_data_buf,
              BUFFER_SIZE * AC97DescriptorList::SIZE / PAGE_SIZE);
        pfree((addr_t)_list, 1);
    }

    AC97DescriptorList* GetBuffer() { return _list; }

    Int Handle()
    {
        size_t  rsize;
        UInt    stat;
        UInt    index;
        UByte   lvi;
        UShort  sr;

        stat = _channel->GetStatus();
        lvi = (stat >> 8) & 0xFF;
        sr = stat >> 16;

        //TODO: Convert 22.5/44.1KHz audio to the native freqency (48KHz)
        //DOUT("\tstat:\t0x%X %u %u\n", sr, lvi, stat & 0xFF);
        if (sr == 0x8) {
            if (_data_length == 0) {
                // end of playback
                return 0;
            }

            // Calculate the next LVI
            lvi = (lvi == 31) ? NUM_SLOT - 1 : lvi + NUM_SLOT;
            index = lvi - (NUM_SLOT - 1);

            stat_t err = _stream->Read(_data_buf + BUFFER_SIZE * index,
                                       BUFFER_SIZE * NUM_SLOT, &rsize);
            if (err != ERR_NONE) {
                if (err == ERR_IPC_CANCELED) {
                    return 0;
                }
                DOUT("%s\n", stat2msg[err]);
                BREAK("err");
            }

            if (_data_length < rsize) {
                for (UInt i = index; i < index + NUM_SLOT; i++) {
                    if (_data_length / BUFFER_SIZE > 0) {
                        _list->desc[i].length =
                                BUFFER_SIZE / _stream->GetNumChannels();
                        _data_length -= BUFFER_SIZE;
                    }
                    else {
                        _list->desc[i].length = (_data_length % BUFFER_SIZE) /
                                                    _stream->GetNumChannels();
                        _data_length = 0;
                    }
                }
            }
            else {
                _data_length -= rsize;
            }

            stat = (stat & ~0xFF00) | (lvi << 8);
            //DOUT("\tupdate:\t0x%X %u %u\n", sr, lvi, stat & 0xFF);
        }
        _channel->SetStatus(stat);

        return 1;
    }
};


#define FILE_SERVER     "ram"
#define AUDIO_FILE      "test.wav"

int
main(int argc, char* argv[])
{
    L4_ThreadId_t       tid;
    WaveStream          stream;
    AC97Audio           audio;
    AC97DescriptorList* buf;
    stat_t              err;

    err = NameService::Get(FILE_SERVER, &tid);
    if (err != ERR_NONE) {
        return err;
    }

    err = stream.Open(tid, AUDIO_FILE);
    if (err != ERR_NONE) {
        return err;
    }

    err = audio.Initialize();
    if (err != ERR_NONE) {
        return err;
    }

    AC97Channel& channel = (AC97Channel&)audio.GetChannel(AC97Channel::pcm_out);

    MyListener listener(&stream, &channel);

    err = channel.Connect();
    if (err != ERR_NONE) {
        System.Print("Disconnected. Channel is busy.\n");
        return -1;
    }

    buf = listener.GetBuffer();
    DOUT("user bufdesc list @ %p\n", buf);
    channel.SetBuffer(buf);
    channel.SetListener((AudioChannelListener*)&listener);

    System.Print("Playing '%s'...\n", AUDIO_FILE);

    err = channel.Start();
    if (err != ERR_NONE) {
        System.Print("Channel is busy.\n");
        return -1;
    }

    channel.Join();
    channel.Stop();
    channel.Disconnect();
    stream.Close();

    return 0;
}

