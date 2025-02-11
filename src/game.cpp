#include "hz_io.h"
#include "hz_world.h"
#include "game.h"

Entity* CreateEntity(Vec3 pos, Quaternion rot, Vec3 scale, GameAsset gfx, Material mat, GameState* state)
{
    for (u32 i = 0; i < ARRAY_COUNT(state->entities); ++i)
    {
        if (!state->entities[i].isEnabled)
        {
            state->entities[i].isEnabled = true;
            state->entities[i].transform.position = pos;
            state->entities[i].transform.rotation = rot;
            state->entities[i].transform.scale = scale;
            state->entities[i].collider.position = pos;
            state->entities[i].collider.extents = 0.5f * scale;
            state->entities[i].gfx = gfx;
            state->entities[i].mat = mat;
            state->entities[i].animIndex = INVALID_VALUE;
            return &state->entities[i];
        }
    }
    return NULL;
}

void Destroy(Entity* entity)
{
    entity->isEnabled = false;
}

HPI void Init(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory)
{
    state->running = true;

    for (int i = 0; i < (int)GameMode::TERMINATOR; ++i)
    {
        state->camera[i].worldUp = { 0, 1, 0 };
        state->camera[i].fovy = 60.0f * DEG2RAD;
        state->camera[i].aspect = (float)state->width / state->height;
        state->camera[i].position = { 0, 5, -5 };
        state->camera[i].direction = { 0, -1, 1 };
    }

    state->gameMode = GameMode::InGame;
    state->dt = 1 / 60.0f;

    // state->assets[(int)GameAsset::Sphere] = LoadAsset("asset\\sphere.hza", &gameMemory->persistantArena);
    // state->assets[(int)GameAsset::CubeRig] = LoadAsset("asset\\cube-rig.hza", &gameMemory->persistantArena);
    // state->assets[(int)GameAsset::DefaultTexture] = GenAssetTexture(GenTexture(1, 1, { 1, 1, 1, 1 }, &gameMemory->persistantArena));
    // state->assets[(int)GameAsset::BrickTexture] = LoadAsset("asset\\brick-texture-2106361449.hza", &gameMemory->persistantArena);
    // state->assets[(int)GameAsset::SpidyTexture] = LoadAsset("asset\\Char_GhostSpider_D.hza", &gameMemory->persistantArena);

    state->assets[(int)GameAsset::Cube] = LoadAsset("asset\\cube.hza", &gameMemory->persistantArena);
    state->assets[(int)GameAsset::Cube].id = state->api.UploadModel(renderGroup->renderer, state->assets[(int)GameAsset::Cube].loadedModel, &gameMemory->persistantArena);
    state->assets[(int)GameAsset::Terrain] = LoadAsset("asset\\cube.hza", &gameMemory->persistantArena);
    state->assets[(int)GameAsset::Terrain].id = state->api.UploadModel(renderGroup->renderer, state->assets[(int)GameAsset::Terrain].loadedModel, &gameMemory->persistantArena);

    state->assets[(int)GameAsset::DefaultTexture] = GenAssetTexture(GenTexture(1, 1, { 1, 1, 1, 1 }, &gameMemory->persistantArena));
    state->assets[(int)GameAsset::DefaultTexture].id = state->api.UploadTexture(renderGroup->renderer, state->assets[(int)GameAsset::DefaultTexture].texture, &gameMemory->persistantArena);

    Material brickMat = {};
    brickMat.color = { 1, 1, 1, 1 };
    brickMat.textureCount = 1;
    brickMat.textureId = 0;

    Material spidyMat = {};
    spidyMat.color = { 1, 1, 1, 1 };
    spidyMat.textureCount = 1;
    spidyMat.textureId = &state->assets[(int)GameAsset::SpidyTexture].id;

    renderGroup->lightDirection = Normalize(Vec3{ -1, -1, 0 });
    renderGroup->diffuse = { 1, 1, 1, 1 };

    // state->player = CreateEntity({ 0, 0.5f, 0 }, {}, { 1, 1, 1 }, GameAsset::Cube, spidyMat, state);
    // state->player->animIndex = 0;
}

