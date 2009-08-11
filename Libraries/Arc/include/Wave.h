
#ifndef ARC_AUDIO_WAVE_H
#define ARC_AUDIO_WAVE_H

#include <Debug.h>
#include <System.h>
#include <FileStream.h>
#include <String.h>
#include <MemoryAllocator.h>

struct WaveChunkHeader
{
    char        chunk_id[4];
    UInt        chunk_size;
};


struct WaveFileChunk
{
    static const char* ID;
    static const char* WAVE_ID;
    char            wave_id[4];
};


struct WaveFormatChunk
{
    static const char* ID;
    UShort          format_tag;
    UShort          num_channels;
    UInt            sampling_rate;
    UInt            avg_rate;
    UShort          block_size;
    UShort          bps;
};


struct WavePadChunk
{
    static const char* ID;
};


struct WaveDataChunk
{
    static const char* ID;
};


class WaveStream : public FileStream
{
protected:
    WaveFormatChunk*    _format;
    size_t              _wave_size;

    stat_t OpenStream(L4_ThreadId_t tid, const char* path)
    {
        stat_t  err;

        err = FileStream::Connect(tid);
        if (err != ERR_NONE) {
            return err;
        }

        err = FileStream::Open(path, FileStream::READ);
        if (err != ERR_NONE) {
            FileStream::Disconnect();
            return err;
        }

        return ERR_NONE;
    }

    void CloseStream()
    {
        FileStream::Close();
        FileStream::Disconnect();
    }

    virtual stat_t ReadChunkHeader(const char* id, WaveChunkHeader* h)
    {
        size_t  rsize;

        for (;;) {
            FileStream::Read(h, sizeof(WaveChunkHeader), &rsize);
            if (strncmp(h->chunk_id, WavePadChunk::ID, 4) == 0) {
                FileStream::Seek(h->chunk_size, FileStream::SEEK_CUR);
            }
            else {
                break;
            }
        }

        DOUT("'%c%c%c%c' %u bytes\n",
             h->chunk_id[0], h->chunk_id[1], h->chunk_id[2],
             h->chunk_id[3], h->chunk_size);

        if (strncmp(h->chunk_id, id, 4) == 0) {
            return ERR_NONE;
        }
        else {
            return ERR_NOT_FOUND;
        }
    }

public:
    WaveStream() : _format(0), _wave_size(0) {}

    virtual ~WaveStream()
    {
        if (_wave_size != 0) {
            Close();
        }
    }

    virtual stat_t Open(L4_ThreadId_t tid, const char* path)
    {
        ENTER;
        stat_t          err;
        WaveChunkHeader header;
        char            buf[4];
        size_t          rsize;

        err = OpenStream(tid, path);
        if (err != ERR_NONE) {
            return err;
        }

        err = ReadChunkHeader(WaveFileChunk::ID, &header);
        if (err != ERR_NONE) {
            CloseStream();
            return err;
        }

        FileStream::Read(buf, 4, &rsize);
        if (strncmp(buf, WaveFileChunk::WAVE_ID, 4) != 0) {
            CloseStream();
            return ERR_NOT_FOUND;
        }

        err = ReadChunkHeader(WaveFormatChunk::ID, &header);
        if (err != ERR_NONE) {
            CloseStream();
            return err;
        }

        _format = (WaveFormatChunk*)malloc(header.chunk_size);
        if (_format == 0) {
            CloseStream();
            return ERR_OUT_OF_MEMORY;
        }

        FileStream::Read((void*)_format, header.chunk_size, &rsize);

        DOUT("tag:%.4lX ch:%.4lX rate:%u avg:%u blk:%u bps:%u\n",
             _format->format_tag, _format->num_channels,
             _format->sampling_rate, _format->avg_rate, _format->block_size,
             _format->bps);

        err = ReadChunkHeader(WaveDataChunk::ID, &header);
        if (err != ERR_NONE) {
            mfree(_format);
            CloseStream();
            return err;
        }

        _wave_size = header.chunk_size;

        EXIT;
        return ERR_NONE;
    }

    /*
    virtual stat_t Read(void* buf, size_t count, size_t* rsz)
    {
        ENTER;
        stat_t err = FileStream::Read(buf, count, rsz);
        EXIT;
        return err;
    }
    */

    virtual void Close()
    {
        ENTER;
        CloseStream();
        if (_format != 0) {
            mfree(_format);
            _format = 0;
        }
        _wave_size = 0;
        EXIT;
    }

    virtual UInt GetNumChannels()
    {
        if (_format != 0) {
            return _format->num_channels;
        }
        else {
            return 0;
        }
    }

    virtual Int Size() { return _wave_size; }
};

#endif // ARC_AUDIO_WAVE_H

