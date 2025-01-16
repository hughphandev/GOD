#ifndef HZ_ANIM_H
#define HZ_ANIM_H

#include "hz_math.h"

#define MAX_BONES_PER_VERT 4
#define MAX_CHILD_BONES 20
struct Bone
{
    Mat4 offsetMatrix;
    Mat4 localMatrix;
    s32 parentIndex;
    s32 childIndices[MAX_CHILD_BONES];
};

struct Vec3Key
{
    f32 normalizedTime;
    Vec3 value;
};

struct QuatKey
{
    f32 normalizedTime;
    Quaternion value;
};

enum AnimBehaviour
{

};


struct NodeAnim
{
    u32 nodeId;

    u32 positionKeyCount;
    Vec3Key* positionKeys;

    u32 rotationKeyCount;
    QuatKey* rotationKeys;

    u32 scalingKeyCount;
    Vec3Key* scalingKeys;

    AnimBehaviour preState;
    AnimBehaviour postState;
};

struct Animation
{
    f32 duration;
    u32 channelCount;
    NodeAnim* channels;
};


struct NodeTransform
{
    u32 id;
    Mat4 transform;
};

Vec3 GetValue(Vec3Key* keys, u32 keyCount, float t)
{
    ASSERT(keyCount > 0);
    u32 i = 0;
    for (; t <= keys[i].normalizedTime && i < keyCount; ++i);
    if (i == keyCount) return keys[keyCount - 1].value;
    else if (i == 0)return keys[0].value;
    else
    {
        return Lerp(keys[i - 1].value, keys[1].value, (t - keys[i - 1].normalizedTime) / (keys[i].normalizedTime - keys[i - 1].normalizedTime));
    }
}

Quaternion GetValue(QuatKey* keys, u32 keyCount, float t)
{
    ASSERT(keyCount > 0);
    u32 i = 0;
    for (; t <= keys[i].normalizedTime && i < keyCount; ++i);
    if (i == keyCount) return keys[keyCount - 1].value;
    else if (i == 0)return keys[0].value;
    else
    {
        return Lerp(keys[i - 1].value, keys[1].value, (t - keys[i - 1].normalizedTime) / (keys[i].normalizedTime - keys[i - 1].normalizedTime));
    }
}

NodeTransform* ReadNodeTransform(Animation anim, float t, MemoryArena* arena)
{
    NodeTransform* result = PUSH_ARRAY(arena, NodeTransform, anim.channelCount);
    for (u32 i = 0; i < anim.channelCount; ++i)
    {
        Vec3 pos = GetValue(anim.channels[i].positionKeys, anim.channels[i].positionKeyCount, t);
        Quaternion rotation = GetValue(anim.channels[i].rotationKeys, anim.channels[i].rotationKeyCount, t);
        Vec3 scale = GetValue(anim.channels[i].scalingKeys, anim.channels[i].scalingKeyCount, t);

        result[i].transform = TRS(pos, rotation, scale);
        result[i].id = anim.channels[i].nodeId;
    }
    return result;
}

u32 FindFirstIndex(NodeTransform* nodes, u32 nodeCount, u32 id)
{
    for (u32 i = 0; i < nodeCount; ++i)
    {
        if (nodes[i].id == id) return i;
    }
    return INVALID_VALUE;
}

#endif