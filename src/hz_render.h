#ifndef HZ_RENDER_H
#define HZ_RENDER_H

#include "hz_memory.h"
#include "hz_math.h"
#include <d3d11.h>

struct Vert
{
    Vec3 possition;
    Vec3 normal;
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

struct Texture
{
    u32 width, height;
    u32* texel;
};

struct LoadedModel
{
    Mat4 transform;

    Vert* vertices;
    u32 vertexCount;
    u32* indices;
    u32 indexCount;

    Texture texture;
};

struct ModelInfo
{
    s32 id;
    Mat4 transform;
};

struct Transform
{
    Vec3 position;
    Quaternion rotation;
    Vec3 scale;
};

struct alignas(16) ConstantBuffer
{
    Mat4 mvp;
    Color color;

    Color diffuse;
    Color ambient;
    Vec3 lightDirection;
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

struct RenderCommandModel
{
    Camera* camera;
    ModelInfo model;
    // Vec3 position;
    // Quaternion rotation;
    // Vec3 scale;
    Color color;
    Mat4 transform;
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

void PushRenderModel(RenderGroup* renderGroup, Camera* camera, ModelInfo model, Color color, Mat4 transform)
{
    RenderCommandModel* command = PUSH_RENDER_ELEMENT(renderGroup, RenderCommandModel);
    command->camera = camera;
    command->model = model;
    command->color = color;
    command->transform = transform;
}

#endif