#include "default.hlsl"

cbuffer VSPerScene : register(b2)
{

}

cbuffer VSPerFrame : register(b1)
{

}

#define MAX_BONES 100
cbuffer VSPerInstance : register(b0)
{
  matrix mvp;
  matrix model;
  matrix bones[MAX_BONES];
};

VSOut vs_main(VSIn input) {
  VSOut output = (VSOut)0; // zero the memory first
matrix boneTransform =
    bones[input.boneIds[0]] * input.weights[0] +
    bones[input.boneIds[1]] * input.weights[1] +
    bones[input.boneIds[2]] * input.weights[2] +
    bones[input.boneIds[3]] * input.weights[3] ; 
  output.position = mul(mul(float4(input.positionLocal, 1.0), boneTransform), mvp);
  output.uv = input.uv;
  output.normal = mul(input.normal, (float3x3)model);
  output.boneIds = input.boneIds;
  output.weights = input.weights;
  return output;
}