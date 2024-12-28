#include <Host.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

namespace Centi
{
    void* HostMmapAnon(size_t length)
    {
        void* ret = mmap(nullptr, length, PROT_READ | PROT_WRITE, 
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (MAP_FAILED == ret)
            return nullptr;
        return ret;
    }

    void* HostGeneralAlloc(size_t length)
    {
        return malloc(length);
    }

    void HostGeneralFree(void* ptr, size_t length)
    {
        (void)length;
        free(ptr);
    }

    struct termios defaultInControl;

    bool HostSetupTerminal()
    {
        struct termios ioControl;

        if (tcgetattr(STDIN_FILENO, &ioControl) == -1)
            return false;
        defaultInControl = ioControl;

        ioControl.c_cc[VMIN] = 0;
        ioControl.c_cc[VTIME] = 1;

        ioControl.c_cflag |= (CS8);

        ioControl.c_lflag &= ~(ECHO);
        ioControl.c_lflag &= ~(ICANON);
        ioControl.c_lflag &= ~(ISIG);
        ioControl.c_lflag &= ~(IEXTEN);

        ioControl.c_iflag &= ~(IXON);
        ioControl.c_iflag &= ~(ICRNL);
        ioControl.c_iflag &= ~(BRKINT);
        ioControl.c_iflag &= ~(INPCK);
        ioControl.c_iflag &= ~(ISTRIP);

        ioControl.c_oflag &= ~(OPOST);
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &ioControl) == -1)
            return false;

        //TODO: use terminfo to get these values correctly
        HostPutChars("\e[?1049h");
        return true;
    }

    bool HostResetTerminal()
    {
        HostPutChars("\e[?1049l");

        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &defaultInControl) == -1)
            return false;

        return true;
    }

    bool HostGetTerminalSize(unsigned long& width, unsigned long& height)
    {
        struct winsize size;
        if (ioctl(STDIN_FILENO, TIOCGWINSZ, &size) == -1)
            return false;

        height = size.ws_row;
        width = size.ws_col;
        return true;
    }

    sl::Opt<char> HostGetChar()
    {
        char c;
        const ssize_t ret = read(STDIN_FILENO, &c, 1);
        if (ret == -1)
            return {}; //TODO: report fatal error

        if (ret == 0)
            return {};
        return c;
    }

    sl::Opt<size_t> HostPutChars(sl::StringSpan chars)
    {
        const ssize_t ret = write(STDOUT_FILENO, chars.Begin(), chars.Size());
        if (ret == -1)
            return {};
        return static_cast<size_t>(ret);
    }
}
