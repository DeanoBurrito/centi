#include <editor/Buffer.h>
#include <Host.h>
#include <Atomic.h>
#include <Maths.h>
#include <Locks.h>

namespace Centi::Editor
{
    sl::Atomic<size_t> bufferIdAlloc {};
    sl::List<Buffer, &Buffer::listHook> allBuffers;

    static void DestroyBufferRows(Buffer* buffer)
    {
        for (auto row = buffer->rows.PopFront(); row != nullptr; row = buffer->rows.PopFront())
        {
            HostGeneralFree(row->data.Begin(), row->data.Size());
            HostDelete(row);
        }
    }

    static bool ReallyDestroyBuffer(Buffer* buffer, bool force)
    {
        return true;
    }

    void CleanupBuffer(Buffer* buffer)
    {
        ReallyDestroyBuffer(buffer, false);
    }

    BufferRef CreateBuffer(bool writable)
    {
        BufferRef buffer = HostNew<Buffer>();
        buffer->writable = writable;
        buffer->dirty = false;
        buffer->id = bufferIdAlloc++;

        allBuffers.PushBack(&*buffer);

        return buffer;
    }

    bool DestroyBuffer(BufferRef buffer, bool force)
    {
        if (!buffer.Valid())
            return false;

        return ReallyDestroyBuffer(buffer.Drop(), force);
    }

    size_t ReadBuffer(BufferRef buffer, sl::Vector2u offset, sl::Span<char> into)
    {
        if (!buffer.Valid())
            return 0;

        auto row = buffer->rows.Begin();
        for (size_t i = 0; i < offset.y && row != buffer->rows.End(); i++)
            row = ++row;
        if (row == buffer->rows.End())
            return 0;

        const size_t copyLength = sl::Min(into.Size(), row->data.Size() - offset.x);
        memcpy(into.Begin(), row->data.Begin() + offset.x, copyLength);

        return copyLength;
    }

    void SetBuffer(BufferRef buffer, sl::StringSpan contents)
    {
        if (!buffer.Valid())
            return;

        DestroyBufferRows(&*buffer);

        size_t rowStart = 0;
        for (size_t i = 0; i < contents.Size(); i++)
        {
            if (contents[i] != '\n')
                continue;

            BufferRow* row = HostNew<BufferRow>();
            if (row == nullptr)
                return;

            char* rowBuff = static_cast<char*>(HostGeneralAlloc(i - rowStart));
            if (rowBuff == nullptr)
            {
                HostDelete(row);
                return;
            }

            memcpy(rowBuff, &contents[rowStart], i - rowStart);
            row->realDataLength = i - rowStart;
            row->data = sl::Span<char>(rowBuff, i - rowStart);
            buffer->rows.PushBack(row);
            rowStart = i + 1;
        }
    }
}
