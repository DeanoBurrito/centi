#pragma once

#include <Types.h>
#include <Optional.h>
#include <Span.h>

extern "C"
{
    void* memchr(const void* ptr, int ch, size_t count);
    int memcmp(const void* lhs, const void* rhs, size_t count);
    void* memset(void* dest, int ch, size_t count);
    void* memcpy(void* det, const void* src, size_t count);
    void* memmove(void* dest, const void* src, size_t count);

    unsigned long long strtoull(const char* str, char** str_end, int base);
}

namespace Centi
{
    void* HostMmapAnon(size_t length);
    void* HostGeneralAlloc(size_t length);
    void HostGeneralFree(void* ptr, size_t length);

    bool HostSetupTerminal();
    bool HostResetTerminal();
    bool HostGetTerminalSize(unsigned long& width, unsigned long& height);
    sl::Opt<char> HostGetChar();
    sl::Opt<size_t> HostPutChars(sl::StringSpan chars);
}
