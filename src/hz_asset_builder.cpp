#include <stdio.h>
#include <stdlib.h>
#include "hz_asset_format.h"
#include "assimp/cimport.h"        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>    // Post processing flags

int main(int argc, char const* argv[])
{
    printf("Packing %s -> %s\n", argv[2], argv[1]);
    const struct aiScene* scene = aiImportFile(argv[2],
                                               aiProcess_CalcTangentSpace |
                                               aiProcess_GenSmoothNormals |
                                               aiProcess_MakeLeftHanded |
                                               aiProcess_FlipUVs |
                                               aiProcess_Triangulate |
                                               aiProcess_JoinIdenticalVertices |
                                               aiProcess_SortByPType);
    if (NULL == scene) {
        printf("Error!\n");
        printf(aiGetErrorString());
        return false;
    }

    size_t totalSize = GIGABYTES(1);
    MemoryArena arena;
    InitMemoryArena(&arena, totalSize, malloc(totalSize));

    char fullPath[256];
    int count = FindLastIndex((char*)argv[2], '\\') + 1;
    Copy(fullPath, (char*)argv[2], count);
    fullPath[count] = '\0';

    FILE* out = fopen(argv[1], "wb");
    if (out)
    {
        AssetHeader* header = PUSH_TYPE(&arena, AssetHeader);
        header->magicNumber = U32CODE('h', 'z', 'a', 'f');
        header->version = 0;
        header->asset.type = AssetType::Model;

        LoadedModel* loadedModel = &header->asset.loadedModel;
        loadedModel->meshCount = scene->mNumMeshes;
        loadedModel->matCount = scene->mNumMaterials;
        loadedModel->meshes = PUSH_ARRAY(&arena, LoadedMesh, loadedModel->meshCount);
        loadedModel->mats = PUSH_ARRAY(&arena, Texture, loadedModel->matCount);

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
                    loadedModel->mats[i].texel = PUSH_ARRAY(&arena, u32, x * y);
                    memcpy(loadedModel->mats[i].texel, texel, sizeof(u32) * x * y);
                    loadedModel->mats[i].texel = (u32*)((u64)loadedModel->mats[i].texel - (u64)arena.base);
                }
            }
            loadedModel->mats = (Texture*)((u64)loadedModel->mats - (u64)arena.base);
        }


        for (u32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
        {
            aiMesh* aiMesh = scene->mMeshes[meshIndex];
            LoadedMesh* mesh = &loadedModel->meshes[meshIndex];
            mesh->vertexCount = aiMesh->mNumVertices;
            mesh->indexCount = aiMesh->mNumFaces * 3;
            mesh->boneCount = aiMesh->mNumBones;
            Memcpy(&mesh->transform, &scene->mRootNode[meshIndex].mTransformation, sizeof(Mat4));

            mesh->vertices = PUSH_ARRAY(&arena, Vert, mesh->vertexCount);
            for (u32 i = 0; i < mesh->vertexCount; ++i)
            {
                mesh->vertices[i].possition = { aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z };
                mesh->vertices[i].normal = { aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z };
                mesh->vertices[i].uv = { aiMesh->mTextureCoords[meshIndex][i].x, aiMesh->mTextureCoords[meshIndex][i].y };
            }

            mesh->indices = PUSH_ARRAY(&arena, u32, mesh->indexCount);
            for (u32 i = 0; i < mesh->indexCount; ++i)
            {
                mesh->indices[i] = aiMesh->mFaces[i / 3].mIndices[i % 3];
            }

            mesh->bones = PUSH_ARRAY(&arena, Bone, mesh->boneCount);
            for (u32 i = 0; i < aiMesh->mNumBones; ++i)
            {
                mesh->bones[i].weightCount = aiMesh->mBones[i]->mNumWeights;
                Memcpy(&mesh->bones[i].offsetMatrix, &aiMesh->mBones[i]->mOffsetMatrix, sizeof(Mat4));
                mesh->bones[i].weights = PUSH_ARRAY(&arena, VertWeight, mesh->bones[i].weightCount);
            }


            mesh->matIndex = scene->mMeshes[meshIndex]->mMaterialIndex;

        }

        for (u32 i = 0; i < loadedModel->meshCount; ++i)
        {
            LoadedMesh* mesh = &loadedModel->meshes[i];
            for (u32 boneIndex = 0; boneIndex < mesh->boneCount; ++boneIndex)
            {
                mesh->bones[boneIndex].weights = (VertWeight*)((u64)mesh->bones[boneIndex].weights - (u64)arena.base);
            }
            mesh->bones = (Bone*)((u64)mesh->bones - (u64)arena.base);
            mesh->vertices = (Vert*)((u64)mesh->vertices - (u64)arena.base);
            mesh->indices = (u32*)((u64)mesh->indices - (u64)arena.base);
        }
        loadedModel->meshes = (LoadedMesh*)((u64)loadedModel->meshes - (u64)arena.base);

        fwrite(arena.base, arena.used, 1, out);

        fclose(out);
    }
    else
    {
        printf("Can't open output file!");
    }

    printf("Packing completed\n");
    return 0;
}