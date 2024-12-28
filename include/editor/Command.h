#pragma once

#include <Types.h>
#include <List.h>
#include <Span.h>
#include <Optional.h>

namespace Centi::Editor
{
    constexpr size_t MaxBindingTagLength = 7;

    enum class InputMode
    {
        Normal,
        Insert,
        Command,
        Visual,

        Count
    };

    class Editor;
    struct EditorWindow;

    using BindingCallback = void (*)(Editor& editor);

    struct BindingHandler
    {
        sl::ListHook listHook;

        BindingCallback callback;
        char tag[MaxBindingTagLength];
        char tagLength;
    };

    using CommandResponseCallback = size_t (*)(Editor& editor, EditorWindow* window, sl::StringSpan args, void* opaque);

    struct CommandHandler
    {
        sl::ListHook listHook;

        CommandResponseCallback callback;
        void* opaque;
        sl::StringSpan tag;
    };

    class CommandProcessor
    {
    private:
        Editor* editor;
        sl::List<BindingHandler, &BindingHandler::listHook> bindings[static_cast<size_t>(InputMode::Count)];
        sl::List<CommandHandler, &CommandHandler::listHook> commands;

        InputMode mode = InputMode::Normal;
        char inputBuffer[MaxBindingTagLength];
        size_t inputLength = 0;

    public:
        inline void BindEditor(Editor& ed)
        { editor = &ed; }

        void AddBuiltins();
        bool AddBinding(InputMode mode, BindingHandler* handler);
        bool AddCommand(CommandHandler* handler);

        void Process();
        void DoCommand(sl::StringSpan command);
        size_t PeekInputBuffer(sl::Span<char> buffer) const;
        InputMode Mode(sl::Opt<InputMode> set = {});
    };
}
