#include "game.h"
#include "hz_io.h"

Entity* CreateEntity(Vec3 pos, Quaternion rot, Vec3 scale, GameAsset gfx, GameState* state)
{
    for (u32 i = 0; i < ARRAY_COUNT(state->entities); ++i)
    {
        if (!state->entities[i].isEnabled)
        {
            state->entities[i].isEnabled = true;
            state->entities[i].transform.position = pos;
            state->entities[i].transform.rotation = rot;
            state->entities[i].transform.scale = scale;
            state->entities[i].gfx = gfx;
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
    state->tittle = "GOD";
    state->width = 1280;
    state->height = 720;

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

    state->assets[(int)GameAsset::Cube] = LoadAsset("asset\\cube.hza", &gameMemory->persistantArena);
    state->assets[(int)GameAsset::Sphere] = LoadAsset("asset\\sphere.hza", &gameMemory->persistantArena);
    state->assets[(int)GameAsset::CubeRig] = LoadAsset("asset\\cube-rig.hza", &gameMemory->persistantArena);

    state->player = CreateEntity({}, {}, { 1, 1, 1 }, GameAsset::CubeRig, state);
    state->player->animIndex = 0;

    for (int i = -10; i < 10; ++i)
    {
        for (int j = -10; j < 10; j++)
        {
            CreateEntity({ (f32)i, -0.5f, (f32)j }, {}, { 1, 1, 1 }, GameAsset::Cube, state);
        }
    }
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

            f32 speed = 1;
            if (state->input.up.isDown)
            {
                state->player->transform.position.z += state->dt * speed;
            }
            if (state->input.down.isDown)
            {
                state->player->transform.position.z -= state->dt * speed;
            }
            if (state->input.left.isDown)
            {
                state->player->transform.position.x -= state->dt * speed;
            }
            if (state->input.right.isDown)
            {
                state->player->transform.position.x += state->dt * speed;
            }

            f32 div = state->t / state->assets[(int)state->player->gfx].animations[state->player->animIndex].duration;
            state->player->normalizedTime = div - Floor(div);

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

    PushRenderClear(renderGroup, { 0.5f, 0.5f, 0.5f, 1.0f });

    for (int i = 0; i < ARRAY_COUNT(state->entities); ++i)
    {
        Entity entity = state->entities[i];
        if (entity.isEnabled)
        {
            Asset asset = state->assets[(int)entity.gfx];
            if (entity.animIndex == INVALID_VALUE)
            {
                PushRenderModel(renderGroup, &state->camera[(int)state->gameMode], asset.id, { 1.0f, 1.0f, 1.0f, 1.0f }, TRS(entity.transform.position, entity.transform.rotation, entity.transform.scale), 0, NULL, {}, 0, NULL);
            }
            else
            {
                NodeTransform* trans = ReadNodeTransform(asset.animations[entity.animIndex], entity.normalizedTime, &gameMemory->transientArena);
                PushRenderModel(renderGroup, &state->camera[(int)state->gameMode], asset.id, { 1.0f, 1.0f, 1.0f, 1.0f }, Translate(state->player->transform.position), asset.loadedModel.boneCount, asset.loadedModel.bones, asset.loadedModel.globalInverseTransform, asset.animations[entity.animIndex].channelCount, trans);
            }

        }
    }
}