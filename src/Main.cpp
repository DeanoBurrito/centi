#include <Host.h>
#include <editor/Editor.h>

int main(int argc, const char** argv)
{
    using namespace Centi;

    if (!HostSetupTerminal())
        return 1;

    (void)argc; (void)argv; //TODO: argument parsing
    Editor::Editor ed {};
    ed.Run();

    if (!HostResetTerminal())
        return -1;
    return 0;
}
