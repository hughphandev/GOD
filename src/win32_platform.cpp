#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

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
                d3d11.swapChain->Present(0, 0);

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


            // Vert vertices[] = {
            //     {0.0f, 0.5f, 0.0f, 1.0f, 1.0f},
            //     {0.5f, -0.5f, 0.0f, 0.0f, 1.0f},
            //     {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f},
            // };

            // D3D11_BUFFER_DESC vbd = {};
            // vbd.ByteWidth = sizeof(vertices);
            // vbd.Usage = D3D11_USAGE_DEFAULT;
            // vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            // vbd.CPUAccessFlags = 0;
            // vbd.MiscFlags = 0;
            // vbd.StructureByteStride = sizeof(Vert);
            // ID3D11Buffer* vertexBuffer;
            // D3D11_SUBRESOURCE_DATA vrd = {};
            // vrd.pSysMem = vertices;
            // d3d11.device->CreateBuffer(&vbd, &vrd, &vertexBuffer);

            // u16 indicies[] =
            // {
            //     0, 1, 2,
            // };

            // D3D11_BUFFER_DESC ibd = {};
            // ibd.ByteWidth = sizeof(indicies);
            // ibd.Usage = D3D11_USAGE_DEFAULT;
            // ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            // ibd.CPUAccessFlags = 0;
            // ibd.MiscFlags = 0;
            // ibd.StructureByteStride = sizeof(u16);
            // ID3D11Buffer* indexBuffer;
            // D3D11_SUBRESOURCE_DATA ird = {};
            // ird.pSysMem = indicies;
            // d3d11.device->CreateBuffer(&ibd, &ird, &indexBuffer);
            // d3d11.deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

            // struct ConstantBuffer
            // {
            //     Mat4 transform;
            // };

            // ConstantBuffer cbuffer =
            // {
            //     {
            //         1.0f, 0.0f, 0.0f, 0.0f,
            //         0.0f, 1.0f, 0.0f, 0.0f,
            //         0.0f, 0.0f, 1.0f, 0.0f,
            //         0.0f, 0.0f, 0.0f, 1.0f,
            //     },
            // };
            // D3D11_BUFFER_DESC tbd = {};
            // tbd.ByteWidth = sizeof(cbuffer);
            // tbd.Usage = D3D11_USAGE_DYNAMIC;
            // tbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            // tbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            // tbd.MiscFlags = 0;
            // tbd.StructureByteStride = 0;
            // D3D11_SUBRESOURCE_DATA trd = {};
            // trd.pSysMem = &cbuffer;
            // ID3D11Buffer* constantBuffer;
            // d3d11.device->CreateBuffer(&tbd, &trd, &constantBuffer);
            // d3d11.deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

            D3D11_VIEWPORT viewPort;
            viewPort.TopLeftX = 0;
            viewPort.TopLeftY = 0;
            viewPort.Width = 800;
            viewPort.Height = 600;
            viewPort.MinDepth = 0;
            viewPort.MaxDepth = 1;


            //             UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
            // #if defined( DEBUG ) || defined( _DEBUG )
            //             flags |= D3DCOMPILE_DEBUG; // add more debug output
            // #endif
            //             ID3DBlob* vertexShaderBlob = NULL, * pixelShaderBlob = NULL, * error_blob = NULL;

            //             // COMPILE VERTEX SHADER
            //             HRESULT hr = D3DCompileFromFile(
            //                 L"shaders.hlsl",
            //                 nullptr,
            //                 D3D_COMPILE_STANDARD_FILE_INCLUDE,
            //                 "vs_main",
            //                 "vs_5_0",
            //                 flags,
            //                 0,
            //                 &vertexShaderBlob,
            //                 &error_blob);
            //             if (FAILED(hr)) {
            //                 if (error_blob) {
            //                     OutputDebugStringA((char*)error_blob->GetBufferPointer());
            //                     error_blob->Release();
            //                 }
            //                 if (vertexShaderBlob) { vertexShaderBlob->Release(); }
            //                 ASSERT(false);
            //             }

            //             // COMPILE PIXEL SHADER
            //             hr = D3DCompileFromFile(
            //                 L"shaders.hlsl",
            //                 nullptr,
            //                 D3D_COMPILE_STANDARD_FILE_INCLUDE,
            //                 "ps_main",
            //                 "ps_5_0",
            //                 flags,
            //                 0,
            //                 &pixelShaderBlob,
            //                 &error_blob);
            //             if (FAILED(hr)) {
            //                 if (error_blob) {
            //                     OutputDebugStringA((char*)error_blob->GetBufferPointer());
            //                     error_blob->Release();
            //                 }
            //                 if (pixelShaderBlob) { pixelShaderBlob->Release(); }
            //                 ASSERT(false);
            //             }

            //             ID3D11VertexShader* vertexShader;
            //             ID3D11PixelShader* pixelShader;
            //             d3d11.device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), 0, &vertexShader);
            //             d3d11.device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), 0, &pixelShader);


            //             d3d11.deviceContext->VSSetShader(vertexShader, 0, 0);
            //             d3d11.deviceContext->PSSetShader(pixelShader, 0, 0);

            //             D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
            //             {
            //                 {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            //                 { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(Vec3), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            //             };
            //             ID3D11InputLayout* inputLayout = {};
            //             d3d11.device->CreateInputLayout(layoutDesc, ARRAY_COUNT(layoutDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayout);
            //             d3d11.deviceContext->IASetInputLayout(inputLayout);

            //             UINT stride[] = { sizeof(Vert) , sizeof(Vert) };
            //             UINT offset[] = { offsetof(Vert, pos), offsetof(Vert, uv) };
            //             d3d11.deviceContext->IASetVertexBuffers(0, 2, &vertexBuffer, stride, offset);

            //             d3d11.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            d3d11.deviceContext->RSSetViewports(1, &viewPort);

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

            MSG msg;
            while (running)
            {
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

                PushRenderClear(renderGroup, { 1.0f, 1.0f, 1.0f, 1.0f });

                // D3D11_MAPPED_SUBRESOURCE mapped_subresource;
                // d3d11.deviceContext->Map((ID3D11Resource*)constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
                // ((ConstantBuffer*)(mapped_subresource.pData))[0].transform =
                // {

                //         Cos((f32)GetTickCount() / 100), Sin((f32)GetTickCount() / 100), 0.0f, 0.0f,
                //         -Sin((f32)GetTickCount() / 100), Cos((f32)GetTickCount() / 100), 0.0f, 0.0f,
                //         0.0f, 0.0f, 1.0f, 0.0f,
                //         0.0f, 0.0f, 0.0f, 1.0f,
                // };

                // d3d11.deviceContext->Unmap((ID3D11Resource*)constantBuffer, 0);

                // d3d11.deviceContext->DrawIndexed(ARRAY_COUNT(indicies), 0, 0);

                Win32RenderOutput(renderGroup, d3d11);

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
