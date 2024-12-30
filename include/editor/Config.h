#pragma once

#include <Span.h>

namespace Centi::Editor
{
    struct ColourPair //colour, with a 'u'
    {
        sl::StringSpan fg;
        sl::StringSpan bg;
    };

    struct Config
    {
        struct
        {
            ColourPair cursor;
            ColourPair miscText;
        } theme;
    };

    void SetDefaultConfig(Config& cfg);
    void SetCommandLineConfig(Config& cfg, sl::StringSpan cmdline);
}
