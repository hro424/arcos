#ifndef ARC_MICRO_SHELL_LIST_COMMAND_H
#define ARC_MICRO_SHELL_LIST_COMMAND_H

#include "Command.h"
#include <FileStream.h>
#include <Types.h>
#include <String.h>
#include <NameService.h>
#include <System.h>

class ListCommand : public Command
{
private:
    static const char*  NAME;
    FileStream          _fs;

public:
    virtual Bool Match(const char* str, size_t len)
    { return (strncmp(str, NAME, strlen(NAME)) == 0); }

    virtual stat_t Execute(int argc, char* argv[]);
};

inline stat_t
ListCommand::Execute(int argc, char* argv[])
{
    struct Dir {
        UInt    inode;
        UShort  record_len;
        UByte   name_len;
        UByte   type;
        char    name[];
    };

    static char     buffer[4096];
    static char     name_buf[256];
    size_t          rsize;
    size_t          len;
    stat_t          err;
    Dir*            ptr;
    char*           path;
    char*           fs_name;
    L4_ThreadId_t   server;

    if (argc < 2) {
        return ERR_NOT_FOUND;
    }

    fs_name = argv[1];
    path = argv[1];
    len = strlen(path) + 1;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == ':') {
            path[i] = '\0';
            path = &path[i + 1];
            break;
        }
    }

    err = NameService::Get(fs_name, &server);
    if (err != ERR_NONE) {
        return err;
    }


    if (_fs.Connect(server) != ERR_NONE) {
        System.Print("file system not found\n");
        return ERR_NONE;
    }

    err = _fs.Open(path, FileStream::READ);
    if (err != ERR_NONE) {
        return err;
    }

    _fs.Read(buffer, 4096, &rsize);
    _fs.Close();

    System.Print("INO\t\tTYPE\tNAME\n");
    for (UInt i = 0; i < rsize;) {
        ptr = reinterpret_cast<Dir*>(buffer + i);
        memcpy(name_buf, ptr->name, ptr->name_len);
        name_buf[ptr->name_len] = '\0';
        System.Print("%8lu\t%u\t%s\n", ptr->inode, ptr->type, name_buf);
        i += ptr->record_len;
    }

    _fs.Disconnect();

    return ERR_NONE;
}

#endif // ARC_MICRO_SHELL_LIST_COMMAND_H
