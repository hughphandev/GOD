#include "default.hlsl"

cbuffer PSPerScene : register(b2)
{

}

cbuffer PSPerFrame : register(b1)
{
  float3 lightDirection;
}

cbuffer PSPerInstance : register(b0)
{
  float4 color;
  float4 diffuse;
  float4 ambient;
};

Texture2D meshTexture;

SamplerState MeshTextureSampler;


PSOut ps_main(VSOut input) {
  PSOut output = (PSOut)0;
  float intensity = dot(input.normal, -lightDirection);
  intensity = intensity / 0.49;
  intensity = floor(intensity);
  intensity = intensity * 0.49; 
  intensity = remap(0, 1, 0.3, 1, intensity);
  float4 tint = diffuse * intensity + ambient;
  output.color = meshTexture.Sample(MeshTextureSampler, input.uv) * tint;
  return output; // must return an RGBA colour
}