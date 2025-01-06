#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <timeapi.h>

#include <sys/stat.h> 
#include "hz_utils.h"
#include "hz_math.h"
#include "hz_render.h"
#include "game.h"

struct Win32Mesh
{
    bool isValid;
    Mat4 transform;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11InputLayout* inputLayout;
    ID3D11SamplerState* samplerState;
    UINT shaderResIndex;

    ID3D11Buffer* vsPerInstance;
    ID3D11Buffer* psPerInstance;

    UINT stride[1];
    UINT offset[1];
    UINT indexCount;
};

struct Win32Model
{
    bool isValid;
    u32 meshCount;
    Win32Mesh* meshes;
    u32 shaderResCount;
    ID3D11ShaderResourceView** shaderRes;
};

struct Win32D3D11
{
    IDXGISwapChain* swapChain;
    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11DepthStencilView* depthStencilView;


    //TODO: test code
#define MAX_MODEL_COUNT 256
    Win32Model models[MAX_MODEL_COUNT];

    ID3D11Buffer* vsPerFrame;
    ID3D11Buffer* psPerFrame;

    ID3D11Buffer* vsPerScene;
    ID3D11Buffer* psPerScene;

    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;

    void* defaultVertexShader;
    u32 defaultVertexShaderSize;
    void* defaultPixelShader;
    u32 defaultPixelShaderSize;
};

typedef decltype(Init) GameInit;
typedef decltype(Update) GameUpdate;

struct Win32GameCode
{
    HMODULE gameDll;
    GameInit* InitGame;
    GameUpdate* UpdateGame;
};

Win32GameCode Win32LoadGameCode(char* dll)
{
    Win32GameCode result = {};
    result.gameDll = LoadLibraryA(dll);
    result.InitGame = (GameInit*)GetProcAddress(result.gameDll, "Init");
    result.UpdateGame = (GameUpdate*)GetProcAddress(result.gameDll, "Update");
    return result;
}

void Win32ReloadGameCode(Win32GameCode* gameCode)
{
    static long long lastEditTimeTimeStampDll;

    struct stat st;
    if (stat("game_temp.dll", &st) == 0)
    {
        long long currentEditTimeStampDll = st.st_mtime;
        if (currentEditTimeStampDll > lastEditTimeTimeStampDll)
        {
            if (gameCode->gameDll)
            {
                FreeLibrary(gameCode->gameDll);
                gameCode->gameDll = NULL;
            }

            while (!CopyFile("game_temp.dll", "game.dll", FALSE))
            {
                Sleep(10);
            }
            DeleteFile("game_temp.dll");

            *gameCode = Win32LoadGameCode("game.dll");

            lastEditTimeTimeStampDll = currentEditTimeStampDll;
        }
    }
}

static void D3d11InitConstantBuffer(Win32D3D11* d3d11, void* data, UINT size, ID3D11Buffer** buffer)
{
    D3D11_BUFFER_DESC constBufferDesc = {};
    constBufferDesc.ByteWidth = size;
    constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constBufferDesc.MiscFlags = 0;
    constBufferDesc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA constResDesc = {};
    constResDesc.pSysMem = data;
    d3d11->device->CreateBuffer(&constBufferDesc, &constResDesc, buffer);
}

static Win32Mesh Win32LoadMesh(Win32D3D11* d3d11, LoadedMesh initialMesh)
{
    Win32Mesh result = {};
    result.indexCount = initialMesh.indexCount;
    result.transform = initialMesh.transform;
    result.shaderResIndex = initialMesh.matIndex;

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(*initialMesh.vertices) * initialMesh.vertexCount;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = sizeof(*initialMesh.vertices);
    D3D11_SUBRESOURCE_DATA vertexResDesc = {};
    vertexResDesc.pSysMem = initialMesh.vertices;
    d3d11->device->CreateBuffer(&vertexBufferDesc, &vertexResDesc, &result.vertexBuffer);


    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(*initialMesh.indices) * initialMesh.indexCount;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = sizeof(*initialMesh.indices);
    D3D11_SUBRESOURCE_DATA indexResDesc = {};
    indexResDesc.pSysMem = initialMesh.indices;
    d3d11->device->CreateBuffer(&indexBufferDesc, &indexResDesc, &result.indexBuffer);


    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    d3d11->device->CreateSamplerState(&samplerDesc, &result.samplerState);


    VSPerInstance vsPerInstance = {};
    D3d11InitConstantBuffer(d3d11, &vsPerInstance, sizeof(VSPerInstance), &result.vsPerInstance);

    PSPerInstance psPerInstance = {};
    D3d11InitConstantBuffer(d3d11, &psPerInstance, sizeof(PSPerInstance), &result.psPerInstance);


    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(Vec3), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(Vec3) + sizeof(Vec3), D3D11_INPUT_PER_VERTEX_DATA, 0},
        // {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(Vec3), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    d3d11->device->CreateInputLayout(layoutDesc, ARRAY_COUNT(layoutDesc), d3d11->defaultVertexShader, d3d11->defaultVertexShaderSize, &result.inputLayout);

    result.stride[0] = sizeof(*initialMesh.vertices);
    result.offset[0] = 0;
    return result;
}

