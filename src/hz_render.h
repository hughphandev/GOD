#ifndef HZ_RENDER_H
#define HZ_RENDER_H

#include "hz_memory.h"
#include "hz_math.h"

struct Vert
{
    Vec3 pos;
    Vec2 uv;
};

struct RenderGroup
{
    MemoryArena pushBuffer;
};

enum RenderCommandType
{
    RC_RenderCommandClear,
    RC_RenderCommandModel,
};

struct RenderCommandHeader
{
    RenderCommandType type;
};

struct RenderCommandClear
{
    Color color;
};


// Implementation

void* PushRenderElement_(RenderGroup* renderGroup, size_t size, RenderCommandType type)
{
    RenderCommandHeader* result = (RenderCommandHeader*)PushSize_(&renderGroup->pushBuffer, size + sizeof(RenderCommandHeader));

    if (result)
    {
        result->type = type;
    }
    else
    {
        INVALID_CODE_PATH;
    }
    return result + 1;
}

#define PUSH_RENDER_ELEMENT(renderGroup, type) (type*)PushRenderElement_(renderGroup, sizeof(type), RC_##type) 

void PushRenderClear(RenderGroup* renderGroup, Color color)
{
    RenderCommandClear* clear = PUSH_RENDER_ELEMENT(renderGroup, RenderCommandClear);
    clear->color = color;
}


#endif