#ifndef HZ_ASSET_H
#define HZ_ASSET_H

#include "hz_asset_format.h"
#include "hz_io.h"


Asset LoadAsset(char* fileName, MemoryArena* arena)
{
    File file = ReadFile(fileName, arena);
    AssetHeader* header = (AssetHeader*)file.content;
    Asset result = {};
    result.type = header->asset.type;
    switch (header->asset.type)
    {
        case AssetType::Model:
        {
            result = header->asset;
            result.loadedModel.meshes = (LoadedMesh*)((u8*)file.content + (u64)header->asset.loadedModel.meshes);
            for (u32 i = 0; i < header->asset.loadedModel.meshCount; ++i)
            {
                LoadedMesh mesh = result.loadedModel.meshes[i];
                mesh.vertices = (Vert*)((char*)file.content + (u64)mesh.vertices);
                mesh.indices = (u32*)((char*)file.content + (u64)mesh.indices);
                mesh.texture.texel = (u32*)((char*)file.content + (u64)mesh.texture.texel);
                result.loadedModel.meshes[i] = mesh;
            }
        }
        break;

        INVALID_DEFAULT_CASE;
    }
    return result;
}

#endif