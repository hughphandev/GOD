#include "utils.hlsl"

struct VSIn {
    uint vertexId : SV_VertexID;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct VSOut {
    float4 position : SV_POSITION; // required output of VS
    float2 uv: TEXCOORD;
};

struct PSOut {
    float4 color : SV_TARGET0; // required output of VS
};



