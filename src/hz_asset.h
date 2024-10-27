#ifndef HZ_ASSET_H
#define HZ_ASSET_H

#include "hz_asset_format.h"
#include "hz_io.h"

struct Asset
{
    AssetType type;

    union
    {
        LoadedModel model;
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
            result.model.vertexCount = header->modelAssetInfo.vertCount;
            result.model.indexCount = header->modelAssetInfo.indexCount;
            result.model.vertices = (Vert*)((char*)file.content + header->modelAssetInfo.vert);
            result.model.indices = (u32*)((char*)file.content + header->modelAssetInfo.index);
        }
        break;

        INVALID_DEFAULT_CASE;
    }
    return result;
}

#endif