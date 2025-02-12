#ifndef GAME_H
#define GAME_H

#include "hz_define.h"
#include "hz_render.h"
#include "hz_asset.h"
#include "hz_physics.h"


struct ButtonState
{
    bool isDown;
    int halfTransitionCount;
};

#define MAX_BUTTON 8
struct GameInput
{
    // button
    union
    {
        ButtonState buttons[MAX_BUTTON];
        struct
        {
            ButtonState up;
            ButtonState down;
            ButtonState left;
            ButtonState right;
            ButtonState escape;
            ButtonState space;
            ButtonState f1;
            ButtonState f3;
        };
    };

    // Mouse
    Vec2I mouse;
    Vec3I dMouse;
    bool mouseButtonState[5];

    // clock
    float dt;
};

enum class GameMode
{
    InGame,
    Editor,

    TERMINATOR,
};

typedef void UpdateMesh(Renderer* renderContext, u32 modelId, u32 meshId, LoadedMesh mesh);
typedef u32 UploadModel(Renderer* renderContext, LoadedModel initialModel, MemoryArena* arena);
typedef u32 UploadTexture(Renderer* renderContext, Texture texture, MemoryArena* arena);

struct GameAPI
{
    UpdateMesh* UpdateMesh;
    UploadModel* UploadModel;
    UploadTexture* UploadTexture;
};

enum class GameAsset
{
    Cube,
    Terrain,
    CubeRig,
    DefaultTexture,
    BrickTexture,
    SpidyTexture,

    TERMINATOR,
};

struct Entity
{
    Transform transform;
    Box collider;
    GameAsset gfx;
    Material mat;
    u32 animIndex;
    f32 normalizedTime;
    bool isEnabled;
};

struct Chunk
{
    u32 modelId;
    s32 x, z;
};

struct GameState
{
    bool running = true;
    u32 width, height;

    Entity entities[1000];
    Entity* player;

    Asset assets[(int)GameAsset::TERMINATOR];

    // 3x3
    Chunk chunks[9];

    GameMode gameMode;

    float pitch, yaw;

    bool showCursor;
    bool lockCursor;

    Camera camera[(int)GameMode::TERMINATOR];
    //TODO: maybe introduce double buffering if necessary!
    GameInput input;

    GameAPI api;

    float dt;
    float t;

    //TODO: refactor
    Vec3 offset;
};


struct GameMemory
{
    MemoryArena persistantArena;
    MemoryArena transientArena;
};

HPI void Init(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory);
HPI void Update(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory);

#endif