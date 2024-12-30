#include <Host.h>
#include <List.h>
#include <Locks.h>
#include <Maths.h>
#include <NanoPrintf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

namespace Centi
{
    void* HostGeneralAlloc(size_t length)
    {
        return malloc(length);
    }

    void HostGeneralFree(void* ptr, size_t length)
    {
        (void)length;
        free(ptr);
    }

    struct OpenFile
    {
        sl::ListHook hook;
        sl::StringSpan accessWindow;
        int fd;
    };

    sl::SpinLock openFilesLock;
    sl::List<OpenFile, &OpenFile::hook> openFiles;

    sl::StringSpan HostOpenFileForReading(sl::StringSpan filename)
    {
        OpenFile* store = HostNew<OpenFile>();
        if (store == nullptr)
            return {};

        //make a local (null terminated) copy of the filename, since the string span doesnt play
        //nicely with what the c stdlib expects
        constexpr size_t filenameBuffLen = sl::Min<size_t>(FILENAME_MAX, 1024);
        char filenameBuff[filenameBuffLen];
        npf_snprintf(filenameBuff, filenameBuffLen, "%.*s", (int)filename.Size(), filename.Begin());

        store->fd = open(filenameBuff, O_RDONLY);
        if (store->fd == -1)
        {
            HostDelete(store);
            return {};
        }

        struct stat fileStat;
        if (fstat(store->fd, &fileStat) == -1)
        {
            close(store->fd);
            HostDelete(store);
            return {};
        }

        void* mmapAddr = mmap(nullptr, fileStat.st_size, PROT_READ, MAP_PRIVATE, store->fd, 0);
        if (mmapAddr == MAP_FAILED)
        {
            close(store->fd);
            HostDelete(store);
            return {};
        }

        store->accessWindow = sl::StringSpan(static_cast<const char*>(mmapAddr), fileStat.st_size);
        openFilesLock.Lock();
        openFiles.PushBack(store);
        openFilesLock.Unlock();

        return store->accessWindow;
    }

    void HostCloseFileForReading(sl::StringSpan file)
    {
        OpenFile* store = nullptr;

        openFilesLock.Lock();
        for (auto it = openFiles.Begin(); it != openFiles.End(); ++it)
        {
            if (it->accessWindow == file)
            {
                store = &*it;
                openFiles.Remove(it);
                break;
            }
        }
        openFilesLock.Unlock();

        if (store == nullptr)
            return;

        munmap((void*)store->accessWindow.Begin(), store->accessWindow.Size());
        close(store->fd);
        HostDelete(store);
    }

    bool HostWriteFile(sl::StringSpan filename, sl::StringSpan content)
    {
        return false;
    }

    struct termios defaultTermios;

    bool HostSetupTerminal()
    {
        struct termios ioControl;

        if (tcgetattr(STDIN_FILENO, &ioControl) == -1)
            return false;
        defaultTermios = ioControl;

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

        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &defaultTermios) == -1)
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
            return {};

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
