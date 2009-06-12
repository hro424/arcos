
#ifndef ARC_SYSTEM_FORMAT_H
#define ARC_SYSTEM_FORMAT_H

#include <arc/stdarg.h>
#include <Types.h>

class Stream;

class Formatter
{
public:
    static int Write(Stream* output, ssize_t n, const char* fmt, va_list ap);
};

#endif // ARC_SYSTEM_FORMAT_H
