#include <editor/Command.h>
#include <editor/Editor.h>

namespace Centi::Editor
{
    BindingHandler builtinBindsNormal[] = 
    {
        {
            .callback = [](Editor& editor) 
            { 
                editor.RunCommand("quit"); 
            },
            .tag = "q",
            .tagLength = 1
        },
        {
            .callback = [](Editor& editor) 
            { 
                editor.Mode(InputMode::Command); 
            },
            .tag = ":",
            .tagLength = 1
        },
        {
            .callback = [](Editor& editor) 
            { 
                editor.Mode(InputMode::Insert); 
            },
            .tag = "i",
            .tagLength = 1
        },
        {
            .callback = [](Editor& editor) 
            { 
                editor.Mode(InputMode::Visual); 
            },
            .tag = "v",
            .tagLength = 1,
        },
    };

    BindingHandler builtinBindsInsert[] =
    {
    };

    BindingHandler builtinBindsCommand[] =
    {
    };

    BindingHandler builtinBindsVisual[] =
    {
    };

    CommandHandler builtinCommands[] =
    {
        {
            .callback = [](Editor& editor, EditorWindow* window, sl::StringSpan args, void* opaque) -> size_t
            {
                (void)window;
                (void)args;
                (void)opaque;

                editor.Quit();
                return 0;
            },
            .opaque = nullptr,
            .tag = "quit",
        },
    };

    void CommandProcessor::AddBuiltins()
    {
        constexpr size_t normalCount = sizeof(builtinBindsNormal) / sizeof(BindingHandler);
        for (size_t i = 0; i < normalCount; i++)
            AddBinding(EditorMode::Normal, &builtinBindsNormal[i]);

        constexpr size_t insertCount = sizeof(builtinBindsInsert) / sizeof(BindingHandler);
        for (size_t i = 0; i < insertCount; i++)
            AddBinding(EditorMode::Insert, &builtinBindsInsert[i]);

        constexpr size_t commandCount = sizeof(builtinBindsCommand) / sizeof(BindingHandler);
        for (size_t i = 0; i < commandCount; i++)
            AddBinding(EditorMode::Command, &builtinBindsCommand[i]);

        constexpr size_t visualCount = sizeof(builtinBindsVisual) / sizeof(BindingHandler);
        for (size_t i = 0; i < visualCount; i++)
            AddBinding(EditorMode::Visual, &builtinBindsVisual[i]);

        editor->LogMessage(LogLevel::Info, "Builtin bindings added: %zu normal, %zu insert, %zu command, %zu visual",
            normalCount, insertCount, commandCount, visualCount);

        constexpr size_t builtinCmdCount = sizeof(builtinCommands) / sizeof(CommandHandler);
        for (size_t i = 0; i < builtinCmdCount; i++)
            AddCommand(&builtinCommands[i]);
        editor->LogMessage(LogLevel::Info, "Builtin commands added: %zu", builtinCmdCount);
    }
}
