#ifndef HZ_ASSET_FORMAT_H
#define HZ_ASSET_FORMAT_H
#include "hz_types.h"
#include "hz_render.h"

enum class AssetType
{
    None,
    Model,
    Audio,
    Texture,
};

struct Asset
{
    AssetType type;
    u32 id;

    union
    {
        struct
        {
            LoadedModel loadedModel;
            u32 animCount;
            Animation* animations;
        };
        Texture texture;
    };
};

struct AssetHeader
{
    u32 magicNumber;
    u32 version;

    Asset asset;
};
#endif