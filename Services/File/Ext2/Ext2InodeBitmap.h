
///
/// @file   Services/File/Ext2/Ext2InodeBitmap.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  March 2008
///

//$Id: Ext2InodeBitmap.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_FILE_EXT2_INODE_BITMAP_H
#define ARC_FILE_EXT2_INODE_BITMAP_H

class Bitmap;
class Ext2GroupDesc;
class Ext2Partition;
class Ext2SuperBlock;

class Ext2InodeAllocator
{
private:
    Ext2Partition*  _partition;
    Ext2SuperBlock* _superblock;
    Ext2GroupDesc*  _group_desc;
    Bitmap*         _bitmap;

public:
    Ext2InodeAllocator(Ext2Partition* p,
                       Ext2SuperBlock* sb,
                       Ext2GroupDesc* gd);

    ~Ext2InodeAllocator();

    Int Allocate();

    void Release(Int node);

    void Sync();
};

#endif // ARC_FILE_EXT2_INODE_BITMAP_H

