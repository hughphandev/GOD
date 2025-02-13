#ifndef HZ_D3D11_H
#define HZ_D3D11_H
#include <d3d11.h>
#include <d3dcompiler.h>
#include <windowsx.h>

#include "hz_math.h"
#include "hz_io.h"
#include "hz_render.h"

struct D3D11Mesh
{
    bool isValid;
    Mat4 transform;
    UINT vertexCount;
    ID3D11Buffer* vertexBuffer;
    UINT indexCount;
    ID3D11Buffer* indexBuffer;
    UINT shaderResIndex;

    ID3D11InputLayout* inputLayout;
    ID3D11SamplerState* samplerState;

    ID3D11Buffer* vsPerInstance;
    ID3D11Buffer* psPerInstance;

    UINT stride[1];
    UINT offset[1];
};

struct D3D11Model
{
    u32 meshCount;
    D3D11Mesh* meshes;
};

struct D3D11Texture
{
    bool isValid;
    u32 width, height;
    ID3D11ShaderResourceView* shaderRes;
};

struct Renderer
{
    IDXGISwapChain* swapChain;
    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11DepthStencilView* depthStencilView;


    //TODO: test code
#define MAX_MODEL_COUNT 256
    u32 modelCount;
    D3D11Model models[MAX_MODEL_COUNT];
#define MAX_TEXTURE_COUNT 8
    D3D11Texture textures[MAX_TEXTURE_COUNT];

    ID3D11Buffer* vsPerFrame;
    ID3D11Buffer* psPerFrame;

    ID3D11Buffer* vsPerScene;
    ID3D11Buffer* psPerScene;

    ID3D11VertexShader* vsDefaultShader;
    ID3D11PixelShader* psDefaultShader;

    ID3D11VertexShader* vsVoxelShader;
    ID3D11PixelShader* psVoxelShader;

    void* defaultVertexShader;
    u32 defaultVertexShaderSize;
    void* defaultPixelShader;
    u32 defaultPixelShaderSize;

    void* voxelVertexShader;
    u32 voxelVertexShaderSize;
    void* voxelPixelShader;
    u32 voxelPixelShaderSize;
};

static void D3d11InitConstantBuffer(Renderer* renderer, void* data, UINT size, ID3D11Buffer** buffer)
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
    renderer->device->CreateBuffer(&constBufferDesc, &constResDesc, buffer);
}

static D3D11Mesh D3D11LoadMesh(Renderer* renderer, LoadedMesh initialMesh)
{
    D3D11Mesh result = {};
    result.vertexCount = initialMesh.vertexCount;
    result.indexCount = initialMesh.indexCount;
    result.transform = initialMesh.transform;
    result.shaderResIndex = initialMesh.matIndex;

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(*initialMesh.vertices) * initialMesh.vertexCount;
    //TODO: static vs dynamic mesh
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = sizeof(*initialMesh.vertices);
    D3D11_SUBRESOURCE_DATA vertexResDesc = {};
    vertexResDesc.pSysMem = initialMesh.vertices;
    renderer->device->CreateBuffer(&vertexBufferDesc, &vertexResDesc, &result.vertexBuffer);


    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(*initialMesh.indices) * initialMesh.indexCount;
    indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = sizeof(*initialMesh.indices);
    D3D11_SUBRESOURCE_DATA indexResDesc = {};
    indexResDesc.pSysMem = initialMesh.indices;
    renderer->device->CreateBuffer(&indexBufferDesc, &indexResDesc, &result.indexBuffer);


    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    renderer->device->CreateSamplerState(&samplerDesc, &result.samplerState);


    VSPerInstance vsPerInstance = {};
    D3d11InitConstantBuffer(renderer, &vsPerInstance, sizeof(VSPerInstance), &result.vsPerInstance);

    PSPerInstance psPerInstance = {};
    D3d11InitConstantBuffer(renderer, &psPerInstance, sizeof(PSPerInstance), &result.psPerInstance);


    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vert, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vert, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vert, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BONE_IDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(Vert, boneIds), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vert, weights), D3D11_INPUT_PER_VERTEX_DATA, 0},
        // {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(Vec3), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    renderer->device->CreateInputLayout(layoutDesc, ARRAY_COUNT(layoutDesc), renderer->defaultVertexShader, renderer->defaultVertexShaderSize, &result.inputLayout);

    result.stride[0] = sizeof(*initialMesh.vertices);
    result.offset[0] = 0;
    return result;
}

