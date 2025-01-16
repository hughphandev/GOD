#include "game.h"
#include "hz_io.h"

HPI void Init(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory)
{
    state->running = true;
    state->tittle = "GOD";
    // state->width = 720;
    // state->height = 1280;
    state->width = 1280;
    state->height = 720;

    state->camera.position = { 0, 0, -5 };
    state->camera.direction = { 0, 0, 1 };
    state->camera.worldUp = { 0, 1, 0 };
    state->camera.fovy = 60.0f * DEG2RAD;
    state->camera.aspect = (float)state->width / state->height;

    state->gameMode = GameMode::InGame;

    state->pitch = 0;
    state->yaw = 90;

    state->dt = 1 / 60.0f;

    state->assets[(int)GameAsset::Cube] = LoadAsset("asset\\cube.hza", &gameMemory->persistantArena);
    state->assets[(int)GameAsset::Sphere] = LoadAsset("asset\\sphere.hza", &gameMemory->persistantArena);
    state->assets[(int)GameAsset::Spindy] = LoadAsset("asset\\ghost-spider-glb.hza", &gameMemory->persistantArena);
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

            float sensitivity = 0.1f;
            state->yaw -= state->input.dMouse.x * sensitivity;
            state->pitch -= state->input.dMouse.y * sensitivity;
            if (state->pitch > 89.0f)
                state->pitch = 89.0f;
            if (state->pitch < -89.0f)
                state->pitch = -89.0f;
            Vec3 direction;
            direction.x = Cos(state->yaw * DEG2RAD) * Cos(state->pitch * DEG2RAD);
            direction.y = Sin(state->pitch * DEG2RAD);
            direction.z = Sin(state->yaw * DEG2RAD) * Cos(state->pitch * DEG2RAD);
            state->camera.direction = Normalize(direction);
            float speed = 10;

            if (state->input.up.isDown)
            {
                state->camera.position += state->dt * state->camera.direction * speed;
            }
            if (state->input.down.isDown)
            {
                state->camera.position -= state->dt * state->camera.direction * speed;
            }
            if (state->input.left.isDown)
            {
                state->camera.position -= state->dt * Normalize(Cross(state->camera.worldUp, state->camera.direction)) * speed;
            }
            if (state->input.right.isDown)
            {
                state->camera.position += state->dt * Normalize(Cross(state->camera.worldUp, state->camera.direction)) * speed;
            }
        }break;

        case GameMode::Editor:
        {
            state->lockCursor = false;
            state->showCursor = true;
            //TODO: edit
        }break;
        INVALID_DEFAULT_CASE
    }

    PushRenderClear(renderGroup, { 0.5f, 0.5f, 0.5f, 1.0f });
    // for (int i = 0; i < 10; ++i)
    // {
    //     for (int j = 0; j < 10; j++)
    //     {
    //         PushRenderModel(renderGroup, &state->camera, state->cubeModel, { 1.0f, 1.0f, 1.0f, 1.0f }, TRS({ (float)i, 0, (float)j }, {}, { 1, 1, 1 }));
    //     }
    // }
    Asset spindy = state->assets[(int)GameAsset::Spindy];
    float div = state->t / spindy.animations[0].duration;
    NodeTransform* trans = ReadNodeTransform(spindy.animations[0], div - Floor(div), &gameMemory->transientArena);
    PushRenderModel(renderGroup, &state->camera, spindy.id, { 1.0f, 1.0f, 1.0f, 1.0f }, TRS({ (float)0, 0, (float)0 }, {}, { 1, 1, 1 }), spindy.loadedModel.boneCount, spindy.loadedModel.bones, spindy.loadedModel.globalInverseTransform, trans, spindy.animations[0].channelCount);
}