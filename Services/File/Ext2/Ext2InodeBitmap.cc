
#include <arc/server.h>
#include <Bitmap.h>
#include "Ext2InodeBitmap.h"
#include "Ext2Partition.h"
#include "Ext2SuperBlock.h"

Ext2InodeAllocator::Ext2InodeAllocator(Ext2Partition* p,
                                       Ext2SuperBlock* sb,
                                       Ext2GroupDesc* gd)
    : _partition(p), _superblock(sb), _group_desc(gd)
{
    // Create a new bitmap
    _bitmap = new Bitmap(_partition->BlockSize() * Bitmap::BITS_PER_BYTE);
    if (_bitmap == 0) {
        FATAL("Ext2: Failed to create inode bitmap");
    }

    // Load the current state from the disk to the bitmap.
    // The length of the inode bitmap is always one block.
    _partition->ReadBlock(_bitmap->GetMap(), _group_desc->inodeBitmap);
}

Ext2InodeAllocator::~Ext2InodeAllocator()
{
    delete _bitmap;
}

Int
Ext2InodeAllocator::Allocate()
{
    Int ino = 0;
    for (size_t i = 0; i < _bitmap->Length(); i++) {
        if (!_bitmap->Test(i)) {
            _bitmap->Set(i);
            _group_desc->freeInodes--;
            _superblock->freeInodes--;
            ino = i;
            break;
        }
    }

    return ino;
}

void
Ext2InodeAllocator::Release(Int ino)
{
    _bitmap->Reset(ino);
    _group_desc->freeInodes++;
    _superblock->freeInodes++;
}

void
Ext2InodeAllocator::Sync()
{
    _partition->WriteBlock(_bitmap->GetMap(), _group_desc->inodeBitmap);
    _partition->SyncSuperBlock();
    _partition->SyncGroupDescriptors();
}


