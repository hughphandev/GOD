#ifndef HZ_ASSET_H
#define HZ_ASSET_H

#include "hz_asset_format.h"
#include "hz_io.h"

struct Asset
{
    AssetType type;

    union
    {
        LoadedModel loadedModel;
    };

};

Asset LoadAsset(char* fileName, MemoryArena* arena)
{
    File file = ReadFile(fileName, arena);
    AssetHeader* header = (AssetHeader*)file.content;
    Asset result = {};
    result.type = header->type;
    switch (header->type)
    {
        case AssetType::Model:
        {
            result.loadedModel = header->loadedModel;
            result.loadedModel.vertices = (Vert*)((char*)file.content + (u64)header->loadedModel.vertices);
            result.loadedModel.indices = (u32*)((char*)file.content + (u64)header->loadedModel.indices);
            result.loadedModel.texture.texel = (u32*)((char*)file.content + (u64)header->loadedModel.texture.texel);
        }
        break;

        INVALID_DEFAULT_CASE;
    }
    return result;
}

#endif