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

enum class MoveDirection
{
    Up,
    Down,
    Left,
    Right,
};

using TextChar = uint32_t; //full-fat utf32 form
