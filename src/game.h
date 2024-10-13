#ifndef GAME_H
#define GAME_H

#include "hz_define.h"
#include "hz_render.h"

struct GameState
{
    LoadedModel testModel;
};

struct GameMemory
{
    MemoryArena persistantArena;
    MemoryArena transientArena;
};

HPI void Init(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory);
HPI void Update(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory);

#endif