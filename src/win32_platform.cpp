#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <assert.h>
static bool running = true;


LRESULT WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
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
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    //  windowClass.hIcon;
    windowClass.lpszClassName = "GodClass";


    if (RegisterClassEx(&windowClass))
    {
        HWND windowHandle = CreateWindowEx(0, windowClass.lpszClassName, "GOD", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

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

        IDXGISwapChain* dxgiSwapChain;
        ID3D11Device* d3d11Device;
        ID3D11DeviceContext* d3d11DeviceContext;

        HRESULT result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_DEBUG, 0, 0, D3D11_SDK_VERSION, &swapChainDesc, &dxgiSwapChain, &d3d11Device, 0, &d3d11DeviceContext);

        if (windowHandle && SUCCEEDED(result))
        {
            ID3D11Texture2D* frameBuffer;
            if (!SUCCEEDED(dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer)))
            {
                //TODO: Logging
                assert(false);
            }

            ID3D11RenderTargetView* renderTargetView;
            if (!SUCCEEDED(d3d11Device->CreateRenderTargetView(frameBuffer, 0, &renderTargetView)))
            {
                //TODO: Logging
                assert(false);
            }

            UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
            flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif
            ID3DBlob* vertexShaderBlob = NULL, * pixelShaderBlob = NULL, * error_blob = NULL;

            // COMPILE VERTEX SHADER
            HRESULT hr = D3DCompileFromFile(
                L"shaders.hlsl",
                nullptr,
                D3D_COMPILE_STANDARD_FILE_INCLUDE,
                "vs_main",
                "vs_5_0",
                flags,
                0,
                &vertexShaderBlob,
                &error_blob);
            if (FAILED(hr)) {
                if (error_blob) {
                    OutputDebugStringA((char*)error_blob->GetBufferPointer());
                    error_blob->Release();
                }
                if (vertexShaderBlob) { vertexShaderBlob->Release(); }
                assert(false);
            }

            // COMPILE PIXEL SHADER
            hr = D3DCompileFromFile(
                L"shaders.hlsl",
                nullptr,
                D3D_COMPILE_STANDARD_FILE_INCLUDE,
                "ps_main",
                "ps_5_0",
                flags,
                0,
                &pixelShaderBlob,
                &error_blob);
            if (FAILED(hr)) {
                if (error_blob) {
                    OutputDebugStringA((char*)error_blob->GetBufferPointer());
                    error_blob->Release();
                }
                if (pixelShaderBlob) { pixelShaderBlob->Release(); }
                assert(false);
            }

            struct Vert
            {
                float x, y, z;
            };

            Vert vertices[] = {
                {0.0f, 0.5f, 0.0f},
                {0.5f, -0.5f, 0.0f},
                {-0.5f, -0.5f, 0.0f},
            };


            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = sizeof(vertices);
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0;
            bufferDesc.MiscFlags = 0;
            bufferDesc.StructureByteStride = sizeof(Vert);


            ID3D11Buffer* vertexBuffer;
            D3D11_SUBRESOURCE_DATA subResData = {};
            subResData.pSysMem = vertices;
            d3d11Device->CreateBuffer(&bufferDesc, &subResData, &vertexBuffer);

            UINT stride = sizeof(Vert);
            UINT offset = 0;
            d3d11DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

            D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
            {
                {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
            };
            ID3D11InputLayout* inputLayout = {};
            d3d11Device->CreateInputLayout(layoutDesc, sizeof(layoutDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayout);
            d3d11DeviceContext->IASetInputLayout(inputLayout);
            d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            ID3D11VertexShader* vertexShader;
            ID3D11PixelShader* pixelShader;
            d3d11Device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), 0, &vertexShader);
            d3d11Device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), 0, &pixelShader);

            d3d11DeviceContext->VSSetShader(vertexShader, 0, 0);
            d3d11DeviceContext->PSSetShader(pixelShader, 0, 0);

            D3D11_VIEWPORT viewPort;
            viewPort.TopLeftX = 0;
            viewPort.TopLeftY = 0;
            viewPort.Width = 800;
            viewPort.Height = 600;
            viewPort.MinDepth = 0;
            viewPort.MaxDepth = 1;

            d3d11DeviceContext->RSSetViewports(1, &viewPort);

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

                d3d11DeviceContext->OMSetRenderTargets(1, &renderTargetView, 0);
                const FLOAT color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                d3d11DeviceContext->ClearRenderTargetView(renderTargetView, color);
                d3d11DeviceContext->Draw(sizeof(vertices) / sizeof(Vert), 0);
                dxgiSwapChain->Present(0, 0);
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
