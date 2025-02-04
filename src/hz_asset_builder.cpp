#include <stdio.h>
#include <stdlib.h>
#include "hz_asset_format.h"
#include "assimp/cimport.h"        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>    // Post processing flags

bool HasBone(const aiScene* scene, aiString name)
{
    for (u32 i = 0; i < scene->mNumMeshes; ++i)
    {
        for (u32 boneIndex = 0; boneIndex < scene->mMeshes[i]->mNumBones; ++boneIndex)
        {
            if (scene->mMeshes[i]->mBones[boneIndex]->mName == name) return true;
        }
    }
    return false;
}

void ParseNodeHierarchy(MemoryArena* arena, aiNode* node, u32* count, u32 parentIndex, aiString* names)
{
    if (node == NULL) return;
    Bone* bone = PUSH_TYPE(arena, Bone);
    memcpy(&bone->localMatrix, &node->mTransformation, sizeof(Mat4));
    bone->parentIndex = parentIndex;
    parentIndex = *count;
    names[*count] = node->mName;
    ++(*count);
    ASSERT(node->mNumChildren <= MAX_CHILD_BONES);
    for (u32 i = 0; i < node->mNumChildren; ++i)
    {
        bone->childIndices[i] = *count;
        ParseNodeHierarchy(arena, node->mChildren[i], count, parentIndex, names);
    }
}

aiNode* FindRootBone(const aiScene* scene, aiNode* startNode)
{
    if (startNode == NULL) return NULL;
    else if (HasBone(scene, startNode->mName) && !HasBone(scene, startNode->mParent->mName)) return startNode;
    else
    {
        for (u32 i = 0; i < startNode->mNumChildren; ++i)
        {
            aiNode* result = FindRootBone(scene, startNode->mChildren[i]);
            if (result) return result;
        }
        return NULL;
    }
}

void ParseBoneHierarchy(MemoryArena* arena, const aiScene* scene, aiNode* node, u32* count, u32 parentIndex, aiString* names)
{
    if (node == NULL || !HasBone(scene, node->mName)) return;
    Bone* bone = PUSH_TYPE(arena, Bone);
    memcpy(&bone->localMatrix, &node->mTransformation, sizeof(Mat4));
    bone->parentIndex = parentIndex;
    parentIndex = *count;
    names[*count] = node->mName;
    ++(*count);
    ASSERT(node->mNumChildren <= MAX_CHILD_BONES);
    for (u32 i = 0; i < node->mNumChildren; ++i)
    {
        bone->childIndices[i] = *count;
        ParseBoneHierarchy(arena, scene, node->mChildren[i], count, parentIndex, names);
    }
}

