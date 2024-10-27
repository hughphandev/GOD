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


    File vsShaderFile = ReadFile("default_vertex.fxo", &gameMemory->persistantArena);
    File psShaderFile = ReadFile("default_pixel.fxo", &gameMemory->persistantArena);
    renderGroup->defaultVertexShader = vsShaderFile.content;
    renderGroup->defaultVertexShaderSize = vsShaderFile.contentSize;
    renderGroup->defaultPixelShader = psShaderFile.content;
    renderGroup->defaultPixelShaderSize = psShaderFile.contentSize;

    state->testModel.vertexCount = 3;
    state->testModel.vertices = PUSH_ARRAY(&gameMemory->persistantArena, Vert, state->testModel.vertexCount);
    state->testModel.indexCount = 3;
    state->testModel.indices = PUSH_ARRAY(&gameMemory->persistantArena, u32, state->testModel.indexCount);

    state->testModel = LoadAsset("asset\\obj.hza", &gameMemory->persistantArena).model;

    state->testModel.transform =
    { 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f };

    state->testModel.texture.width = 1;
    state->testModel.texture.height = 1;
    state->testModel.texture.texel = PUSH_ARRAY(&gameMemory->persistantArena, u32, state->testModel.texture.width * state->testModel.texture.height);
    *state->testModel.texture.texel = ToU32Color({ 1.0f, 0.0f, 0.0f, 1.0f });

    state->camera.position = { 0, 0, -2 };
    state->camera.direction = { 0, 0, 1 };
    state->camera.worldUp = { 0, 1, 0 };
    state->camera.fovy = 60.0f * DEG2RAD;
    state->camera.aspect = (float)state->width / state->height;

    state->gameMode = GameMode::InGame;

    state->pitch = 0;
    state->yaw = 90;
}

HPI void Update(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory)
{
    if (state->input.f1.isDown && state->input.f1.halfTransitionCount > 0)
    {
        state->gameMode = (GameMode)(((int)state->gameMode + 1) % (int)GameMode::TERMINATOR);
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
            char debug[256];
            wsprintf(debug, "%d, %d\n", state->input.mouse.x, state->input.mouse.y);
            OutputDebugStringA(debug);
            if (state->pitch > 89.0f)
                state->pitch = 89.0f;
            if (state->pitch < -89.0f)
                state->pitch = -89.0f;
            Vec3 direction;
            direction.x = Cos(state->yaw * DEG2RAD) * Cos(state->pitch * DEG2RAD);
            direction.y = Sin(state->pitch * DEG2RAD);
            direction.z = Sin(state->yaw * DEG2RAD) * Cos(state->pitch * DEG2RAD);
            state->camera.direction = Normalize(direction);

            if (state->input.up.isDown)
            {
                state->camera.position += 0.01f * state->camera.direction;
            }
            if (state->input.down.isDown)
            {
                state->camera.position -= 0.01f * state->camera.direction;
            }
            if (state->input.left.isDown)
            {
                state->camera.position -= 0.01f * Normalize(Cross(state->camera.worldUp, state->camera.direction));
            }
            if (state->input.right.isDown)
            {
                state->camera.position += 0.01f * Normalize(Cross(state->camera.worldUp, state->camera.direction));
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
    PushRenderModel(renderGroup, &state->camera, &state->testModel, { 1.0f, 1.0f, 1.0f, 1.0f });
}