HPI void Update(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory)
{
    if (state->input.f1.isDown && state->input.f1.halfTransitionCount > 0)
    {
        state->gameMode = (GameMode)(((int)state->gameMode + 1) % (int)GameMode::TERMINATOR);
        OutputDebugString("F1");
    }

    switch (state->gameMode)
    {
        case GameMode::InGame:
        {
            state->lockCursor = true;
            state->showCursor = false;

            f32 speed = 10;
            if (state->input.up.isDown)
            {
                state->offset.z += state->dt * speed;
            }
            if (state->input.down.isDown)
            {
                state->offset.z -= state->dt * speed;
            }
            if (state->input.left.isDown)
            {
                state->offset.x -= state->dt * speed;
            }
            if (state->input.right.isDown)
            {
                state->offset.x += state->dt * speed;
            }
            if (state->input.space.isDown)
            {
                state->offset.y += state->dt * speed;
            }


            for (int i = 0; i < ARRAY_COUNT(state->entities); ++i)
            {
                if (state->player && state->player != &state->entities[i])
                {
                    Box col = IntersectBox(state->player->collider, state->entities[i].collider);
                    if (col.extents > EPSILON)
                    {
                        int minIndex = 0;
                        for (int index = 1; index < ARRAY_COUNT(col.extents.elements); ++index)
                        {
                            if (col.extents.elements[minIndex] > col.extents.elements[index]) minIndex = index;
                        }
                        state->player->transform.position.elements[minIndex] += (col.position.elements[minIndex] > state->entities[i].collider.position.elements[minIndex] ? col.extents.elements[minIndex] : -col.extents.elements[minIndex]);
                        state->player->collider.position = state->player->transform.position;
                    }
                }
            }
        }break;

        case GameMode::Editor:
        {
            float sensitivity = 0.1f;
            state->yaw -= state->input.dMouse.x * sensitivity;
            state->pitch -= state->input.dMouse.y * sensitivity;
            if (state->pitch > 89.0f)
                state->pitch = 89.0f;
            if (state->pitch < -89.0f)
                state->pitch = -89.0f;
            state->camera[(int)state->gameMode].direction = GetCamDir(state->pitch * DEG2RAD, state->yaw * DEG2RAD);
            float speed = 10;

            if (state->input.up.isDown)
            {
                state->camera[(int)state->gameMode].position += state->dt * state->camera[(int)state->gameMode].direction * speed;
            }
            if (state->input.down.isDown)
            {
                state->camera[(int)state->gameMode].position -= state->dt * state->camera[(int)state->gameMode].direction * speed;
            }
            if (state->input.left.isDown)
            {
                state->camera[(int)state->gameMode].position -= state->dt * Normalize(Cross(state->camera[(int)state->gameMode].worldUp, state->camera[(int)state->gameMode].direction)) * speed;
            }
            if (state->input.right.isDown)
            {
                state->camera[(int)state->gameMode].position += state->dt * Normalize(Cross(state->camera[(int)state->gameMode].worldUp, state->camera[(int)state->gameMode].direction)) * speed;
            }
            state->lockCursor = false;
            state->showCursor = true;
            //TODO: edit
        }break;
        INVALID_DEFAULT_CASE
    }

    // f32 div = state->t / state->assets[(int)state->player->gfx].animations[state->player->animIndex].duration;
    // state->player->normalizedTime = div - Floor(div);

    PushRenderClear(renderGroup, { 0.5f, 0.5f, 0.5f, 1.0f });
#define CHUNKS 16

    Material mat = {};
    mat.textureCount = 1;
    mat.textureId = 0;
    mat.color = { 1, 1, 1, 1 };

    f32* densities = PUSH_ARRAY(&gameMemory->transientArena, f32, CHUNKS * CHUNKS * CHUNKS);
    for (int x = 0; x < CHUNKS; ++x)
    {
        for (int y = 0; y < CHUNKS; ++y)
        {
            for (int z = 0; z < CHUNKS; ++z)
            {
                Vec3 coord = (state->offset + 10.0f * Vec3{ (f32)x, (f32)y, (f32)z });
                f32 noise = (Noise(coord.x, coord.y, coord.z) + 1.0f) / 2.0f;
                densities[(u32)(x * CHUNKS * CHUNKS + y * CHUNKS + z)] = noise;

                // mat.color = Color{ 0, noise > 0.5f ? 1.0f : 0, 0, 1 };
                // PushRenderModel(renderGroup, &state->camera[(u32)state->gameMode], state->assets[(u32)GameAsset::Cube].id, mat, TRS({ (f32)x, (f32)y, (f32)z }, {}, { 0.1f, 0.1f, 0.1f }), 0, 0, {}, 0, 0);
            }
        }
    }

    Vec3 corners[] = {
        {0, 0, 0},{1, 0, 0},{1, 1, 0},{0, 1, 0},
        {0, 0, 1},{1, 0, 1},{1, 1, 1},{0, 1, 1}
    };

    LoadedMesh mesh = {};
    mesh.transform = MAT4_IDENTITY;
    mesh.vertices = PUSH_MARK(&gameMemory->transientArena, Vert);
    u32 trisCount = 0;
    for (int x = 0; x < CHUNKS - 1; ++x)
    {
        for (int y = 0; y < CHUNKS - 1; ++y)
        {
            for (int z = 0; z < CHUNKS - 1; ++z)
            {
                GRIDCELL cell = {};
                Vec3 pos = Vec3{ (f32)x, (f32)y, (f32)z };
                for (u32 i = 0; i < ARRAY_COUNT(cell.p); ++i)
                {
                    cell.p[i] = pos + corners[i];
                    cell.val[i] = densities[(u32)(cell.p[i].x * CHUNKS * CHUNKS + cell.p[i].y * CHUNKS + cell.p[i].z)];
                }

                trisCount += MarchingCube(cell, 0.5f, &gameMemory->transientArena);
            }
        }
    }

    if (trisCount > 0)
    {
        mesh.indices = PUSH_ARRAY(&gameMemory->transientArena, u32, trisCount * 3);
        for (u32 i = 0; i < trisCount * 3; ++i)
        {
            mesh.indices[i] = i;
        }

        mesh.vertexCount = trisCount * 3;
        mesh.indexCount = trisCount * 3;

        mat.color = { 0, 0.5f, 0, 1 };
        state->api.UpdateMesh(renderGroup->renderer, state->assets[(u32)GameAsset::Terrain].id, 0, mesh);
        PushRenderModel(renderGroup, &state->camera[(u32)state->gameMode], state->assets[(u32)GameAsset::Terrain].id, mat, MAT4_IDENTITY, 0, 0, {}, 0, 0);

    }

    // for (int i = 0; i < ARRAY_COUNT(state->entities); ++i)
    // {
    //     Entity entity = state->entities[i];
    //     if (entity.isEnabled)
    //     {
    //         Asset asset = state->assets[(int)entity.gfx];
    //         if (entity.animIndex == INVALID_VALUE)
    //         {
    //             PushRenderModel(renderGroup, &state->camera[(int)state->gameMode], asset.id, entity.mat, TRS(entity.transform.position, entity.transform.rotation, entity.transform.scale), 0, NULL, {}, 0, NULL);
    //         }
    //         else
    //         {
    //             NodeTransform* trans = ReadNodeTransform(asset.animations[entity.animIndex], entity.normalizedTime, &gameMemory->transientArena);
    //             PushRenderModel(renderGroup, &state->camera[(int)state->gameMode], asset.id, entity.mat, Translate(state->player->transform.position), asset.loadedModel.boneCount, asset.loadedModel.bones, asset.loadedModel.globalInverseTransform, asset.animations[entity.animIndex].channelCount, trans);
    //         }

    //     }
    // }
}