static void Win32UpdateMesh(Renderer* renderer, u32 modelId, u32 meshId, LoadedMesh meshData)
{
    D3D11Mesh* target = &renderer->models[modelId].meshes[meshId];
    if (target->vertexCount == meshData.vertexCount)
    {
        D3D11_MAPPED_SUBRESOURCE subRes;

        renderer->deviceContext->Map(target->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);

        Memcpy(subRes.pData, meshData.vertices, sizeof(*meshData.vertices) * meshData.vertexCount);

        renderer->deviceContext->Unmap(target->vertexBuffer, 0);
    }
    else
    {
        target->vertexCount = meshData.vertexCount;
        if (target->vertexBuffer) target->vertexBuffer->Release();
        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = sizeof(*meshData.vertices) * meshData.vertexCount;
        //TODO: static vs dynamic mesh
        vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = sizeof(*meshData.vertices);
        D3D11_SUBRESOURCE_DATA vertexResDesc = {};
        vertexResDesc.pSysMem = meshData.vertices;
        renderer->device->CreateBuffer(&vertexBufferDesc, &vertexResDesc, &target->vertexBuffer);
    }

    if (target->indexCount == meshData.indexCount)
    {
        D3D11_MAPPED_SUBRESOURCE subRes;

        renderer->deviceContext->Map(target->indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);

        Memcpy(subRes.pData, meshData.indices, sizeof(*meshData.indices) * meshData.indexCount);

        renderer->deviceContext->Unmap(target->indexBuffer, 0);
    }
    else
    {
        target->indexCount = meshData.indexCount;
        if (target->indexBuffer) target->indexBuffer->Release();
        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = sizeof(*meshData.indices) * meshData.indexCount;
        indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        indexBufferDesc.MiscFlags = 0;
        indexBufferDesc.StructureByteStride = sizeof(*meshData.indices);
        D3D11_SUBRESOURCE_DATA indexResDesc = {};
        indexResDesc.pSysMem = meshData.indices;
        renderer->device->CreateBuffer(&indexBufferDesc, &indexResDesc, &target->indexBuffer);
    }
}

static u32 Win32UploadModel(Renderer* renderer, LoadedModel initialModel, MemoryArena* arena)
{
    u32 result = renderer->modelCount++;
    D3D11Model* win32Model = &renderer->models[result];

    win32Model->meshCount = initialModel.meshCount;
    win32Model->meshes = PUSH_ARRAY(arena, D3D11Mesh, initialModel.meshCount);

    for (u32 i = 0; i < initialModel.meshCount; ++i)
    {
        win32Model->meshes[i] = D3D11LoadMesh(renderer, initialModel.meshes[i]);
    }
    return result;
}

static u32 Win32UploadTexture(Renderer* renderer, Texture texture, MemoryArena* arena)
{
    u32 result = INVALID_VALUE;
    for (int i = 0; i < MAX_TEXTURE_COUNT; ++i)
    {
        if (!renderer->textures[i].isValid)
        {
            renderer->textures[i].isValid = true;
            result = i;
            break;
        }
    }
    ASSERT(result != INVALID_VALUE);

    D3D11Texture* win32Texture = &renderer->textures[result];
    win32Texture->shaderRes = PUSH_TYPE(arena, ID3D11ShaderResourceView);
    if (texture.width > 0 && texture.height > 0)
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = texture.width;
        textureDesc.Height = texture.height;
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
        initData.pSysMem = texture.texel;
        initData.SysMemPitch = sizeof(*texture.texel) * texture.width;
        initData.SysMemSlicePitch = sizeof(*texture.texel) * texture.width * texture.height;

        ID3D11Texture2D* tex = nullptr;
        renderer->device->CreateTexture2D(&textureDesc, &initData, &tex);

        D3D11_SHADER_RESOURCE_VIEW_DESC resDesc;
        resDesc.Format = textureDesc.Format;
        resDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        resDesc.Texture2D.MostDetailedMip = 0;
        resDesc.Texture2D.MipLevels = 1;
        renderer->device->CreateShaderResourceView(tex, &resDesc, &win32Texture->shaderRes);
    }
    return result;
}


