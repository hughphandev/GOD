#include "default.hlsl"

cbuffer VSPerScene : register(b2)
{

}

cbuffer VSPerFrame : register(b1)
{

}

cbuffer VSPerInstance : register(b0)
{
  matrix mvp;
};

VSOut vs_main(VSIn input) {
  VSOut output = (VSOut)0; // zero the memory first
  output.position = mul(float4(input.position_local, 1.0), mvp);
  output.uv = input.uv;
  output.normal = input.normal;
  return output;
}