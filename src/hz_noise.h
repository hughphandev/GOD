#ifndef HZ_NOISE_H
#define HZ_NOISE_H

#define FNL_IMPL
#include <FastNoise/FastNoiseLite.h>

static inline f32 Noise(f32 x, f32 y, f32 z)
{
    fnl_state state = fnlCreateState();
    return fnlGetNoise3D(&state, x, y, z);
}
#endif