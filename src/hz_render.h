#ifndef HZ_RENDER_H
#define HZ_RENDER_H

#include "hz_memory.h"
#include "hz_math.h"

struct Vert
{
    Vec3 pos;
    Vec2 uv;
};


struct Camera
{
    Vec3 position;
    Vec3 direction;
    Vec3 worldUp;
    float fovy;
    float aspect;
};

struct Model
{
    Mat4 transform;

    Vert* vertices;
    u32 vertexCount;
    u32* indices;
    u32 indexCount;
};

struct Transform
{
    Vec3 position;
    Quaternion rotation;
    Vec3 scale;
};

struct RenderGroup
{
    MemoryArena pushBuffer;
    void* defaultVertexShader;
    u32 defaultVertexShaderSize;
    void* defaultPixelShader;
    u32 defaultPixelShaderSize;
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

struct RenderCommandModel
{
    Camera* camera;
    Model* model;
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
    RenderCommandClear* command = PUSH_RENDER_ELEMENT(renderGroup, RenderCommandClear);
    command->color = color;
}

void PushRenderModel(RenderGroup* renderGroup, Camera* camera, Model* model, Color color)
{
    RenderCommandModel* command = PUSH_RENDER_ELEMENT(renderGroup, RenderCommandModel);
    command->camera = camera;
    command->model = model;
    command->color = color;
}


#endif