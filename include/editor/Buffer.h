#pragma once

#include <Types.h>
#include <sl/List.h>
#include <RefCount.h>
#include <Vectors.h>
#include <Span.h>

namespace Centi::Editor
{
    struct BufferRow
    {
        sl::ListHook rowsListHook;
        size_t realDataLength;
        sl::Span<char> data;
    };

    struct Buffer
    {
        sl::ListHook listHook;
        sl::RefCount refCount;
        sl::List<BufferRow, &BufferRow::rowsListHook> rows;
        size_t id;
        size_t rowCount;
        bool writable;
        bool dirty;
    };

    void CleanupBuffer(Buffer* buffer);

    using BufferRef = sl::Ref<Buffer, &Buffer::refCount, CleanupBuffer>;

    BufferRef CreateBuffer(bool writable);
    bool DestroyBuffer(BufferRef buffer, bool force);

    size_t ReadBuffer(BufferRef buffer, sl::Vector2u offset, sl::Span<char> into);
    void SetBuffer(BufferRef buffer, sl::StringSpan contents);
}
