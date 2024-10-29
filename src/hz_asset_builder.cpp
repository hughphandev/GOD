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
    const struct aiScene* scene = aiImportFile("cube.glb",
                                               aiProcess_CalcTangentSpace |
                                               aiProcess_Triangulate |
                                               aiProcess_FlipUVs |
                                               aiProcess_JoinIdenticalVertices |
                                               aiProcess_SortByPType);
    if (NULL == scene) {
        printf(aiGetErrorString());
        return false;
    }

    int x, y, comp, req_comp = 4;
    stbi_uc* texel = stbi_load_from_memory((stbi_uc*)scene->mTextures[0]->pcData, scene->mTextures[0]->mWidth, &x, &y, &comp, req_comp);
    // stbi_uc* texel = stbi_load("brick-texture-2106361449.jpg", &x, &y, &comp, req_comp);

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

    char path[256];
    sprintf_s(path, "%s\\%s", argv[1], "obj.hza");
    FILE* out = fopen(path, "wb");
    if (out)
    {
        fwrite(&header, sizeof(header), 1, out);
        for (u32 i = 0; i < header.loadedModel.vertexCount; ++i)
        {
            Vert vert = {};
            vert.pos = { scene->mMeshes[0]->mVertices[i].x, scene->mMeshes[0]->mVertices[i].y, scene->mMeshes[0]->mVertices[i].z };
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


    return 0;
}
