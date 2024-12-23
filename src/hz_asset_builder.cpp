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
    printf("Packing %s -> %s", argv[2], argv[1]);
    const struct aiScene* scene = aiImportFile(argv[2],
                                               aiProcess_CalcTangentSpace |
                                               aiProcess_Triangulate |
                                               aiProcess_FlipUVs |
                                               aiProcess_JoinIdenticalVertices |
                                               aiProcess_SortByPType);
    if (NULL == scene) {
        printf("Error!\n");
        printf(aiGetErrorString());
        return false;
    }

    int x, y, comp, req_comp = 4;
    stbi_uc* texel = stbi_load_from_memory((stbi_uc*)scene->mTextures[0]->pcData, scene->mTextures[0]->mWidth, &x, &y, &comp, req_comp);

    AssetHeader header = {};
    header.magicNumber = U32CODE('h', 'z', 'a', 'f');
    header.version = 0;
    header.type = AssetType::Model;

    header.loadedModel.vertexCount = scene->mMeshes[0]->mNumVertices;
    header.loadedModel.indexCount = scene->mMeshes[0]->mNumFaces * 3;

    header.loadedModel.texture.width = x;
    header.loadedModel.texture.height = y;

    header.loadedModel.vertices = (Vert*)sizeof(header);
    header.loadedModel.indices = (u32*)((char*)header.loadedModel.vertices + header.loadedModel.vertexCount * sizeof(Vert));
    header.loadedModel.texture.texel = (u32*)((char*)header.loadedModel.indices + header.loadedModel.indexCount * sizeof(u32));

    Memcpy(&header.loadedModel.transform, &scene->mRootNode[0].mTransformation, sizeof(Mat4));

    FILE* out = fopen(argv[1], "wb");
    if (out)
    {
        fwrite(&header, sizeof(header), 1, out);
        for (u32 i = 0; i < header.loadedModel.vertexCount; ++i)
        {
            Vert vert = {};
            vert.possition = { scene->mMeshes[0]->mVertices[i].x, scene->mMeshes[0]->mVertices[i].y, scene->mMeshes[0]->mVertices[i].z };
            vert.normal = { scene->mMeshes[0]->mNormals[i].x, scene->mMeshes[0]->mNormals[i].y, scene->mMeshes[0]->mNormals[i].z };
            vert.uv = { scene->mMeshes[0]->mTextureCoords[0][i].x, scene->mMeshes[0]->mTextureCoords[0][i].y };
            fwrite(&vert, sizeof(vert), 1, out);
        }

        for (u32 i = 0; i < scene->mMeshes[0]->mNumFaces; ++i)
        {
            for (u32 j = 0; j < scene->mMeshes[0]->mFaces[i].mNumIndices; ++j)
            {
                u32 index = scene->mMeshes[0]->mFaces[i].mIndices[j];
                fwrite(&index, sizeof(index), 1, out);
            }
        }
        fwrite(texel, sizeof(stbi_uc) * req_comp, header.loadedModel.texture.width * header.loadedModel.texture.height, out);
        fclose(out);
    }
    else
    {
        printf("Can't open output file!");
    }

    printf("Packing completed");
    return 0;
}