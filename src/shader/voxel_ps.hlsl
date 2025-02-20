#include "voxel.hlsl"

cbuffer PSPerScene : register(b2)
{

}

cbuffer PSPerFrame : register(b1)
{
  float3 lightDirection;
  float4 diffuse;
}

cbuffer PSPerInstance : register(b0)
{
  float4 color;
};

Texture2D meshTexture;

SamplerState MeshTextureSampler;


PSOut main(VSOut input)
{
    PSOut output = (PSOut)0;
    output.color = float4(1,0,0,1); //the red color
    return output;
}