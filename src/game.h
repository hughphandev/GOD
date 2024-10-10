#ifndef GAME_H
#define GAME_H

#include "hz_render.h"

struct GameState
{
    RenderGroup renderGroup;

};

struct GameMemory
{
    MemoryArena persistantArena;
    MemoryArena transientArena;
};


#endif