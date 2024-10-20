#ifndef GAME_H
#define GAME_H

#include "hz_define.h"
#include "hz_render.h"

struct Entity
{
    Transform transform;
    Model model;
};

struct ButtonState
{
    bool isDown;
    int halfTransitionCount;
};

struct GameInput
{
    // button
    union
    {
        ButtonState buttons[8];
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
    i32 mouseX, mouseY, mouseZ;
    i32 dMouseX, dMouseY;
    bool mouseButtonState[5];

    // clock
    float dt;
};

struct GameState
{
    bool running = true;
    char* tittle;
    int width, height;
    Model testModel;

    Camera camera;
    //TODO: maybe introduce double buffering if necessary!
    GameInput input;
};


struct GameMemory
{
    MemoryArena persistantArena;
    MemoryArena transientArena;
};

HPI void Init(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory);
HPI void Update(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory);

#endif