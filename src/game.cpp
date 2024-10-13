#include "game.h"
#include "hz_io.h"

HPI void Init(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory)
{
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


    state->testModel.vertices[0] = { 0.0f, 0.5f, 0.0f, 1.0f, 1.0f };
    state->testModel.vertices[1] = { 0.5f, -0.5f, 0.0f, 0.0f, 1.0f };
    state->testModel.vertices[2] = { -0.5f, -0.5f, 0.0f, 1.0f, 0.0f };

    state->testModel.indices[0] = 0;
    state->testModel.indices[1] = 1;
    state->testModel.indices[2] = 2;
    state->testModel.transform =
    { 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f };
}
HPI void Update(GameState* state, RenderGroup* renderGroup, GameMemory* gameMemory)
{
    PushRenderClear(renderGroup, { 0.5f, 0.5f, 0.5f, 1.0f });
    PushRenderModel(renderGroup, state->testModel, { 1.0f, 1.0f, 1.0f, 1.0f });
}