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

struct AssetHeader
{
    u32 magicNumber;
    u32 version;

    AssetType type;
    union
    {
        LoadedModel loadedModel;
    };
};
#endif