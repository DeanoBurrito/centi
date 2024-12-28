#pragma once

#include <Types.h>
#include <editor/Command.h>
#include <editor/Buffer.h>
#include <Vectors.h>

namespace Centi::Editor
{
    constexpr size_t MaxWorkItemArgs = 6;
    constexpr size_t MaxRowRenderLength = 256;

    struct EditorWorkItem;
    class Editor;

    using EditorWorkCallback = void (*)(EditorWorkItem* item, Editor* editor);

    struct EditorWorkItem
    {
        sl::FwdListHook queueHook;

        void* args[MaxWorkItemArgs];
        EditorWorkCallback callback;
    };

    struct EditorMessage
    {
        sl::FwdListHook queueHook;

        LogLevel level;
        sl::StringSpan text;
    };

    struct EditorWindow
    {
        sl::FwdListHook dirtyList;

        sl::Vector2u size;
        sl::Vector2u position;
        sl::Vector2u scroll;
        sl::Vector2u cursor;
        BufferRef buffer;

        struct
        {
            bool queued;
            bool status;
            bool cursor;
            bool scroll;
        } dirty;
    };

    class Editor
    {
    private:
        bool shouldQuit;
        sl::Vector2u displaySize;
        CommandProcessor cmdProcessor;
        EditorWindow* rootWindow;
        EditorWindow* focusedWindow;

        sl::FwdList<EditorWorkItem, &EditorWorkItem::queueHook> workItems;
        sl::FwdList<EditorMessage, &EditorMessage::queueHook> logs;
        sl::FwdList<EditorWindow, &EditorWindow::dirtyList> dirtyWindows;
        
        bool UpdateDisplaySpec();
        void RunWorkItems(size_t limit);
        void RenderMessageBar();
        void RenderWindow(EditorWindow* window);
        void Render();

    public:
        Editor();
        ~Editor();

        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;
        Editor(Editor&&) = delete;
        Editor& operator=(Editor&&) = delete;

        void Run();
        void Quit();
        [[gnu::format(printf, 3, 4)]]
        bool LogMessage(LogLevel level, const char* format, ...);

        void RedrawWindow(EditorWindow* window);
        void MoveCursor(MoveDirection dir, size_t count);

        inline void RunCommand(sl::StringSpan command)
        { cmdProcessor.DoCommand(command); }

        inline InputMode Mode(sl::Opt<InputMode> set = {})
        { return cmdProcessor.Mode(set); }
    };
}
