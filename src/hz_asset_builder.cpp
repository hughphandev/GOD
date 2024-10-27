#include <stdio.h>
#include <stdlib.h>
#include "hz_asset_format.h"
#include "assimp/cimport.h"        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags

int main(int argc, char const* argv[])
{
    const struct aiScene* scene = aiImportFile("obj.obj",
                                               aiProcess_CalcTangentSpace |
                                               aiProcess_Triangulate |
                                               aiProcess_JoinIdenticalVertices |
                                               aiProcess_SortByPType);
    if (NULL == scene) {
        printf(aiGetErrorString());
        return false;
    }

    AssetHeader header = {};
    header.magicNumber = U32CODE('h', 'z', 'a', 'f');
    header.version = 0;
    header.type = AssetType::Model;

    header.modelAssetInfo.vertCount = scene->mMeshes[0]->mNumVertices;
    header.modelAssetInfo.indexCount = scene->mMeshes[0]->mNumFaces * 3;

    header.modelAssetInfo.vert = sizeof(header);
    header.modelAssetInfo.index = header.modelAssetInfo.vert + header.modelAssetInfo.vertCount * sizeof(Vert);

    char path[256];
    sprintf_s(path, "%s\\%s", argv[1], "obj.hza");
    FILE* out = fopen(path, "wb");
    if (out)
    {
        fwrite(&header, sizeof(header), 1, out);
        for (u32 i = 0; i < header.modelAssetInfo.vertCount; ++i)
        {
            Vert vert = {};
            vert.pos = { scene->mMeshes[0]->mVertices[i].x, scene->mMeshes[0]->mVertices[i].y, scene->mMeshes[0]->mVertices[i].z };
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

        fclose(out);
    }


    return 0;
}
