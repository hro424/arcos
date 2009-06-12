
#include <arc/server.h>
#include "Inode.h"
#include "Ext2InodeTable.h"
#include "Ext2Partition.h"

Ext2InodeTable::Ext2InodeTable(Ext2Partition* p, UInt start, UInt npb)
    : _partition(p), _start(start), _nodes_per_block(npb)
{
    _rbuf = new char[_partition->BlockSize()];
    if (_rbuf == 0) {
        FATAL("Ext2: Failed to allocate i-table read buf");
    }

    _wbuf = new char[_partition->BlockSize()];
    if (_wbuf == 0) {
        FATAL("Ext2: Failed to allocate i-table write buf");
    }
}

Ext2InodeTable::~Ext2InodeTable()
{
    delete[] _rbuf;
    delete[] _wbuf;
}

void
Ext2InodeTable::Dump()
{
    _partition->ReadBlock(_rbuf, _start);
    for (size_t i = 0; i < _partition->BlockSize(); i += 16) {
        for (size_t j = i; j < i + 16; j++) {
            Debug.Print("%.2X ", _rbuf[j] & 0xFF);
        }
        Debug.Print("\n");
    }
}

void
Ext2InodeTable::Read(UInt index, Ext2Inode* inode)
{
    UInt        block_number;
    UInt        offset;
    Ext2Inode*  ptr;
  
    block_number = _start + index / _nodes_per_block;
    offset = index % _nodes_per_block;

    _partition->ReadBlock(_rbuf, block_number);

    ptr = reinterpret_cast<Ext2Inode*>(_rbuf) + offset;
    *inode = *ptr;
}

void
Ext2InodeTable::Write(UInt index, const Ext2Inode* inode)
{
    UInt        block_number;
    UInt        offset;
    Ext2Inode*  ptr;
   
    block_number = _start + index / _nodes_per_block;
    offset = index % _nodes_per_block;

    _partition->ReadBlock(_wbuf, block_number);

    ptr = reinterpret_cast<Ext2Inode*>(_wbuf) + offset;
    *ptr = *inode;

    _partition->WriteBlock(_wbuf, block_number);
}

void
Ext2InodeTable::Sync()
{
}