static ModelInfo Win32LoadModel(Win32D3D11* d3d11, LoadedModel initialModel, MemoryArena* arena)
{
    ModelInfo result = {};

    for (int i = 0; i < MAX_MODEL_COUNT; ++i)
    {
        if (!d3d11->models[i].isValid)
        {
            result.id = i;
            break;
        }
        else if (i == MAX_MODEL_COUNT - 1)
        {
            result.id = -1;
        }
    }

    Win32Model* win32Model = &d3d11->models[result.id];
    win32Model->isValid = true;

    win32Model->meshCount = initialModel.meshCount;
    win32Model->meshes = PUSH_ARRAY(arena, Win32Mesh, initialModel.meshCount);

    win32Model->shaderResCount = initialModel.matCount;
    win32Model->shaderRes = PUSH_ARRAY(arena, ID3D11ShaderResourceView*, win32Model->shaderResCount);
    for (u32 i = 0; i < initialModel.matCount; ++i)
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = initialModel.mats[i].width;
        textureDesc.Height = initialModel.mats[i].height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = initialModel.mats[i].texel;
        initData.SysMemPitch = sizeof(*initialModel.mats[i].texel) * initialModel.mats[i].width;
        initData.SysMemSlicePitch = sizeof(*initialModel.mats[i].texel) * initialModel.mats[i].width * initialModel.mats[i].height;

        ID3D11Texture2D* tex = nullptr;
        d3d11->device->CreateTexture2D(&textureDesc, &initData, &tex);

        D3D11_SHADER_RESOURCE_VIEW_DESC resDesc;
        resDesc.Format = textureDesc.Format;
        resDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        resDesc.Texture2D.MostDetailedMip = 0;
        resDesc.Texture2D.MipLevels = 1;
        d3d11->device->CreateShaderResourceView(tex, &resDesc, &win32Model->shaderRes[i]);
    }

    for (u32 i = 0; i < initialModel.meshCount; ++i)
    {
        win32Model->meshes[i] = Win32LoadMesh(d3d11, initialModel.meshes[i]);
    }
    return result;
}

