
///
/// @brief  Ext2 partition representation
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  March 2008
///

//$Id: Ext2Partition.cc 385 2008-08-28 18:38:11Z hro $

#include <arc/server.h>
#include <Disk.h>
#include <Assert.h>
#include "Ext2SuperBlock.h"
#include "Ext2DataBlock.h"
#include "Ext2InodeBitmap.h"
#include "Ext2InodeTable.h"
#include "Inode.h"
#include "Ext2Partition.h"

Ext2SuperBlock*
Ext2Partition::Probe(Partition* partition)
{
    Ext2SuperBlock* sb;
    Ext2SuperBlock* ptr;
    char*           buffer;
    ENTER;

    // Figure out the size of a block

    buffer = (char*)malloc(PAGE_SIZE);
    if (buffer == 0) {
        return 0;
    }

    partition->SetBlockSize(sizeof(Ext2SuperBlock) * 2);
    partition->ReadBlock(buffer, 0);
    ptr = (Ext2SuperBlock *)(buffer + sizeof(Ext2SuperBlock));
    if (ptr->magic != Ext2SuperBlock::MAGIC) {
        partition->SetBlockSize(0);
        mfree(buffer);
        return 0;
    }

    partition->SetBlockSize(ptr->BlockSize());

    // Cache the superblock

    sb = (Ext2SuperBlock*)malloc(sizeof(Ext2SuperBlock));

    memcpy(sb, ptr, sizeof(Ext2SuperBlock));

    mfree(buffer);

    EXIT;
    return sb;
}

#ifdef SYS_DEBUG
void
Ext2Partition::DumpGroups()
{
    System.Print("--- Dump Block Groups ---\n");
    System.Print("     block map  inode map  inode table  free blocks  free inodes  dirs\n");
    System.Print("---- ---------  ---------  -----------  -----------  -----------  ----\n");
    for (UInt i = 0; i < _groups; i++) {
        System.Print("%4lu %9lu  %9lu  %11lu  %11u  %11u  %4u\n",
                     i, _group_desc[i].blockBitmap, _group_desc[i].inodeBitmap,
                     _group_desc[i].inodeTable, _group_desc[i].freeBlocks,
                     _group_desc[i].freeInodes, _group_desc[i].usedDirs);
    }
    System.Print("--- Dump Block Groups End ---\n");
}
#endif // SYS_DEBUG

stat_t
Ext2Partition::InitGroupDescriptors()
{
    size_t  len;
    stat_t  err = ERR_UNKNOWN;
    ENTER;

    //
    // Initialize the pointer to the group descriptors
    //
    len = sizeof(Ext2GroupDesc) * _groups;
    if (len < _partition->BlockSize()) {
        len = _partition->BlockSize();
    }

    _group_desc = (Ext2GroupDesc *)malloc(len);
    if (_group_desc == 0) {
        FATAL("Ext2: Failed to allocate group descriptors");
        err = ERR_OUT_OF_MEMORY;
    }
    else {
        err = _partition->Read(_group_desc,
                   _superblock->firstDataBlock + Ext2GroupDesc::OFFSET,
                   (len + _partition->BlockSize()) / _partition->BlockSize());
    }

#ifdef SYS_DEBUG
    DumpGroups();
#endif // SYS_DEBUG

    EXIT;
    return err;
}

stat_t
Ext2Partition::InitDataBlockBitmap()
{
    ENTER;
    _data_block_allocator = new Ext2DataBlockAllocator*[_groups];
    if (_data_block_allocator == 0) {
        FATAL("Ext2: Failed to allocate data block allocator");
        return ERR_OUT_OF_MEMORY;
    }

    for (UInt i = 0; i < _groups; i++) {
        _data_block_allocator[i] = new Ext2DataBlockAllocator(this,
                                                              _superblock,
                                                              &_group_desc[i]);
        if (_data_block_allocator[i] == 0) {
            FATAL("Ext2: Failed to allocate data block allocator");
            return ERR_OUT_OF_MEMORY;
        }
    }

    EXIT;
    return ERR_NONE;
}

stat_t
Ext2Partition::InitInodeBitmap()
{
    ENTER;
    _inode_allocator = new Ext2InodeAllocator*[_groups];
    if (_inode_allocator == 0) {
        FATAL("Ext2: Failed to allocate inode allocator");
        return ERR_OUT_OF_MEMORY;
    }

    for (UInt i = 0; i < _groups; i++) {
        _inode_allocator[i] = new Ext2InodeAllocator(this,
                                                     _superblock,
                                                     &_group_desc[i]);
        if (_inode_allocator[i] == 0) {
            FATAL("Ext2: Failed to allocate inode allocator");
            return ERR_OUT_OF_MEMORY;
        }
    }

    EXIT;
    return ERR_NONE;
}

stat_t
Ext2Partition::InitInodeTable()
{
    ENTER;
    _inode_table = new Ext2InodeTable*[_groups];
    if (_inode_table == 0) {
        FATAL("Ext2: Failed to allocate inode table");
        return ERR_OUT_OF_MEMORY;
    }

    for (UInt i = 0; i < _groups; i++) {
        _inode_table[i] = new Ext2InodeTable(this,
                                             _group_desc[i].inodeTable,
                                             _superblock->NodesPerBlock());
        if (_inode_table[i] == 0) {
            FATAL("Ext2: Failed to allocate inode table");
            return ERR_OUT_OF_MEMORY;
        }
    }

    EXIT;
    return ERR_NONE;
}

stat_t
Ext2Partition::Initialize(Partition *partition)
{
    ENTER;

    _superblock = Probe(partition);
    if (_superblock == 0) {
        return ERR_NOT_FOUND;
    }

    _partition = partition;

    //
    // Get number of block groups
    //
    _groups = _superblock->Groups();

    DOUT("Block size: %lu\n", _superblock->BlockSize());
    DOUT("Number of blocks: %lu\n", _superblock->blocks);
    DOUT("Number of block groups: %lu\n", _groups);
    DOUT("Inodes per group: %lu\n", _superblock->inodesPerGroup);

    InitGroupDescriptors();
    InitDataBlockBitmap();
    InitInodeBitmap();
    InitInodeTable();

    EXIT;
    return ERR_NONE;
}

Int
Ext2Partition::AllocateInode()
{
    Int ino = 0;
    UInt i = 0;

    do {
        assert(_inode_allocator[i] != 0);
        ino = _inode_allocator[i]->Allocate();
    }
    while (ino == 0 && i < _groups);

    return ino + _superblock->inodesPerGroup * i;
}

void
Ext2Partition::ReleaseInode(Int ino)
{
    UInt group = ino / _superblock->inodesPerGroup;
    if (group >= _groups) {
        return;
    }

    ino %= _superblock->inodesPerGroup;
    assert(_inode_allocator[group] != 0);
    _inode_allocator[group]->Release(ino);
}

void
Ext2Partition::Read(Int ino, Ext2Inode* inode)
{
    Int group = ino / _superblock->inodesPerGroup;
    Int offset = ino % _superblock->inodesPerGroup;

    DOUT("group: %ld offset: %ld\n", group, offset);
    assert(_inode_table[group] != 0);
    _inode_table[group]->Read(offset - 1, inode);
}