static void D3D11InitScene(u32 width, u32 height, RenderGroup* renderGroup, MemoryArena* arena, HWND windowHandle)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
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

    HRESULT result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_DEBUG, 0, 0, D3D11_SDK_VERSION, &swapChainDesc, &renderGroup->renderer->swapChain, &renderGroup->renderer->device, 0, &renderGroup->renderer->deviceContext);

    if (SUCCEEDED(result))
    {
        Renderer* renderer = renderGroup->renderer;
        ID3D11Texture2D* frameBuffer;
        if (!SUCCEEDED(renderer->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer)))
        {
            //TODO: Logging
            ASSERT(false);
        }

        if (!SUCCEEDED(renderer->device->CreateRenderTargetView(frameBuffer, 0, &renderer->renderTargetView)))
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
        renderer->device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);

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

            renderer->device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
        }

        // renderer->deviceContext->RSSetState(rasterizerState);
        // renderer->deviceContext->OMSetDepthStencilState(depthStencilState, 0);

        D3D11_TEXTURE2D_DESC depthTextureDesc = {};
        depthTextureDesc.Width = width;
        depthTextureDesc.Height = height;
        depthTextureDesc.MipLevels = 1;
        depthTextureDesc.ArraySize = 1;
        depthTextureDesc.SampleDesc.Count = 1;
        depthTextureDesc.SampleDesc.Quality = 0;
        depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        ID3D11Texture2D* depthStencilTexture;
        result = renderer->device->CreateTexture2D(&depthTextureDesc, NULL, &depthStencilTexture);

        if (result != S_OK)
        {
            ASSERT(false);
            //TODO: Logging
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = depthTextureDesc.Format;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        result = renderer->device->CreateDepthStencilView(depthStencilTexture, NULL, &renderer->depthStencilView);
        depthStencilTexture->Release();

        if (result != S_OK)
        {
            ASSERT(false);
            //TODO: Logging
        }

        // renderer->deviceContext->OMSetRenderTargets(1, &renderer->renderTargetView, renderer->depthStencilView);

        D3D11_VIEWPORT viewPort = {};
        viewPort.TopLeftX = 0;
        viewPort.TopLeftY = 0;
        viewPort.Width = (FLOAT)width;
        viewPort.Height = (FLOAT)height;
        viewPort.MinDepth = 0;
        viewPort.MaxDepth = 1;

        VSPerFrame vsPerFrame = {};
        D3d11InitConstantBuffer(renderer, &vsPerFrame, sizeof(VSPerFrame), &renderer->vsPerFrame);

        PSPerFrame psPerFrame = {};
        D3d11InitConstantBuffer(renderer, &psPerFrame, sizeof(PSPerFrame), &renderer->psPerFrame);

        VSPerScene vsPerScene = {};
        D3d11InitConstantBuffer(renderer, &vsPerScene, sizeof(VSPerScene), &renderer->vsPerScene);

        PSPerScene psPerScene = {};
        D3d11InitConstantBuffer(renderer, &psPerScene, sizeof(PSPerScene), &renderer->psPerScene);

        File vsDefaultFile = ReadFile("default_vs.fxo", arena);
        File psDefaultFile = ReadFile("default_ps.fxo", arena);
        renderer->defaultVertexShader = vsDefaultFile.content;
        renderer->defaultVertexShaderSize = vsDefaultFile.contentSize;
        renderer->defaultPixelShader = psDefaultFile.content;
        renderer->defaultPixelShaderSize = psDefaultFile.contentSize;
        renderer->device->CreateVertexShader(renderer->defaultVertexShader, renderer->defaultVertexShaderSize, 0, &renderer->vsDefaultShader);
        renderer->device->CreatePixelShader(renderer->defaultPixelShader, renderer->defaultPixelShaderSize, 0, &renderer->psDefaultShader);

        File vsVoxelFile = ReadFile("voxel_vs.fxo", arena);
        File psVoxelFile = ReadFile("voxel_ps.fxo", arena);
        renderer->voxelVertexShader = vsVoxelFile.content;
        renderer->voxelVertexShaderSize = vsVoxelFile.contentSize;
        renderer->voxelPixelShader = psVoxelFile.content;
        renderer->voxelPixelShaderSize = psVoxelFile.contentSize;
        renderer->device->CreateVertexShader(renderer->voxelVertexShader, renderer->voxelVertexShaderSize, 0, &renderer->vsVoxelShader);
        renderer->device->CreatePixelShader(renderer->voxelPixelShader, renderer->voxelPixelShaderSize, 0, &renderer->psVoxelShader);


        renderer->deviceContext->VSSetConstantBuffers(2, 1, &renderer->vsPerScene);
        renderer->deviceContext->PSSetConstantBuffers(2, 1, &renderer->psPerScene);

        renderer->deviceContext->RSSetViewports(1, &viewPort);
    }
}

#endif