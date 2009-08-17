
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


class PCMPlayer : public AudioChannelListener
{
private:
    static const size_t NUM_SLOT = 16;
    WaveStream*         _stream;
    AC97Channel*        _channel;
    AC97DescriptorList* _list;
    UByte*              _data_buf;
    size_t              _data_length;

    UByte               _buffer[PAGE_SIZE * 1000];
    UByte*              _cur;

public:
    static const size_t BUFFER_SIZE = PAGE_SIZE / 2;

    PCMPlayer(WaveStream* f, AC97Channel* c) : _stream(f), _channel(c)
    {
        // Allocate the space for the buffer descriptor list
        _list = (AC97DescriptorList*)palloc(1);
        Pager.Map((addr_t)_list, L4_ReadWriteOnly);

        for (size_t i = 0; i < AC97DescriptorList::SIZE; i++) {
            memset(&_list->desc[i], 0, sizeof(AC97BufferDescriptor));
        }

        // Allocate the buffer where audio data is stored.
        size_t num = BUFFER_SIZE * AC97DescriptorList::SIZE / PAGE_SIZE;
        _data_buf = (UByte*)palloc(num);
        for (int i = 0; i < num; i++) {
            Pager.Map(((addr_t)_data_buf) + PAGE_SIZE * i, L4_ReadWriteOnly);
        }
        memset(_data_buf, 0, BUFFER_SIZE * AC97DescriptorList::SIZE);
        DOUT("Data buffer allocated @ %p\n", _data_buf);
    }

    virtual ~PCMPlayer()
    {
        pfree((addr_t)_data_buf,
              BUFFER_SIZE * AC97DescriptorList::SIZE / PAGE_SIZE);
        pfree((addr_t)_list, 1);
    }

    void Initialize()
    {
        if (_list == 0) {
            return;
        }

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
            DOUT("desc[%2lu] %.8lX %.8lX\n",
                 i, _list->desc[i].raw[0], _list->desc[i].raw[1]);
        }

        // Initialize the LVI register
        UInt civ = _channel->GetStatus() & 0xFF;
        UInt init_lvi = ((NUM_SLOT * 2 - 1) + civ) % 32;
        _channel->SetStatus(init_lvi << 8);
        //_channel->SetStatus((NUM_SLOT * 2 - 1) << 8);
        DOUT("Initial LVI %lu CIV %lu\n", init_lvi, civ);

        _data_length = _stream->Size();

        //XXX:HACK
        _stream->Read(_buffer, _stream->Size(), 0);
        _cur = _buffer;
    }

    void Clear()
    {
        if (_list == 0) {
            return;
        }

        for (size_t i = 0; i < AC97DescriptorList::SIZE; i++) {
            _list->desc[i].raw[0] = 0;
            _list->desc[i].raw[1] = 0;
        }
        _channel->SetStatus(0);
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
        DOUT("\tstat:\tsr:0x%X lvi:%u civ:%u\n", sr, lvi, stat & 0xFF);

        //TODO: Convert 22.5/44.1KHz audio to the AC97's native
        //      frequency (48KHz)
        if (sr == 0x8) {
            if (_data_length == 0) {
                // end of playback
                return 1;
            }

            // Calculate the next LVI
            lvi = (lvi == 31) ? NUM_SLOT - 1 : lvi + NUM_SLOT;
            index = lvi - (NUM_SLOT - 1);

            /*XXX: HACK
            stat_t err = _stream->Read(_data_buf + BUFFER_SIZE * index,
                                       BUFFER_SIZE * NUM_SLOT, &rsize);
            if (err != ERR_NONE) {
                if (err == ERR_IPC_CANCELED) {
                    return 1;
                }
                DOUT("%s\n", stat2msg[err]);
                BREAK("err");
            }
            */
            memcpy(_data_buf + BUFFER_SIZE * index, _cur,
                   BUFFER_SIZE * NUM_SLOT);
            _cur += BUFFER_SIZE * NUM_SLOT;

            if (_data_length < rsize) {
                // Disable all the interrupt on completion
                for (UInt i = 0; i < AC97DescriptorList::SIZE; i++) {
                    _list->desc[i].ioc = 0;
                }

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
                // Update LVI
                stat = (stat & ~0xFF00) | (lvi << 8);
            }
        }

        //UInt _stat = _channel->GetStatus();
        //UByte _lvi = (_stat >> 8) & 0xFF;
        //UShort _sr = _stat >> 16;
        //DOUT("\tstat:\tsr:0x%X lvi:%u civ:%u\n", _sr, _lvi, _stat & 0xFF);

        //DOUT("\tupdate:\tsr:0x%X lvi:%u\n", sr, lvi);
        _channel->SetStatus(stat);

        return 0;
    }
};


int
main(int argc, char* argv[])
{
    static const char*  DEFAULT_SERVER = "ram";
    static const char*  DEFAULT_FILE = "vitamin_b.wav";
    L4_ThreadId_t       tid;
    WaveStream          stream;
    AC97Audio           audio;
    PCMPlayer*          player;
    stat_t              err;
    const char*         server_name = DEFAULT_SERVER;
    const char*         file_name = DEFAULT_FILE;

    if (argc > 1) {
        char* ptr = argv[1];
        while (*ptr != ':' && *ptr != '\0') {
            ptr++;
        }
        if (*ptr == ':') {
            *ptr = '\0';
            server_name = argv[1];
            file_name = ptr + 1;
        }
        else if (*ptr == '\0') {
            server_name = DEFAULT_SERVER;
            file_name = argv[1];
        }
    }

    DOUT("using '%s' '%s'\n", server_name, file_name);

    err = NameService::Get(server_name, &tid);
    if (err != ERR_NONE) {
        return -1;
    }
    DOUT("fs: %.8lX\n", tid.raw);

    err = stream.Open(tid, file_name);
    if (err != ERR_NONE) {
        DOUT("file not found\n");
        return -1;
    }

    err = audio.Initialize();
    if (err != ERR_NONE) {
        return -1;
    }

    AC97Channel& channel = (AC97Channel&)audio.GetChannel(AC97Channel::pcm_out);

    err = channel.Connect();
    if (err != ERR_NONE) {
        System.Print("Disconnected. Channel is busy.\n");
        return -1;
    }

    player = new PCMPlayer(&stream, &channel);
    player->Initialize();

    channel.SetBuffer(player->GetBuffer());
    channel.SetListener((AudioChannelListener*)player);

    System.Print("Playing '%s' ...\n", file_name);

    err = channel.Start();
    if (err != ERR_NONE) {
        System.Print("Channel is busy.\n");
        delete player;
        return -1;
    }

    // Wait for the playback end
    channel.Join();

    channel.Stop();
    System.Print("done.\n");

    channel.Disconnect();

    delete player;
    stream.Close();

    return 0;
}

