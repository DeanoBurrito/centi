#pragma once

#include <Types.h>
#include <sl/List.h>
#include <RefCount.h>
#include <Vectors.h>
#include <Span.h>

namespace Centi::Editor
{
    /* Structure of text-storage primitives:
     * A buffer represents a version of a file in-memory, it stores some per-file
     * config (is it readonly etc). Each buffer has a list of rows.
     * A buffer row has a small buffer for the common case where a row only contains a
     * few characters (a newline or opening curly brace), if the text's lengthe exceeds
     * this ammount the short buffer is unused and text data is stored in the list
     * of buffer segments.
     * A segment is the final storage container and contains a fixed size array,
     * the segment's parent row contains the length of text in that row which can be used
     * to determine how much of a segment's buffer contains valid text
     */
    struct Buffer;

    constexpr size_t BufferSegmentLength = 128;
    constexpr size_t BufferShortStoreLength = 4;

    struct BufferSegment
    {
        sl::ListHook rowHook;
        union
        {
            size_t count;
            TextChar text[BufferSegmentLength];
        };
    };

    struct BufferRow
    {
        sl::ListHook rowsListHook;
        sl::List<BufferSegment, &BufferSegment::rowHook> segments;
        size_t length;
        TextChar shortStore[BufferShortStoreLength];
    };

    struct Buffer
    {
        sl::ListHook listHook;
        sl::RefCount refCount;
        sl::List<BufferRow, &BufferRow::rowsListHook> rows;
        size_t id;
        bool writable;
        bool dirty;
    };

    void CleanupBuffer(Buffer* buffer);

    using BufferRef = sl::Ref<Buffer, &Buffer::refCount, CleanupBuffer>;

    BufferRef CreateBuffer(sl::StringSpan initContent, bool writable);
    bool DestroyBuffer(BufferRef buffer, bool force);

    size_t ReadBuffer(BufferRef buffer, sl::Vector2u offset, sl::Span<char> into);
}
