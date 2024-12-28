#include <editor/Editor.h>
#include <NanoPrintf.h>
#include <Host.h>

namespace Centi::Editor
{
    bool Editor::UpdateDisplaySpec()
    {
        constexpr size_t MaxReplyLength = 32;

        if (!HostGetTerminalSize(displaySize.x, displaySize.y))
            return false;

        LogMessage(LogLevel::Info, "Display spec updated: w=%lu, h=%lu", 
            displaySize.x, displaySize.y);
        return true;
    }

    void Editor::EnsureRootWindowExists()
    {
    }

    void Editor::RunWorkItems(size_t limit)
    {
        for (size_t i = 0; i < limit; i++)
        {
            auto item = workItems.PopFront();
            if (item == nullptr)
                return;

            item->callback(item, this);
        }
    }

    void Editor::RenderMessageBar()
    {
        constexpr const char FormatStr[] = "\e[%lu;1H\e[2K%s%s\e[%luG%.*s";

        const char* modeStr = [](InputMode m)
        {
            switch (m)
            {
            case InputMode::Normal: return "NORMAL";
            case InputMode::Insert: return "INSERT";
            case InputMode::Command: return "COMMAND";
            case InputMode::Visual: return "VISUAL";
            default: return "WTF?!";
            }
        }(Mode());

        const char* msgStr = "";

        char inputBuffer[MaxBindingTagLength];
        const size_t inputLen = cmdProcessor.PeekInputBuffer(inputBuffer);

        char buffer[MaxRowRenderLength];
        const size_t writtenLen = npf_snprintf(buffer, MaxRowRenderLength, 
            FormatStr, displaySize.y, modeStr, msgStr, displaySize.x - 
            MaxBindingTagLength * 2, (int)inputLen, inputBuffer);

        HostPutChars({ buffer, writtenLen });
    }

    Editor::Editor()
    {
        shouldQuit = false;

        cmdProcessor.BindEditor(*this);
        cmdProcessor.AddBuiltins();

        UpdateDisplaySpec();
        EnsureRootWindowExists();
    }

    Editor::~Editor()
    {
    }

    void Editor::Run()
    {
        HostPutChars("\e[2J");
        while (!shouldQuit)
        {
            cmdProcessor.Process();
            RunWorkItems(-1);
            RenderMessageBar();
        }
    }

    void Editor::Quit()
    {
        shouldQuit = true;
    }

    void Editor::LogMessage(LogLevel level, const char* format, ...)
    {
        (void)level;
        (void)format; //TODO: log formatter + history
    }
}
