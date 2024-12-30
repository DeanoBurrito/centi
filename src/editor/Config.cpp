#include <editor/Config.h>

namespace Centi::Editor
{
    void SetDefaultConfig(Config& cfg)
    {
        //TODO: detect terminal capabilities and set based on that
        cfg.theme.cursor.fg = "30";
        cfg.theme.cursor.bg = "47";
        cfg.theme.miscText.fg = "37";
        cfg.theme.miscText.bg = "40";
    }

    void SetCommandLineConfig(Config& cfg, sl::StringSpan cmdline)
    {
    }
}
