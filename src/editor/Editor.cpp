#include <editor/Editor.h>
#include <EscapeCodes.h>
#include <NanoPrintf.h>
#include <Host.h>
#include <Maths.h>

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
        constexpr const char FormatStr[] = "\e[%lu;1H\e[2K%s %s\e[%luG%.*s";
        constexpr const char FormatStr2[] = CSI_CURSOR_SET_POS_("1", "%lu") CSI_ERASE_LINE(2) "%s %s" CSI_CURSOR_SET_X_("%lu") "%.*s";

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
        if (!logs.Empty())
            msgStr = logs.Front().text.Begin();

        char inputBuffer[MaxBindingTagLength];
        const size_t inputLen = cmdProcessor.PeekInputBuffer(inputBuffer);

        char buffer[MaxRowRenderLength];
        const size_t writtenLen = npf_snprintf(buffer, MaxRowRenderLength, 
            FormatStr, displaySize.y, modeStr, msgStr, displaySize.x - 
            MaxBindingTagLength * 2, (int)inputLen, inputBuffer);

        HostPutChars({ buffer, writtenLen });
    }

    void Editor::RenderWindow(EditorWindow* window)
    {
        if (window == nullptr)
            return;

        char lineBuff[MaxRowRenderLength];
        size_t lineBuffLen = 0;

        HostPutChars(CSI_CURSOR_SET_POS(1, 1));
        HostPutChars("~");
        for (size_t i = 1; i < window->size.y - 1; i++)
            HostPutChars(CSI_CURSOR_DOWN_LINE(1)"~");

        if (window->dirty.cursor)
        {
            window->dirty.cursor = false;
            window->dirty.status = true;
        }

        if (window->dirty.status)
        {
            constexpr const char FormatStr[] = CSI_CURSOR_DOWN_LINE(1) CSI_ERASE_LINE(2) "cursor %lu:%lu";
            window->dirty.status = false;

            lineBuffLen = npf_snprintf(lineBuff, MaxRowRenderLength, FormatStr,
                window->cursor.y, window->cursor.x);
            HostPutChars({ lineBuff, lineBuffLen });
        }
    }

    void Editor::Render()
    {
        while (!dirtyWindows.Empty())
        {
            auto win = dirtyWindows.PopFront();
            win->dirty.queued = false;
            RenderWindow(win);
        }
        RenderMessageBar();
    }

    Editor::Editor()
    {
        shouldQuit = false;

        cmdProcessor.BindEditor(*this);
        cmdProcessor.AddBuiltins();

        UpdateDisplaySpec();
        
        rootWindow = focusedWindow = new EditorWindow();
        rootWindow->size = displaySize;
        rootWindow->size.y--; //leave space for message bar
        rootWindow->dirty.cursor = true;
        dirtyWindows.PushBack(rootWindow);
    }

    Editor::~Editor()
    {
    }

    void Editor::Run()
    {
        HostPutChars(CSI_ERASE(2) CSI_HIDE_CURSOR);

        while (!shouldQuit)
        {
            cmdProcessor.Process();
            RunWorkItems(-1);

            Render();
        }

        HostPutChars(CSI_SHOW_CURSOR);
    }

    void Editor::Quit()
    {
        shouldQuit = true;
    }

    bool Editor::LogMessage(LogLevel level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        const size_t finalLength = npf_vsnprintf(nullptr, 0, format, args);
        va_end(args);

        EditorMessage* msg = HostNew<EditorMessage>();
        if (msg == nullptr)
            return false;

        char* buffer = static_cast<char*>(HostGeneralAlloc(finalLength + 1));
        if (buffer == nullptr)
        {
            HostDelete(msg);
            return false;
        }

        npf_vsnprintf(buffer, finalLength + 1, format, args);
        msg->text = sl::StringSpan(buffer, finalLength);
        msg->level = level;

        logs.PushFront(msg);
        return true;
    }

    void Editor::RedrawWindow(EditorWindow* window)
    {
        if (window == nullptr)
            return;

        if (window->dirty.queued)
            return;
        window->dirty.queued = true;
        dirtyWindows.PushBack(window);
    }

    void Editor::MoveCursor(MoveDirection dir, size_t count)
    {
        if (focusedWindow == nullptr)
            return;

        switch (dir)
        {
        case MoveDirection::Up: 
            focusedWindow->cursor.y = sl::Clamp(focusedWindow->cursor.y - count, 0ul, focusedWindow->size.y);
            break;
        case MoveDirection::Down: 
            focusedWindow->cursor.y = sl::Clamp(focusedWindow->cursor.y + count, 0ul, focusedWindow->size.y);
            break;
        case MoveDirection::Left: 
            focusedWindow->cursor.x = sl::Clamp(focusedWindow->cursor.x - count, 0ul, focusedWindow->size.x);
            break;
        case MoveDirection::Right: 
            focusedWindow->cursor.x = sl::Clamp(focusedWindow->cursor.x + count, 0ul, focusedWindow->size.x);
            break;
        }

        focusedWindow->dirty.cursor = true;
        RedrawWindow(focusedWindow);
    }
}
