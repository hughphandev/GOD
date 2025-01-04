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

    char fullPath[256];
    int count = FindLastIndex((char*)argv[2], '\\') + 1;
    Copy(fullPath, (char*)argv[2], count);
    fullPath[count] = '\0';

    AssetHeader header = {};
    header.magicNumber = U32CODE('h', 'z', 'a', 'f');
    header.version = 0;
    header.asset.type = AssetType::Model;

    header.asset.loadedModel.meshCount = scene->mNumMeshes;

    FILE* out = fopen(argv[1], "wb");
    if (out)
    {
        header.asset.loadedModel.meshes = (LoadedMesh*)sizeof(header);
        fwrite(&header, sizeof(header), 1, out);

        Texture* textures = (Texture*)malloc(sizeof(textures) * scene->mNumTextures);
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
                    textures[i].width = x;
                    textures[i].height = y;
                    textures[i].texel = (u32*)texel;
                }
            }
        }

        void* base = (u8*)(sizeof(header) + sizeof(LoadedMesh) * scene->mNumMeshes);
        for (u32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
        {
            LoadedMesh mesh = {};
            mesh.vertexCount = scene->mMeshes[meshIndex]->mNumVertices;
            mesh.indexCount = scene->mMeshes[meshIndex]->mNumFaces * 3;
            Memcpy(&mesh.transform, &scene->mRootNode[meshIndex].mTransformation, sizeof(Mat4));

            mesh.vertices = (Vert*)base;
            base = (u8*)base + scene->mMeshes[meshIndex]->mNumVertices * sizeof(Vert);

            mesh.indices = (u32*)base;
            base = (u8*)base + scene->mMeshes[meshIndex]->mNumFaces * 3 * sizeof(u32);

            mesh.texture.width = (u32)textures[meshIndex].width;
            mesh.texture.height = (u32)textures[meshIndex].height;
            mesh.texture.texel = (u32*)(base);
            base = (u8*)base + textures[meshIndex].width * textures[meshIndex].height * sizeof(u32);

            fwrite(&mesh, sizeof(mesh), 1, out);
        }

        for (u32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
        {
            for (u32 i = 0; i < scene->mMeshes[meshIndex]->mNumVertices; ++i)
            {
                Vert vert = {};
                vert.possition = { scene->mMeshes[meshIndex]->mVertices[i].x, scene->mMeshes[meshIndex]->mVertices[i].y, scene->mMeshes[meshIndex]->mVertices[i].z };
                vert.normal = { scene->mMeshes[meshIndex]->mNormals[i].x, scene->mMeshes[meshIndex]->mNormals[i].y, scene->mMeshes[meshIndex]->mNormals[i].z };
                vert.uv = { scene->mMeshes[meshIndex]->mTextureCoords[meshIndex][i].x, scene->mMeshes[meshIndex]->mTextureCoords[meshIndex][i].y };
                fwrite(&vert, sizeof(vert), 1, out);
            }

            for (u32 i = 0; i < scene->mMeshes[meshIndex]->mNumFaces; ++i)
            {
                for (u32 j = 0; j < scene->mMeshes[meshIndex]->mFaces[i].mNumIndices; ++j)
                {
                    u32 index = scene->mMeshes[meshIndex]->mFaces[i].mIndices[j];
                    fwrite(&index, sizeof(index), 1, out);
                }
            }

            Texture texture = textures[scene->mMeshes[meshIndex]->mMaterialIndex];
            fwrite(texture.texel, sizeof(u32), texture.width * texture.height, out);
        }

        fclose(out);
    }
    else
    {
        printf("Can't open output file!");
    }

    printf("Packing completed\n");
    return 0;
}