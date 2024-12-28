#include <editor/Command.h>
#include <editor/Editor.h>
#include <Host.h>
#include <Maths.h>

namespace Centi::Editor
{
    bool CommandEngine::AddBinding(InputMode mode, BindingHandler* handler)
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

    bool CommandEngine::AddCommand(CommandHandler* handler)
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

    void CommandEngine::Process()
    {
        if (inputLength >= MaxBindingTagLength)
            inputLength = 0;

        const auto maybeInput = HostGetChar();
        if (!maybeInput.HasValue())
            return;

        editor->RedrawMessageBar();
        if (*maybeInput == '\e')
        {
            Mode(InputMode::Normal);
            inputLength = 0;
            return;
        }

        //TODO: we should normalize the input chars
        inputBuffer[inputLength++] = *maybeInput;

        size_t leadingDigits = 0;
        while (leadingDigits < inputLength && isdigit(inputBuffer[leadingDigits]))
            leadingDigits++;
        const size_t realInputLength = inputLength - leadingDigits;

        auto& scanList = bindings[static_cast<size_t>(mode)];
        for (auto it = scanList.Begin(); it != scanList.End(); ++it)
        {
            if (realInputLength != it->tagLength)
                continue;
            if (memcmp(inputBuffer + leadingDigits, it->tag, realInputLength) != 0)
                continue;

            const size_t repeatCount = sl::Max(1ull, strtoull(inputBuffer, nullptr, 0));
            for (size_t i = 0; i < repeatCount; i++)
                it->callback(*editor);
            inputLength = 0;
            return;
        }

        if (mode == InputMode::Command)
        {
            if (cmdInputLength >= MaxCommandLength)
                cmdInputLength = 0;

            switch (*maybeInput)
            {
            case '\r':
                Mode(InputMode::Normal);
                DoCommand({ &cmdInputBuffer[1], cmdInputLength });
                cmdInputLength = 0;
                break;
            case 0x7F:
                if (cmdInputLength > 0)
                    cmdInputLength--;
                break;
            default:
                cmdInputBuffer[1 + cmdInputLength] = *maybeInput;
                cmdInputLength++;
                break;
            }
            inputLength = 0;
        }
    }

    void CommandEngine::DoCommand(sl::StringSpan command)
    {
        if (command.Empty())
            return;
        if (command[command.Size() - 1] == 0)
            command = command.Subspan(0, command.Size() - 1);

        for (auto it = commands.Begin(); it != commands.End(); ++it)
        {
            if (command != it->tag)
                continue;

            editor->LogMessage(LogLevel::Trace, "Running command %.*s", (int)command.Size(), 
                command.Begin());
            const size_t retCode = it->callback(*editor, command, it->opaque);
            editor->LogMessage(LogLevel::Trace, "Command %.*s exited with code %zu", 
                (int)command.Size(), command.Begin(), retCode);

            return;
        }

        editor->LogMessage(LogLevel::Error, "Not a command: %.*s", (int)command.Size(), command.Begin());
    }

    size_t CommandEngine::PeekInputBuffer(sl::Span<char> buffer) const
    {
        const size_t length = sl::Min(buffer.Size(), inputLength);
        for (size_t i = 0; i < length; i++)
            buffer[i] = inputBuffer[i];

        return length;
    }

    sl::StringSpan CommandEngine::PeekCommandBuffer() const
    {
        return sl::StringSpan(cmdInputBuffer, cmdInputLength + 1);
    }

    InputMode CommandEngine::Mode(sl::Opt<InputMode> set)
    {
        const auto ret = mode;
        if (set.HasValue())
        {
            mode = *set;

            if (mode == InputMode::Command)
            {
                cmdInputBuffer[0] = ':';
                cmdInputLength = 0;
            }
            editor->RedrawMessageBar();
        }

        return ret;
    }
}
