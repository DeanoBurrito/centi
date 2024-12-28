#include <editor/Command.h>
#include <editor/Editor.h>
#include <Host.h>
#include <Maths.h>

namespace Centi::Editor
{
    bool CommandProcessor::AddBinding(InputMode mode, BindingHandler* handler)
    {
        //TODO: normalize tag chars + convert keys like delete (which becomes an escape seq)
        auto& list = bindings[static_cast<size_t>(mode)];

        for (auto it = list.Begin(); it != list.End(); ++it)
        {
            if (memcmp(it->tag, handler->tag, MaxBindingTagLength) == 0)
                return false;
        }

        list.PushBack(handler);
        return true;
    }

    bool CommandProcessor::AddCommand(CommandHandler* handler)
    {
        //TODO: normalize tag chars + convert keys like delete (which becomes an escape seq)
        if (handler->tag[handler->tag.Size() - 1] == 0)
            handler->tag = handler->tag.Subspan(0, handler->tag.Size() - 1);

        for (auto it = commands.Begin(); it != commands.End(); ++it)
        {
            if (it->tag == handler->tag)
                return false;
        }

        commands.PushBack(handler);
        return true;
    }

    void CommandProcessor::Process()
    {
        if (inputLength >= MaxBindingTagLength)
            inputLength = 0;

        const auto maybeInput = HostGetChar();
        if (!maybeInput.HasValue())
            return;

        if (*maybeInput == '\e')
        {
            Mode(InputMode::Normal);
            inputLength = 0;
            return;
        }

        inputBuffer[inputLength++] = *maybeInput;
        //TODO: we should normalize the input chars

        auto& scanList = bindings[static_cast<size_t>(editor->Mode())];
        for (auto it = scanList.Begin(); it != scanList.End(); ++it)
        {
            if (inputLength != it->tagLength)
                continue;
            if (memcmp(inputBuffer, it->tag, inputLength) != 0)
                continue;

            it->callback(*editor);
            inputLength = 0;
            break;
        }
    }

    void CommandProcessor::DoCommand(sl::StringSpan command)
    {
        if (command.Empty())
            return;
        if (command[command.Size() - 1] == 0)
            command = command.Subspan(0, command.Size() - 1);

        for (auto it = commands.Begin(); it != commands.End(); ++it)
        {
            if (command != it->tag)
                continue;

            editor->LogMessage(LogLevel::Trace, "Running command %.*s", (int)command.Size(), command.Begin());
            it->callback(*editor, nullptr, command, it->opaque);
            return;
        }
    }

    size_t CommandProcessor::PeekInputBuffer(sl::Span<char> buffer) const
    {
        const size_t length = sl::Min(buffer.Size(), inputLength);
        for (size_t i = 0; i < length; i++)
            buffer[i] = inputBuffer[i];

        return length;
    }

    InputMode CommandProcessor::Mode(sl::Opt<InputMode> set)
    {
        const auto ret = mode;
        if (set.HasValue())
            mode = *set;

        return ret;
    }
}
