#pragma once

#include <stdint.h>
#include <stddef.h>

enum class LogLevel
{
    Error,
    Warning,
    Info,
    Trace,
};

enum class EditorMode
{
    Normal,
    Insert,
    Command,
    Visual,

    Count
};

using TextChar = uint32_t; //full-fat utf32 form
