#ifndef ARC_FILE_EXT2_INODE_TABLE_H
#define ARC_FILE_EXT2_INODE_TABLE_H

class Ext2Partition;
class Ext2Inode;

class Ext2InodeTable
{
private:
    Ext2Partition*      _partition;
    UInt                _start;
    UInt                _nodes_per_block;
    char*               _rbuf;
    char*               _wbuf;

public:

    Ext2InodeTable(Ext2Partition* p, UInt start, UInt npb);

    ~Ext2InodeTable();

    void Read(UInt index, Ext2Inode* inode);

    void Write(UInt index, const Ext2Inode* inode);

    ///
    /// Writes back the table to the table on disk
    ///
    void Sync();

    void Dump();
};

#endif // ARC_FILE_EXT2_INODE_TABLE_H
