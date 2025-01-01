#include "utils.hlsl"

/* vertex attributes go here to input to the vertex shader */
struct VSIn {
    float3 position_local : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct VSOut {
    float4 position : SV_POSITION; // required output of VS
    float3 normal : NORMAL;
    float2 uv: TEXCOORD;
};

struct PSOut {
    float4 color : SV_TARGET0; // required output of VS
};



