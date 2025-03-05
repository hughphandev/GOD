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
        state->camera[i].aspect = (f32)state->width / state->height;
        state->camera[i].position = { 0, 0, 1 };
        state->camera[i].direction = { 0, 0, 1 };
    }

    state->yaw = 90.0f;

    state->gameMode = GameMode::InGame;
    state->dt = 1 / 60.0f;

    // state->assets[(int)GameAsset::Sphere] = LoadAsset("asset\\sphere.hza", &gameMemory->persistantArena);
    // state->assets[(int)GameAsset::CubeRig] = LoadAsset("asset\\cube-rig.hza", &gameMemory->persistantArena);
    // state->assets[(int)GameAsset::DefaultTexture] = GenAssetTexture(GenTexture(1, 1, { 1, 1, 1, 1 }, &gameMemory->persistantArena));
    // state->assets[(int)GameAsset::BrickTexture] = LoadAsset("asset\\brick-texture-2106361449.hza", &gameMemory->persistantArena);
    // state->assets[(int)GameAsset::SpidyTexture] = LoadAsset("asset\\Char_GhostSpider_D.hza", &gameMemory->persistantArena);

    // state->assets[(int)GameAsset::Cube] = LoadAsset("asset\\cube.hza", &gameMemory->persistantArena);
    // state->assets[(int)GameAsset::Cube].id = state->api.UploadModel(renderGroup->renderer, state->assets[(int)GameAsset::Cube].loadedModel, &gameMemory->persistantArena);

    // state->assets[(int)GameAsset::DefaultTexture] = GenAssetTexture(GenTexture(1, 1, { 1, 1, 1, 1 }, &gameMemory->persistantArena));
    // state->assets[(int)GameAsset::DefaultTexture].id = state->api.UploadTexture(renderGroup->renderer, state->assets[(int)GameAsset::DefaultTexture].texture, &gameMemory->persistantArena);

    Material brickMat = {};
    brickMat.color = { 1, 1, 1, 1 };
    brickMat.textureCount = 1;
    brickMat.textureId = 0;

    Material spidyMat = {};
    spidyMat.color = { 1, 1, 1, 1 };
    spidyMat.textureCount = 1;
    spidyMat.textureId = &state->assets[(int)GameAsset::SpidyTexture].id;

    renderGroup->lightDirection = Normalize(Vec3{ -1, 0, 0 });
    renderGroup->diffuse = { 1, 1, 1, 1 };

    // for (s32 x = 0; x < 3; ++x)
    // {
    //     for (s32 z = 0; z < 3; ++z)
    //     {
    //         state->chunks[x * 3 + z].x = x;
    //         state->chunks[x * 3 + z].z = z;
    //         state->chunks[x * 3 + z].modelId = state->api.UploadModel(renderGroup->renderer, LoadChunk(x - 1, z - 1, &gameMemory->transientArena), &gameMemory->persistantArena);
    //     }
    // }

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

    VoxelData voxel = {};
    voxel.color = { 1, 1, 0, 1 };
    PushRenderVoxel(renderGroup, voxel, MAT4_IDENTITY, &state->camera[(u32)state->gameMode]);

    // Material mat = {};
    // mat.textureCount = 1;
    // mat.textureId = 0;
    // mat.color = { 1, 1, 1, 1 };

    // for (u32 i = 0; i < ARRAY_COUNT(state->chunks); ++i)
    // {
    //     PushRenderModel(renderGroup, &state->camera[(u32)state->gameMode], state->chunks[i].modelId, mat, MAT4_IDENTITY, 0, 0, {}, 0, 0);
    // }
    // PushRenderModel(renderGroup, &state->camera[(u32)state->gameMode], state->assets[(u32)GameAsset::Cube].id, mat, Translate({ 0, 70, 0 }), 0, 0, {}, 0, 0);

    // PushRenderVoxel(renderGroup);

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