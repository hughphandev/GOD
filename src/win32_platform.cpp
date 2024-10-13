#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <sys/stat.h> 
#include "hz_math.h"
#include "hz_utils.h"
#include "hz_render.h"
#include "game.h"
static bool running = true;

struct Win32D3D11
{
    IDXGISwapChain* swapChain;
    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    ID3D11RenderTargetView* renderTargetView;
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

static void Win32RenderOutput(RenderGroup* renderGroup, Win32D3D11 d3d11)
{
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
                d3d11.deviceContext->OMSetRenderTargets(1, &d3d11.renderTargetView, 0);

                base = (u8*)base + sizeof(*entry);
            } break;

            case RC_RenderCommandModel:
            {
                RenderCommandModel* entry = (RenderCommandModel*)base;

                static bool first = false;
                if (!first)
                {
                    first = true;
                    D3D11_BUFFER_DESC vertexBufferDesc = {};
                    vertexBufferDesc.ByteWidth = sizeof(*entry->model.vertices) * entry->model.vertexCount;
                    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
                    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                    vertexBufferDesc.CPUAccessFlags = 0;
                    vertexBufferDesc.MiscFlags = 0;
                    vertexBufferDesc.StructureByteStride = sizeof(Vert);
                    ID3D11Buffer* vertexBuffer;
                    D3D11_SUBRESOURCE_DATA vertexResDesc = {};
                    vertexResDesc.pSysMem = entry->model.vertices;
                    d3d11.device->CreateBuffer(&vertexBufferDesc, &vertexResDesc, &vertexBuffer);


                    D3D11_BUFFER_DESC indexBufferDesc = {};
                    indexBufferDesc.ByteWidth = sizeof(*entry->model.indices) * entry->model.indexCount;
                    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
                    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                    indexBufferDesc.CPUAccessFlags = 0;
                    indexBufferDesc.MiscFlags = 0;
                    indexBufferDesc.StructureByteStride = sizeof(*entry->model.indices);
                    ID3D11Buffer* indexBuffer;
                    D3D11_SUBRESOURCE_DATA indexResDesc = {};
                    indexResDesc.pSysMem = entry->model.indices;
                    d3d11.device->CreateBuffer(&indexBufferDesc, &indexResDesc, &indexBuffer);

                    struct ConstantBuffer
                    {
                        Mat4 transform;
                        Color color;
                    };

                    ConstantBuffer cbuffer;
                    cbuffer.transform = entry->model.transform;
                    cbuffer.color = entry->color;

                    D3D11_BUFFER_DESC constBufferDesc = {};
                    constBufferDesc.ByteWidth = sizeof(cbuffer);
                    constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
                    constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                    constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                    constBufferDesc.MiscFlags = 0;
                    constBufferDesc.StructureByteStride = 0;
                    D3D11_SUBRESOURCE_DATA constResDesc = {};
                    constResDesc.pSysMem = &cbuffer;
                    ID3D11Buffer* constantBuffer;
                    d3d11.device->CreateBuffer(&constBufferDesc, &constResDesc, &constantBuffer);
                    d3d11.deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

                    D3D11_VIEWPORT viewPort = {};
                    viewPort.TopLeftX = 0;
                    viewPort.TopLeftY = 0;
                    viewPort.Width = 800;
                    viewPort.Height = 600;
                    viewPort.MinDepth = 0;
                    viewPort.MaxDepth = 1;

                    ID3D11VertexShader* vertexShader;
                    ID3D11PixelShader* pixelShader;
                    d3d11.device->CreateVertexShader(renderGroup->defaultVertexShader, renderGroup->defaultVertexShaderSize, 0, &vertexShader);
                    d3d11.device->CreatePixelShader(renderGroup->defaultPixelShader, renderGroup->defaultPixelShaderSize, 0, &pixelShader);


                    d3d11.deviceContext->VSSetShader(vertexShader, 0, 0);
                    d3d11.deviceContext->PSSetShader(pixelShader, 0, 0);

                    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
                    {
                        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(Vec3), D3D11_INPUT_PER_VERTEX_DATA, 0 }
                    };
                    ID3D11InputLayout* inputLayout = {};
                    d3d11.device->CreateInputLayout(layoutDesc, ARRAY_COUNT(layoutDesc), renderGroup->defaultVertexShader, renderGroup->defaultVertexShaderSize, &inputLayout);

                    UINT stride[] = { sizeof(*entry->model.vertices) };
                    UINT offset[] = { 0 };

                    d3d11.deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);
                    d3d11.deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
                    d3d11.deviceContext->IASetInputLayout(inputLayout);
                    d3d11.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    d3d11.deviceContext->RSSetViewports(1, &viewPort);
                }

                d3d11.deviceContext->OMSetRenderTargets(1, &d3d11.renderTargetView, 0);
                d3d11.deviceContext->DrawIndexed(entry->model.indexCount, 0, 0);

                base = (u8*)base + sizeof(*entry);
            } break;

            default:
                break;
        }
    }
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
            running = false;
            OutputDebugStringA("WM_DESROY\n");
        }
        break;

        case WM_CLOSE:
        {
            running = false;
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

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmdLine, int cmdShow)
{
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = Win32WindowProc;
    windowClass.hInstance = instance;
    //  windowClass.hIcon;
    windowClass.lpszClassName = "GodClass";


    if (RegisterClassEx(&windowClass))
    {
        HWND windowHandle = CreateWindowEx(0, windowClass.lpszClassName, "GOD", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

        Win32D3D11 d3d11 = {};

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferDesc.Width = 0;
        swapChainDesc.BufferDesc.Height = 0;
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

        HRESULT result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_DEBUG, 0, 0, D3D11_SDK_VERSION, &swapChainDesc, &d3d11.swapChain, &d3d11.device, 0, &d3d11.deviceContext);

        if (windowHandle && SUCCEEDED(result))
        {
            ID3D11Texture2D* frameBuffer;
            if (!SUCCEEDED(d3d11.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer)))
            {
                //TODO: Logging
                ASSERT(false);
            }

            if (!SUCCEEDED(d3d11.device->CreateRenderTargetView(frameBuffer, 0, &d3d11.renderTargetView)))
            {
                //TODO: Logging
                ASSERT(false);
            }


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

            MSG msg;
            while (running)
            {
                Win32ReloadGameCode(&gameCode);
                while (PeekMessage(&msg, windowHandle, 0, 0, PM_REMOVE))
                    switch (msg.message)
                    {
                        case WM_KEYDOWN:
                        {
                            OutputDebugStringA("WM_KEYDOWN\n");
                        }
                        break;
                        case WM_KEYUP:
                        {
                            OutputDebugStringA("WM_KEYUP\n");
                        }
                        break;
                        case WM_SYSKEYDOWN:
                        {
                            OutputDebugStringA("WM_SYSKEYDOWN\n");
                        }
                        break;
                        case WM_SYSKEYUP:
                        {
                            OutputDebugStringA("WM_SYSKEYUP\n");
                        }
                        break;

                        default:
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                        break;
                    }

                ResetMemoryArena(&gameMemory.transientArena);
                ResetMemoryArena(&renderGroup->pushBuffer);
                gameCode.UpdateGame(gameState, renderGroup, &gameMemory);

                Win32RenderOutput(renderGroup, d3d11);
                d3d11.swapChain->Present(0, 0);
            }
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
