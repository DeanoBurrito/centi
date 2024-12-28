#pragma once

#include <Types.h>
#include <List.h>
#include <Span.h>
#include <Optional.h>

namespace Centi::Editor
{
    constexpr size_t MaxBindingTagLength = 7;
    constexpr size_t MaxCommandLength = 128;

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
        sl::ListHook listHook {};

        BindingCallback callback;
        char tag[MaxBindingTagLength];
        unsigned char tagLength;
    };

    using CommandResponseCallback = size_t (*)(Editor& editor, sl::StringSpan args, void* opaque);

    struct CommandHandler
    {
        sl::ListHook listHook {};

        CommandResponseCallback callback;
        void* opaque;
        sl::StringSpan tag;
    };

    class CommandEngine
    {
    private:
        Editor* editor;
        sl::List<BindingHandler, &BindingHandler::listHook> bindings[static_cast<size_t>(InputMode::Count)];
        sl::List<CommandHandler, &CommandHandler::listHook> commands;

        InputMode mode = InputMode::Normal;
        char inputBuffer[MaxBindingTagLength];
        size_t inputLength = 0;
        char cmdInputBuffer[MaxCommandLength + 1];
        size_t cmdInputLength = 0;

    public:
        inline void BindEditor(Editor& ed)
        { editor = &ed; }

        void AddBuiltins();
        bool AddBinding(InputMode mode, BindingHandler* handler);
        bool AddCommand(CommandHandler* handler);

        void Process();
        void DoCommand(sl::StringSpan command);
        size_t PeekInputBuffer(sl::Span<char> buffer) const;
        sl::StringSpan PeekCommandBuffer() const;
        InputMode Mode(sl::Opt<InputMode> set = {});
    };
}
