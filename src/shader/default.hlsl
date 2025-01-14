#include "utils.hlsl"

/* vertex attributes go here to input to the vertex shader */
struct VSIn {
    float3 positionLocal : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    nointerpolation  uint4 boneIds : BONE_IDS;
    float4 weights : WEIGHTS;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct VSOut {
    float4 position : SV_POSITION; // required output of VS
    float3 normal : NORMAL;
    float2 uv: TEXCOORD;
    nointerpolation  uint4 boneIds : BONE_IDS;
    float4 weights : WEIGHTS;
};

struct PSOut {
    float4 color : SV_TARGET0; // required output of VS
};



