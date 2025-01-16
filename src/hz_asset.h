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
    result.id = INVALID_VALUE;
    switch (header->asset.type)
    {
        case AssetType::Model:
        {
            result = header->asset;
            result.loadedModel.meshes = (LoadedMesh*)((u64)file.content + (u64)header->asset.loadedModel.meshes);
            result.loadedModel.mats = (Texture*)((u64)file.content + (u64)header->asset.loadedModel.mats);
            result.loadedModel.bones = (Bone*)((u64)file.content + (u64)header->asset.loadedModel.bones);
            for (u32 i = 0; i < result.loadedModel.matCount; ++i)
            {
                result.loadedModel.mats[i].texel = (u32*)((u64)file.content + (u64)result.loadedModel.mats[i].texel);
            }
            for (u32 i = 0; i < header->asset.loadedModel.meshCount; ++i)
            {
                LoadedMesh mesh = result.loadedModel.meshes[i];
                mesh.vertices = (Vert*)((u64)file.content + (u64)mesh.vertices);
                mesh.indices = (u32*)((u64)file.content + (u64)mesh.indices);
                result.loadedModel.meshes[i] = mesh;
            }

            result.animations = FILE_TO_MEMORY_ADDRESS(file.content, result.animations, Animation);
            for (u32 i = 0; i < header->asset.animCount; ++i)
            {
                result.animations[i].channels = FILE_TO_MEMORY_ADDRESS(file.content, result.animations[i].channels, NodeAnim);
                for (u32 key = 0; key < result.animations[i].channelCount; ++key)
                {
                    NodeAnim* nodeAnim = &result.animations[i].channels[key];
                    nodeAnim->positionKeys = FILE_TO_MEMORY_ADDRESS(file.content, nodeAnim->positionKeys, Vec3Key);
                    nodeAnim->rotationKeys = FILE_TO_MEMORY_ADDRESS(file.content, nodeAnim->rotationKeys, QuatKey);
                    nodeAnim->scalingKeys = FILE_TO_MEMORY_ADDRESS(file.content, nodeAnim->scalingKeys, Vec3Key);
                }
            }
        }
        break;

        INVALID_DEFAULT_CASE;
    }
    return result;
}

#endif