void BuildModelAsset(const aiScene* scene, const char* inPath, FILE* out, MemoryArena* arena)
{
    char fullPath[256];
    int count = FindLastIndex(inPath, '\\') + 1;
    Copy(fullPath, inPath, count);
    fullPath[count] = '\0';

    AssetHeader* header = PUSH_TYPE(arena, AssetHeader);
    header->magicNumber = U32CODE('h', 'z', 'a', 'f');
    header->version = 0;
    header->asset.type = AssetType::Model;

    LoadedModel* loadedModel = &header->asset.loadedModel;
    loadedModel->meshCount = scene->mNumMeshes;
    loadedModel->matCount = scene->mNumMaterials;
    loadedModel->meshes = PUSH_ARRAY(arena, LoadedMesh, loadedModel->meshCount);
    loadedModel->mats = PUSH_ARRAY(arena, Texture, loadedModel->matCount);
    loadedModel->bones = (Bone*)((u64)arena->base + (u64)arena->used);
    loadedModel->globalInverseTransform = *(Mat4*)(&scene->mRootNode->mTransformation.Inverse());

    aiString names[200] = {};

    aiNode* rootBone = FindRootBone(scene, scene->mRootNode);

    loadedModel->boneCount = 0;
    ParseBoneHierarchy(arena, scene, rootBone, &loadedModel->boneCount, INVALID_VALUE, names);

    for (u32 i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* aiMaterial = scene->mMaterials[i];
        if (aiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString path;
            if (aiGetMaterialTexture(aiMaterial, aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
            {
                int x, y, comp, req_comp = 4;
                stbi_uc* texel;
                if (path.C_Str()[0] == '*')
                {
                    const aiTexture* aiTex = scene->GetEmbeddedTexture(path.C_Str());
                    texel = stbi_load_from_memory((stbi_uc*)aiTex->pcData, aiTex->mWidth, &x, &y, &comp, req_comp);
                }
                else
                {
                    texel = stbi_load(strcat(fullPath, path.C_Str()), &x, &y, &comp, req_comp);
                }
                loadedModel->mats[i].width = x;
                loadedModel->mats[i].height = y;
                loadedModel->mats[i].texel = PUSH_ARRAY(arena, u32, x * y);
                memcpy(loadedModel->mats[i].texel, texel, sizeof(u32) * x * y);
                loadedModel->mats[i].texel = MEMORY_TO_FILE_ADDRESS(arena->base, loadedModel->mats[i].texel, u32);
            }
        }
    }

    for (u32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh* aiMesh = scene->mMeshes[meshIndex];
        LoadedMesh* mesh = &loadedModel->meshes[meshIndex];
        mesh->vertexCount = aiMesh->mNumVertices;
        mesh->indexCount = aiMesh->mNumFaces * 3;
        Memcpy(&mesh->transform, &scene->mRootNode[meshIndex].mTransformation, sizeof(Mat4));

        mesh->vertices = PUSH_ARRAY(arena, Vert, mesh->vertexCount);
        for (u32 i = 0; i < mesh->vertexCount; ++i)
        {
            mesh->vertices[i].position = { aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z };
            mesh->vertices[i].normal = { aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z };
            mesh->vertices[i].uv = { aiMesh->mTextureCoords[meshIndex][i].x, aiMesh->mTextureCoords[meshIndex][i].y };
        }

        for (u32 boneIndex = 0; boneIndex < aiMesh->mNumBones; ++boneIndex)
        {
            for (u32 vertWeightIndex = 0; vertWeightIndex < aiMesh->mBones[boneIndex]->mNumWeights; ++vertWeightIndex)
            {
                aiVertexWeight vertWeight = aiMesh->mBones[boneIndex]->mWeights[vertWeightIndex];
                Vert* vert = &mesh->vertices[vertWeight.mVertexId];
                int index = FindFirstIndex(vert->weights, 0.0f, MAX_BONES_PER_VERT);
                if (index >= 0)
                {
                    u32 gBoneIndex = FindFirstIndex(names, aiMesh->mBones[boneIndex]->mName, ARRAY_COUNT(names));
                    ASSERT(gBoneIndex >= 0);
                    vert->weights[index] = vertWeight.mWeight;
                    vert->boneIds[index] = gBoneIndex;
                    memcpy(&loadedModel->bones[gBoneIndex].offsetMatrix, &aiMesh->mBones[boneIndex]->mOffsetMatrix, sizeof(Mat4));

                }
                else
                {
                    printf("Not enough bones per vertex! additional bones is culled!\n");
                }
            }
        }

        mesh->indices = PUSH_ARRAY(arena, u32, mesh->indexCount);
        for (u32 i = 0; i < mesh->indexCount; ++i)
        {
            mesh->indices[i] = aiMesh->mFaces[i / 3].mIndices[i % 3];
        }

        mesh->matIndex = scene->mMeshes[meshIndex]->mMaterialIndex;
    }

    for (u32 i = 0; i < loadedModel->meshCount; ++i)
    {
        LoadedMesh* mesh = &loadedModel->meshes[i];
        mesh->vertices = MEMORY_TO_FILE_ADDRESS(arena->base, mesh->vertices, Vert);
        mesh->indices = MEMORY_TO_FILE_ADDRESS(arena->base, mesh->indices, u32);
    }


    loadedModel->bones = MEMORY_TO_FILE_ADDRESS(arena->base, loadedModel->bones, Bone);
    loadedModel->mats = MEMORY_TO_FILE_ADDRESS(arena->base, loadedModel->mats, Texture);
    loadedModel->meshes = MEMORY_TO_FILE_ADDRESS(arena->base, loadedModel->meshes, LoadedMesh);

    header->asset.animCount = scene->mNumAnimations;
    header->asset.animations = PUSH_ARRAY(arena, Animation, header->asset.animCount);
    Animation* animations = header->asset.animations;
    for (u32 i = 0; i < header->asset.animCount; ++i)
    {
        animations[i].channelCount = scene->mAnimations[i]->mNumChannels;
        animations[i].duration = (f32)(scene->mAnimations[i]->mDuration / scene->mAnimations[i]->mTicksPerSecond);

        animations[i].channels = PUSH_ARRAY(arena, NodeAnim, animations[i].channelCount);
        for (u32 chanelIndex = 0; chanelIndex < scene->mAnimations[i]->mNumChannels; ++chanelIndex)
        {
            NodeAnim* nodeAnim = &animations[i].channels[chanelIndex];
            aiNodeAnim* aiNodeAnim = scene->mAnimations[i]->mChannels[chanelIndex];

            nodeAnim->nodeId = FindFirstIndex(names, aiNodeAnim->mNodeName, loadedModel->boneCount);
            nodeAnim->positionKeyCount = aiNodeAnim->mNumPositionKeys;
            nodeAnim->positionKeys = PUSH_ARRAY(arena, Vec3Key, nodeAnim->positionKeyCount);
            for (u32 keyIndex = 0; keyIndex < nodeAnim->positionKeyCount; ++keyIndex)
            {
                nodeAnim->positionKeys[keyIndex].normalizedTime = (f32)(aiNodeAnim->mPositionKeys[keyIndex].mTime / scene->mAnimations[i]->mDuration);
                nodeAnim->positionKeys[keyIndex].value = *((Vec3*)&aiNodeAnim->mPositionKeys[keyIndex].mValue);
            }

            nodeAnim->rotationKeyCount = aiNodeAnim->mNumRotationKeys;
            nodeAnim->rotationKeys = PUSH_ARRAY(arena, QuatKey, nodeAnim->rotationKeyCount);
            for (u32 keyIndex = 0; keyIndex < nodeAnim->rotationKeyCount; ++keyIndex)
            {
                nodeAnim->rotationKeys[keyIndex].normalizedTime = (f32)(aiNodeAnim->mRotationKeys[keyIndex].mTime / scene->mAnimations[i]->mDuration);
                nodeAnim->rotationKeys[keyIndex].value.x = aiNodeAnim->mRotationKeys[keyIndex].mValue.x;
                nodeAnim->rotationKeys[keyIndex].value.y = aiNodeAnim->mRotationKeys[keyIndex].mValue.y;
                nodeAnim->rotationKeys[keyIndex].value.z = aiNodeAnim->mRotationKeys[keyIndex].mValue.z;
                nodeAnim->rotationKeys[keyIndex].value.w = aiNodeAnim->mRotationKeys[keyIndex].mValue.w;
            }

            nodeAnim->scalingKeyCount = aiNodeAnim->mNumScalingKeys;
            nodeAnim->scalingKeys = PUSH_ARRAY(arena, Vec3Key, nodeAnim->scalingKeyCount);
            for (u32 keyIndex = 0; keyIndex < nodeAnim->scalingKeyCount; ++keyIndex)
            {
                nodeAnim->scalingKeys[keyIndex].normalizedTime = (f32)(aiNodeAnim->mScalingKeys[keyIndex].mTime / scene->mAnimations[i]->mDuration);
                nodeAnim->scalingKeys[keyIndex].value = *((Vec3*)&aiNodeAnim->mScalingKeys[keyIndex].mValue);
            }

            nodeAnim->positionKeys = MEMORY_TO_FILE_ADDRESS(arena->base, nodeAnim->positionKeys, Vec3Key);
            nodeAnim->rotationKeys = MEMORY_TO_FILE_ADDRESS(arena->base, nodeAnim->rotationKeys, QuatKey);
            nodeAnim->scalingKeys = MEMORY_TO_FILE_ADDRESS(arena->base, nodeAnim->scalingKeys, Vec3Key);
        }
        animations[i].channels = MEMORY_TO_FILE_ADDRESS(arena->base, animations[i].channels, NodeAnim);
    }
    header->asset.animations = MEMORY_TO_FILE_ADDRESS(arena->base, header->asset.animations, Animation);

    fwrite(arena->base, arena->used, 1, out);
}

void BuildTextureAsset(void* texel, u32 width, u32 height, FILE* out, MemoryArena* arena)
{
    AssetHeader* header = PUSH_TYPE(arena, AssetHeader);
    header->magicNumber = U32CODE('h', 'z', 'a', 'f');
    header->version = 0;
    header->asset.type = AssetType::Texture;
    header->asset.texture.width = width;
    header->asset.texture.height = height;
    header->asset.texture.texel = PUSH_ARRAY(arena, u32, width * height);
    memcpy(header->asset.texture.texel, texel, sizeof(u32) * width * height);
    header->asset.texture.texel = MEMORY_TO_FILE_ADDRESS(arena->base, header->asset.texture.texel, u32);
    fwrite(arena->base, arena->used, 1, out);
}

int main(int argc, char const* argv[])
{
    printf("Packing %s -> %s\n", argv[1], argv[2]);


    char fullPath[256];
    int count = FindLastIndex(argv[1], '\\') + 1;
    Copy(fullPath, argv[1], count);
    fullPath[count] = '\0';

    const aiScene* scene = aiImportFile(argv[1],
                                        aiProcess_CalcTangentSpace |
                                        aiProcess_GenSmoothNormals |
                                        aiProcess_MakeLeftHanded |
                                        aiProcess_FlipUVs |
                                        aiProcess_Triangulate |
                                        aiProcess_JoinIdenticalVertices |
                                        aiProcess_EmbedTextures |
                                        aiProcess_SortByPType);

    int width, height, comp;
    stbi_uc* texel = stbi_load(argv[1], &width, &height, &comp, 4);

    size_t totalSize = GIGABYTES(1);
    MemoryArena arena;
    InitMemoryArena(&arena, totalSize, malloc(totalSize));

    FILE* out = fopen(argv[2], "wb");
    if (out)
    {
        if (scene != NULL) {

            BuildModelAsset(scene, argv[1], out, &arena);
            fclose(out);
        }
        else if (texel)
        {
            BuildTextureAsset(texel, width, height, out, &arena);
            fclose(out);
        }
        else
        {
            fclose(out);
            remove(argv[2]);
            printf("Unable to pack:%s\n", argv[1]);
        }
    }

    printf("Packing completed\n");
    return 0;
}