static void Win32InitScene(GameState* gameState, RenderGroup* renderGroup, Win32D3D11* d3d11, GameMemory* gameMemory, HWND windowHandle)
{

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc.Width = gameState->width;
    swapChainDesc.BufferDesc.Height = gameState->height;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.OutputWindow = windowHandle;
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = 0;

    HRESULT result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_DEBUG, 0, 0, D3D11_SDK_VERSION, &swapChainDesc, &d3d11->swapChain, &d3d11->device, 0, &d3d11->deviceContext);

    if (SUCCEEDED(result))
    {
        ID3D11Texture2D* frameBuffer;
        if (!SUCCEEDED(d3d11->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer)))
        {
            //TODO: Logging
            ASSERT(false);
        }

        if (!SUCCEEDED(d3d11->device->CreateRenderTargetView(frameBuffer, 0, &d3d11->renderTargetView)))
        {
            //TODO: Logging
            ASSERT(false);
        }

        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        // depthStencilDesc.StencilEnable = false;
        // depthStencilDesc.StencilReadMask = 0xFF;
        // depthStencilDesc.StencilWriteMask = 0xFF;
        // depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        // depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        // depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        // depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        // depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        // depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        // depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        // depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        ID3D11DepthStencilState* depthStencilState;
        d3d11->device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);

        ID3D11RasterizerState* rasterizerState;
        {
            D3D11_RASTERIZER_DESC rasterizerDesc = {};
            rasterizerDesc.FillMode = D3D11_FILL_SOLID;
            rasterizerDesc.CullMode = D3D11_CULL_BACK;
            rasterizerDesc.FrontCounterClockwise = true;
            rasterizerDesc.DepthBiasClamp = 0;
            rasterizerDesc.DepthClipEnable = true;
            rasterizerDesc.AntialiasedLineEnable = false;
            rasterizerDesc.MultisampleEnable = false;
            rasterizerDesc.ScissorEnable = false;
            rasterizerDesc.SlopeScaledDepthBias = 0.0f;
            rasterizerDesc.DepthBias = 0;

            d3d11->device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
        }

        d3d11->deviceContext->RSSetState(rasterizerState);
        d3d11->deviceContext->OMSetDepthStencilState(depthStencilState, 0);

        D3D11_TEXTURE2D_DESC depthTextureDesc = {};
        depthTextureDesc.Width = gameState->width;
        depthTextureDesc.Height = gameState->height;
        depthTextureDesc.MipLevels = 1;
        depthTextureDesc.ArraySize = 1;
        depthTextureDesc.SampleDesc.Count = 1;
        depthTextureDesc.SampleDesc.Quality = 0;
        depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        ID3D11Texture2D* depthStencilTexture;
        result = d3d11->device->CreateTexture2D(&depthTextureDesc, NULL, &depthStencilTexture);

        if (result != S_OK)
        {
            ASSERT(false);
            //TODO: Logging
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = depthTextureDesc.Format;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        result = d3d11->device->CreateDepthStencilView(depthStencilTexture, NULL, &d3d11->depthStencilView);
        depthStencilTexture->Release();

        if (result != S_OK)
        {
            ASSERT(false);
            //TODO: Logging
        }

        d3d11->deviceContext->OMSetRenderTargets(1, &d3d11->renderTargetView, d3d11->depthStencilView);

        D3D11_VIEWPORT viewPort = {};
        viewPort.TopLeftX = 0;
        viewPort.TopLeftY = 0;
        viewPort.Width = (FLOAT)gameState->width;
        viewPort.Height = (FLOAT)gameState->height;
        viewPort.MinDepth = 0;
        viewPort.MaxDepth = 1;

        VSPerFrame vsPerFrame = {};
        D3d11InitConstantBuffer(d3d11, &vsPerFrame, sizeof(VSPerFrame), &d3d11->vsPerFrame);

        PSPerFrame psPerFrame = {};
        D3d11InitConstantBuffer(d3d11, &psPerFrame, sizeof(PSPerFrame), &d3d11->psPerFrame);

        VSPerScene vsPerScene = {};
        D3d11InitConstantBuffer(d3d11, &vsPerScene, sizeof(VSPerScene), &d3d11->vsPerScene);

        PSPerScene psPerScene = {};
        D3d11InitConstantBuffer(d3d11, &psPerScene, sizeof(PSPerScene), &d3d11->psPerScene);

        File vsShaderFile = ReadFile("default_vs.fxo", &gameMemory->persistantArena);
        File psShaderFile = ReadFile("default_ps.fxo", &gameMemory->persistantArena);
        d3d11->defaultVertexShader = vsShaderFile.content;
        d3d11->defaultVertexShaderSize = vsShaderFile.contentSize;
        d3d11->defaultPixelShader = psShaderFile.content;
        d3d11->defaultPixelShaderSize = psShaderFile.contentSize;

        d3d11->device->CreateVertexShader(d3d11->defaultVertexShader, d3d11->defaultVertexShaderSize, 0, &d3d11->vertexShader);
        d3d11->device->CreatePixelShader(d3d11->defaultPixelShader, d3d11->defaultPixelShaderSize, 0, &d3d11->pixelShader);

        d3d11->deviceContext->VSSetConstantBuffers(2, 1, &d3d11->vsPerScene);
        d3d11->deviceContext->PSSetConstantBuffers(2, 1, &d3d11->psPerScene);

        d3d11->deviceContext->RSSetViewports(1, &viewPort);

        //TODO: refactor to game code
        gameState->cubeModel = Win32LoadModel(d3d11, LoadAsset("asset\\cube.hza", &gameMemory->transientArena).loadedModel, &gameMemory->persistantArena);
        gameState->sphereModel = Win32LoadModel(d3d11, LoadAsset("asset\\sphere.hza", &gameMemory->transientArena).loadedModel, &gameMemory->persistantArena);
        gameState->spidyModel = Win32LoadModel(d3d11, LoadAsset("asset\\ghost-spider.hza", &gameMemory->transientArena).loadedModel, &gameMemory->persistantArena);
    }
}

