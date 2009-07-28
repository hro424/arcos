
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


class MyListener : public AudioChannelListener
{
private:
    static const size_t NUM_SLOT = 4;
    WaveStream*         _stream;
    AC97Channel*        _channel;
    AC97DescriptorList* _list;
    UByte*              _data_buf;

public:
    static const size_t BUFFER_SIZE = 1024;

    MyListener(WaveStream* f, AC97Channel* c) : _stream(f), _channel(c)
    {
        addr_t  data_buf_phys;

        // Allocate the space for the buffer descriptor list
        _list = (AC97DescriptorList*)palloc(1);
        Pager.Map((addr_t)_list, L4_ReadWriteOnly);

        for (size_t i = 0; i < AC97DescriptorList::SIZE; i++) {
            memset(&_list->desc[i], 0, sizeof(AC97BufferDescriptor));
        }

        // Allocate the buffer where audio data is stored.
        _data_buf = (UByte*)palloc(8);
        for (int i = 0; i < 8; i++) {
            Pager.Map(((addr_t)_data_buf) + PAGE_SIZE * i, L4_ReadWriteOnly);
            DOUT("map %.8lX -> %.8lX\n",
                 Pager.Phys(((addr_t)_data_buf) + PAGE_SIZE * i),
                 ((addr_t)_data_buf) + PAGE_SIZE * i);
        }

        memset(_data_buf, 0, BUFFER_SIZE * AC97DescriptorList::SIZE);

        // Initialize the buffer descriptors
        for (size_t i = 0; i < AC97DescriptorList::SIZE; i++) {
            _list->desc[i].address =
                    Pager.Phys(((addr_t)_data_buf) + BUFFER_SIZE * i);
            _list->desc[i].length = BUFFER_SIZE;

            if (i % NUM_SLOT == NUM_SLOT - 1) {
                _list->desc[i].ioc = 1;
            }

            DOUT("desc[%u] %.8lX %.8lX\n",
                 i, _list->desc[i].raw[0], _list->desc[i].raw[1]);
        }

        _channel->SetStatus(7 << 8);
    }

    virtual ~MyListener()
    {
        pfree((addr_t)_data_buf, 8);
        pfree((addr_t)_list, 1);
    }

    AC97DescriptorList* GetBuffer() { return _list; }

    void Handle()
    {
        size_t  rsize;
        UInt    stat;
        UInt    index;
        UByte   lvi;
        UByte   sr;

        stat = _channel->GetStatus();
        lvi = (stat >> 8) & 0xFF;
        sr = stat >> 16;

        DOUT("\tstat:\t%.4lX %u %u\n", sr, lvi, stat & 0xFF);

        if (sr == 0x8) {
            // Calculate the next LVI
            lvi = (lvi == 31) ? NUM_SLOT - 1 : lvi + NUM_SLOT;
            index = lvi - (NUM_SLOT - 1);

            stat_t err = _stream->Read(_data_buf + BUFFER_SIZE * index,
                                       BUFFER_SIZE * NUM_SLOT, &rsize);
            if (err != ERR_NONE) {
                DOUT("%s\n", stat2msg[err]);
                BREAK("err");
            }

            DOUT("next lvi %u index %u read %u\n", lvi, index, rsize);
            for (UInt i = index; i < index + NUM_SLOT; i++) {
                if (rsize / BUFFER_SIZE > 0) {
                    _list->desc[i].length = BUFFER_SIZE;
                    rsize -= BUFFER_SIZE;
                }
                else {
                    _list->desc[i].length = rsize % BUFFER_SIZE;
                    rsize = 0;
                }
            }

            stat = (stat & ~0xFF00) | (lvi << 8);
            DOUT("\tupdate:\t%.4lX %u %u\n", sr, lvi, stat & 0xFF);
        }
        _channel->SetStatus(stat);
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

    L4_Sleep(L4_TimePeriod(20000000));

    channel.Stop();
    channel.Disconnect();

    stream.Close();

    return 0;
}

