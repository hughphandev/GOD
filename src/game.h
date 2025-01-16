#ifndef GAME_H
#define GAME_H

#include "hz_define.h"
#include "hz_render.h"
#include "hz_asset.h"

struct Entity
{
    Transform transform;
};

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

struct GameAPI
{

};

enum class GameAsset
{
    Cube,
    Sphere,
    Spindy,
    TERMINATOR,
};

struct GameState
{
    bool running = true;
    char* tittle;
    int width, height;

    Asset assets[(int)GameAsset::TERMINATOR];

    GameMode gameMode;

    float yaw, pitch;

    bool showCursor;
    bool lockCursor;

    Camera camera;
    //TODO: maybe introduce double buffering if necessary!
    GameInput input;

    GameAPI api;

    float dt;
    float t;
};


struct GameMemory
{
    MemoryArena persistantArena;
    MemoryArena transientArena;
};

HPI void Init(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory);
HPI void Update(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory);

#endif