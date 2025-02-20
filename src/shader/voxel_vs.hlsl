#include "voxel.hlsl"

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
  bool isSkinnedMesh;
};

VSOut main(VSIn input)
{
    VSOut output = (VSOut)0; // zero the memory first
    output.uv = float2(input.vertexId&1,input.vertexId>>1); //you can use these for texture coordinates later
    output.position = float4((output.uv.x-0.5f)*2,-(output.uv.y-0.5f)*2,0,1);
    return output;
}