static void Win32RenderOutput(RenderGroup* renderGroup, Win32D3D11 d3d11)
{
    {
        D3D11_MAPPED_SUBRESOURCE subRes;

        d3d11.deviceContext->Map(d3d11.vsPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);

        VSPerFrame* vsPerFrame = (VSPerFrame*)subRes.pData;
        d3d11.deviceContext->Unmap(d3d11.vsPerFrame, 0);
        d3d11.deviceContext->VSSetConstantBuffers(1, 1, &d3d11.vsPerFrame);
    }

    {
        D3D11_MAPPED_SUBRESOURCE subRes;
        d3d11.deviceContext->Map(d3d11.psPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);

        PSPerFrame* psPerFrame = (PSPerFrame*)subRes.pData;
        psPerFrame->lightDirection = Normalize(Vec3{ -1, -1, 1 });
        d3d11.deviceContext->Unmap(d3d11.psPerFrame, 0);
        d3d11.deviceContext->PSSetConstantBuffers(1, 1, &d3d11.psPerFrame);
    }

    for (void* base = renderGroup->pushBuffer.base; base < (u8*)renderGroup->pushBuffer.base + renderGroup->pushBuffer.used;)
    {
        RenderCommandHeader* header = (RenderCommandHeader*)base;
        base = (u8*)base + sizeof(*header);
        switch (header->type)
        {
            case RC_RenderCommandClear:
            {
                RenderCommandClear* entry = (RenderCommandClear*)base;

                d3d11.deviceContext->ClearRenderTargetView(d3d11.renderTargetView, entry->color.e);
                d3d11.deviceContext->ClearDepthStencilView(d3d11.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

                base = (u8*)base + sizeof(*entry);
            } break;

            case RC_RenderCommandModel:
            {
                RenderCommandModel* entry = (RenderCommandModel*)base;

                Win32Model model = d3d11.models[entry->model.id];
                for (u32 i = 0; i < model.meshCount; ++i)
                {
                    {
                        D3D11_MAPPED_SUBRESOURCE subRes;

                        d3d11.deviceContext->Map(model.meshes[i].vsPerInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);

                        VSPerInstance* vsPerInstance = (VSPerInstance*)subRes.pData;
                        Mat4 worldTransform = model.meshes[i].transform * entry->transform;
                        vsPerInstance->mvp = GetPerspectiveProjection(entry->camera->fovy, entry->camera->aspect, 0.1f, 100.0f) * GetViewMatrix(entry->camera->position, entry->camera->direction, entry->camera->worldUp) * worldTransform;
                        d3d11.deviceContext->Unmap(model.meshes[i].vsPerInstance, 0);
                    }

                    {
                        D3D11_MAPPED_SUBRESOURCE subRes;
                        d3d11.deviceContext->Map(model.meshes[i].psPerInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);

                        PSPerInstance* psPerInstance = (PSPerInstance*)subRes.pData;
                        psPerInstance->color = entry->color;
                        psPerInstance->diffuse = { 1, 1, 1, 1 };
                        psPerInstance->ambient = { 0.1, 0.1, 0.1, 0.1 };

                        d3d11.deviceContext->Unmap(model.meshes[i].psPerInstance, 0);
                    }

                    d3d11.deviceContext->VSSetShader(d3d11.vertexShader, 0, 0);
                    d3d11.deviceContext->PSSetShader(d3d11.pixelShader, 0, 0);
                    d3d11.deviceContext->VSSetConstantBuffers(0, 1, &model.meshes[i].vsPerInstance);
                    d3d11.deviceContext->PSSetConstantBuffers(0, 1, &model.meshes[i].psPerInstance);
                    d3d11.deviceContext->IASetVertexBuffers(0, 1, &model.meshes[i].vertexBuffer, model.meshes[i].stride, model.meshes[i].offset);
                    d3d11.deviceContext->IASetIndexBuffer(model.meshes[i].indexBuffer, DXGI_FORMAT_R32_UINT, 0);
                    d3d11.deviceContext->IASetInputLayout(model.meshes[i].inputLayout);
                    d3d11.deviceContext->PSSetShaderResources(0, 1, &model.shaderRes[model.meshes[i].shaderResIndex]);
                    d3d11.deviceContext->PSSetSamplers(0, 1, &model.meshes[i].samplerState);
                    d3d11.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    d3d11.deviceContext->OMSetRenderTargets(1, &d3d11.renderTargetView, d3d11.depthStencilView);

                    d3d11.deviceContext->DrawIndexed(model.meshes[i].indexCount, 0, 0);

                }
                base = (u8*)base + sizeof(*entry);
            } break;

            default:
                break;
        }
    }
    d3d11.swapChain->Present(0, 0);
}

inline static u64 Win32GetPerfCounter()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result.QuadPart;
}

