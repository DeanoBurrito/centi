#include <editor/Buffer.h>
#include <Host.h>
#include <Atomic.h>
#include <Maths.h>
#include <Locks.h>

namespace Centi::Editor
{
    constexpr size_t SegmentExpansionLength = 4 * GiB;

    sl::Atomic<size_t> bufferIdAlloc {};
    sl::List<Buffer, &Buffer::listHook> allBuffers;

    sl::SpinLock freeSegsLock;
    sl::List<BufferSegment, &BufferSegment::rowHook> freeSegs {};

    static BufferSegment* AllocSegment()
    {
        sl::ScopedLock scopeLock(freeSegsLock);

        if (freeSegs.Empty())
        {
            void* newMem = HostMmapAnon(SegmentExpansionLength);
            if (newMem == nullptr)
                return nullptr;

            BufferSegment* header = new(newMem) BufferSegment();
            header->count = SegmentExpansionLength / sizeof(BufferSegment);
            freeSegs.PushFront(header);
        }

        BufferSegment* seg = freeSegs.PopFront();
        if (seg->count > 1)
        {
            BufferSegment* latest = new(seg + 1) BufferSegment();
            latest->count = seg->count - 1;
            freeSegs.PushFront(latest);
        }

        return seg;
    }

    static void FreeSegment(BufferSegment* seg)
    {
        if (seg == nullptr)
            return;

        sl::ScopedLock scopeLock(freeSegsLock);
        freeSegs.PushFront(seg);
    }

    void CleanupBuffer(Buffer* buffer)
    {
        DestroyBuffer(buffer, false);
    }

    BufferRef CreateBuffer(sl::StringSpan initContent, bool writable)
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

        //TODO: destroy segments and then finally buffer
        return true;
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

        size_t written = 0;
        if (row->length <= BufferShortStoreLength)
        {
            if (row->length > offset.x)
            {
                written = row->length - offset.x;
                memcpy(into.Begin(), row->shortStore + offset.x, written);
            }
            return written;
        }

        const size_t totalWriteLength = sl::Min(into.Size(), row->length - offset.x);
        for (auto seg = row->segments.Begin(); seg != row->segments.End(); ++seg)
        {
            const size_t copyLength = sl::Min(totalWriteLength - written, BufferSegmentLength - offset.x);

            memcpy(&into[written], seg->text + offset.x, copyLength);
            written += copyLength;
            offset.x = 0;
        }

        return written;
    }
}
