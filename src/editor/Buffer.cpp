#include <editor/Buffer.h>

namespace Centi::Editor
{
    static BufferSegment* AllocSegment()
    {}

    static void FreeSegment(BufferSegment* seg)
    {}

    BufferRef CreateBuffer()
    {
    }

    bool DestroyBuffer(Buffer* buff, bool force)
    {
    }
}