inline static u64 Win32GetPerfFrequency()
{
    LARGE_INTEGER result;
    QueryPerformanceFrequency(&result);
    return result.QuadPart;
}

LRESULT Win32WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        }
        break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            OutputDebugStringA("WM_DESROY\n");
        }
        break;

        case WM_CLOSE:
        {
            PostQuitMessage(0);
            OutputDebugStringA("WM_CLOSE\n");
        }
        break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }
        break;

        default:
        {
            result = DefWindowProc(windowHandle, message, wParam, lParam);
        }
        break;
    }
    return result;
}

static void Win32ProcessKeyboardInput(ButtonState* button, bool isDown)
{
    ASSERT(button->isDown != isDown);
    button->isDown = isDown;
    button->halfTransitionCount++;
}

static void Win32ProcessPendingMessage(GameState* gameState)
{
    gameState->input.dMouse = {};
    for (int i = 0; i < MAX_BUTTON; ++i)
    {
        gameState->input.buttons[i].halfTransitionCount = 0;
    }

    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        switch (msg.message)
        {
            case WM_QUIT:
            {
                gameState->running = false;
            }
            break;
            case WM_INPUT:
            {
                UINT dwSize;

                GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
                LPBYTE lpb = new BYTE[dwSize];

                if (GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
                    OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

                RAWINPUT* raw = (RAWINPUT*)lpb;
                if (raw->header.dwType == RIM_TYPEMOUSE)
                {
                    gameState->input.dMouse.x += raw->data.mouse.lLastX;
                    gameState->input.dMouse.y += raw->data.mouse.lLastY;
                }
                delete[] lpb;
            } break;
            case WM_MOUSEMOVE:
            {
                gameState->input.mouse.x = GET_X_LPARAM(msg.lParam);
                gameState->input.mouse.y = GET_Y_LPARAM(msg.lParam);

                gameState->input.mouseButtonState[0] = (msg.wParam & MK_LBUTTON) != 0;
                gameState->input.mouseButtonState[1] = (msg.wParam & MK_RBUTTON) != 0;
                gameState->input.mouseButtonState[2] = (msg.wParam & MK_MBUTTON) != 0;
                gameState->input.mouseButtonState[3] = (msg.wParam & MK_XBUTTON1) != 0;
                gameState->input.mouseButtonState[4] = (msg.wParam & MK_XBUTTON2) != 0;
            }
            case WM_MOUSEWHEEL:
            {
                gameState->input.dMouse.z = GET_WHEEL_DELTA_WPARAM(msg.wParam);
            } break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                bool wasDown = ((msg.lParam & (1 << 30)) != 0);
                bool isDown = ((msg.lParam & (1 << 31)) == 0);

                if (isDown != wasDown)
                {
                    if (msg.wParam == 'W')
                    {
                        Win32ProcessKeyboardInput(&gameState->input.up, isDown);
                    }
                    if (msg.wParam == 'S')
                    {
                        Win32ProcessKeyboardInput(&gameState->input.down, isDown);
                    }
                    if (msg.wParam == 'A')
                    {
                        Win32ProcessKeyboardInput(&gameState->input.left, isDown);
                    }
                    if (msg.wParam == 'D')
                    {
                        Win32ProcessKeyboardInput(&gameState->input.right, isDown);
                    }
                    if (msg.wParam == VK_ESCAPE)
                    {
                        Win32ProcessKeyboardInput(&gameState->input.escape, isDown);
                    }
                    if (msg.wParam == VK_SPACE)
                    {
                        Win32ProcessKeyboardInput(&gameState->input.space, isDown);
                    }
                    if (msg.wParam == VK_F1)
                    {
                        Win32ProcessKeyboardInput(&gameState->input.f1, isDown);
                    }
                    if (msg.wParam == VK_F3)
                    {
                        Win32ProcessKeyboardInput(&gameState->input.f3, isDown);
                    }
                    if (isDown)
                    {
                        bool32 altKey = (msg.lParam & (1 << 29));
                        if (altKey && msg.wParam == VK_F4)
                        {
                            PostQuitMessage(0);
                        }
                    }
                }
            }
            break;
            default:
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            break;
        }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmdLine, int cmdShow)
{
    GameMemory gameMemory;
    size_t persistantArenaSize = MEGABYTES(64);
    size_t transientArenaSize = MEGABYTES(64);
    size_t pushBufferSize = MEGABYTES(4);
    void* memory = VirtualAlloc(0, persistantArenaSize + transientArenaSize + pushBufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    InitMemoryArena(&gameMemory.persistantArena, persistantArenaSize, memory);
    InitMemoryArena(&gameMemory.transientArena, transientArenaSize, (u8*)memory + persistantArenaSize);
    GameState* gameState = PUSH_TYPE(&gameMemory.persistantArena, GameState);
    RenderGroup* renderGroup = PUSH_TYPE(&gameMemory.persistantArena, RenderGroup);
    InitMemoryArena(&renderGroup->pushBuffer, pushBufferSize, (u8*)memory + persistantArenaSize + transientArenaSize);

    Win32GameCode gameCode = Win32LoadGameCode("game.dll");
    gameCode.InitGame(gameState, renderGroup, &gameMemory);

    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = Win32WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    //  windowClass.hIcon;
    windowClass.lpszClassName = gameState->tittle;

    u64 perfFrequency = Win32GetPerfFrequency();

    if (RegisterClassEx(&windowClass))
    {
        HWND windowHandle = CreateWindowEx(0, windowClass.lpszClassName, gameState->tittle, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, gameState->width, gameState->height, 0, 0, instance, 0);

        Win32D3D11 d3d11 = {};
        Win32InitScene(gameState, renderGroup, &d3d11, &gameMemory, windowHandle);

        if (windowHandle)
        {
            RAWINPUTDEVICE rawInputDevices[1];
            rawInputDevices[0].usUsagePage = 0x01;  // Mouse
            rawInputDevices[0].usUsage = 0x02;      // Mouse
            rawInputDevices[0].dwFlags = 0;
            rawInputDevices[0].hwndTarget = windowHandle;

            RegisterRawInputDevices(rawInputDevices, 1, sizeof(RAWINPUTDEVICE));

            bool timeIsGranular = timeBeginPeriod(1) == TIMERR_NOERROR;
            u64 lastPerfCounter = Win32GetPerfCounter();
            while (gameState->running)
            {
                Win32ReloadGameCode(&gameCode);
                Win32ProcessPendingMessage(gameState);

                ResetMemoryArena(&gameMemory.transientArena);
                ResetMemoryArena(&renderGroup->pushBuffer);
                gameCode.UpdateGame(gameState, renderGroup, &gameMemory);

                RECT gameRect;
                GetWindowRect(windowHandle, &gameRect);
                if (gameState->lockCursor) SetCursorPos((gameRect.left + gameRect.right) / 2, (gameRect.top + gameRect.bottom) / 2);
                ShowCursor(gameState->showCursor);

                Win32RenderOutput(renderGroup, d3d11);

                f32 elapsed = (f32)(Win32GetPerfCounter() - lastPerfCounter) / perfFrequency;

                if (elapsed < gameState->dt)
                {
                    if (timeIsGranular)
                    {
                        Sleep((DWORD)((gameState->dt - elapsed) * 1000));
                    }
                    while (elapsed < gameState->dt)
                    {
                        elapsed = (f32)(Win32GetPerfCounter() - lastPerfCounter) / perfFrequency;
                    }
                }
                else
                {
                    //TODO: Missed framerate!
                    LOGWARNING("Missed");
                }
                LOGINFO("%.2fms\n", elapsed * 1000);
                lastPerfCounter = Win32GetPerfCounter();
            }

            CloseWindow(windowHandle);
        }
        else
        {
            //TODO: Logging!
        }
    }
    else
    {
        //TODO: Logging!
    }

    return 0;
}
