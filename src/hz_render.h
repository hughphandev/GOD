#ifndef HZ_RENDER_H
#define HZ_RENDER_H

#include "hz_memory.h"
#include "hz_math.h"
#include "hz_anim.h"

struct Renderer;

struct Vert
{
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
    u32 boneIds[MAX_BONES_PER_VERT];
    f32 weights[MAX_BONES_PER_VERT];
};


struct Camera
{
    Vec3 position;
    Vec3 direction;
    Vec3 worldUp;
    float fovy;
    f32 aspect;
};

struct Texture
{
    u32 width, height;
    u32* texel;
};

struct VertWeight
{
    u32 vertIndex;
    f32 weight;
};

struct LoadedMesh
{
    Mat4 transform;

    u32 vertexCount;
    Vert* vertices;

    u32 indexCount;
    u32* indices;

    u32 matIndex;
};

struct LoadedModel
{
    u32 meshCount;
    LoadedMesh* meshes;
    u32 boneCount;
    Bone* bones;
    Mat4 globalInverseTransform;
};

struct Material
{
    Color color;
    u32 textureCount;
    u32* textureId;
};

struct Transform
{
    Vec3 position;
    Quaternion rotation;
    Vec3 scale;
};

struct VoxelData
{
    Color color;
    Vec3 normal;
};

enum class VoxelDataType
{
    Color,
    BrickCoord,
};

struct VoxelNode
{
    u32 subdivision : 1;
    VoxelDataType dataType : 1;
    u32 child : 30;
    union
    {
        u32 color;
        struct
        {
            u32 _rs : 2;
            u32 x : 10, y : 10, z : 10;
        };
    };
};

struct SparseVoxelTree
{
    VoxelNode* root;
    u32 resolution;
};


struct alignas(16) CSPerInstance
{
};


struct alignas(16) CSPerFrame
{
    Vec3 camPos; f32 _rs2;
    Mat4 invView;
    f32 fovy;
    f32 aspect;
};

#define MAX_GRID_SIZE 20
struct alignas(16) CSPerScene
{
    Mat4 worldTrans;
    VoxelNode voxel[MAX_GRID_SIZE * MAX_GRID_SIZE * MAX_GRID_SIZE];
};

#define MAX_BONES 100
struct alignas(16) VSPerInstance
{
    Mat4 mvp;
    Mat4 model;
    Mat4 bones[MAX_BONES];
    bool isSkinnedMesh;
};


struct alignas(16) VSPerFrame
{

};

struct alignas(16) VSPerScene
{

};

struct alignas(16) PSPerInstance
{
    Color color;
};

struct alignas(16) PSPerFrame
{
    Vec3 lightDirection;
    Color diffuse;
    Color ambient;
};

struct alignas(16) PSPerScene
{

};

struct RenderGroup
{
    Renderer* renderer;
    MemoryArena pushBuffer;

    Vec3 lightDirection;
    Color diffuse;
};

enum RenderCommandType
{
    RC_RenderCommandClear,
    RC_RenderCommandModel,
    RC_RenderCommandVoxel,
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
    u32 modelId;
    Camera* camera;
    Material mat;
    u32 boneCount;
    Bone* bones;
    Mat4 globalInverseTransform;
    Mat4 transform;
    NodeTransform* nodeTransforms;
    u32 channelCount;
};

struct RenderCommandVoxel
{
    Mat4 transform;
    // SparseVoxelTree voxTree;
    VoxelData voxel;
    Camera* camera;
};

// Implementation

void* PushRenderElement_(RenderGroup* renderGroup, size_t size, RenderCommandType type)
{
    RenderCommandHeader* result = (RenderCommandHeader*)_PushSize(&renderGroup->pushBuffer, size + sizeof(RenderCommandHeader));

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

void PushRenderModel(RenderGroup* renderGroup, Camera* camera, u32 modelId, Material mat, Mat4 transform, u32 boneCount, Bone* bones, Mat4 globalInverseTransform, u32 tranCount, NodeTransform* trans)
{
    RenderCommandModel* command = PUSH_RENDER_ELEMENT(renderGroup, RenderCommandModel);
    command->camera = camera;
    command->modelId = modelId;
    command->mat = mat;
    command->transform = transform;
    command->boneCount = boneCount;
    command->bones = bones;
    command->globalInverseTransform = globalInverseTransform;
    command->nodeTransforms = trans;
    command->channelCount = tranCount;
}

void PushRenderVoxel(RenderGroup* renderGroup, VoxelData voxel, Mat4 trans, Camera* cam)
{
    RenderCommandVoxel* command = PUSH_RENDER_ELEMENT(renderGroup, RenderCommandVoxel);
    command->voxel = voxel;
    command->transform = trans;
    command->camera = cam;
}

typedef void UpdateMesh(Renderer* renderContext, u32 modelId, u32 meshId, LoadedMesh mesh);
typedef u32 UploadModel(Renderer* renderContext, LoadedModel initialModel, MemoryArena* arena);
typedef u32 UploadTexture(Renderer* renderContext, Texture texture, MemoryArena* arena);

#endif