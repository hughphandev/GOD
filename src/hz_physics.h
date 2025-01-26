#ifndef HZ_PHYSICS_H
#define HZ_PHYSICS_H

#include "hz_math.h"

struct Box
{
    Vec3 position;
    Vec3 extents;
};

bool OverlapBox(Box b1, Box b2)
{
    Vec3 deltaPos = b2.position - b1.position;
    Vec3 distance = b1.extents + b2.extents;
    return Abs(deltaPos.x) <= Abs(distance.x) && Abs(deltaPos.y) <= Abs(distance.y) && Abs(deltaPos.z) <= Abs(distance.z);
}

Box UnionBox(Box b1, Box b2)
{
    Vec3 deltaPos = b2.position - b1.position;
    Vec3 distance = b1.extents + b2.extents;
    f32 tX = (b1.extents.x / distance.x);
    f32 tY = (b1.extents.y / distance.y);
    f32 tZ = (b1.extents.z / distance.z);

    Box result;
    result.position = {
        b1.position.x + distance.x * tX,
        b1.position.y + distance.y * tY,
        b1.position.z + distance.z * tZ,
    };
    result.extents = distance - Abs(deltaPos);
    return result;
}

#endif