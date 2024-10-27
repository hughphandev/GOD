

/* vertex attributes go here to input to the vertex shader */
struct vs_in {
    float3 position_local : POSITION;
    float2 uv : TEXCOORD;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
    float2 uv: TEXCOORD;
    float4 color : COLOR0; // required output of VS
};

cbuffer cbuf
{
  matrix modelView;
  matrix proj;
  float4 color;
};

vs_out vs_main(vs_in input) {
  vs_out output = (vs_out)0; // zero the memory first
  output.position_clip = mul(mul(float4(input.position_local, 1.0), modelView), proj);
  // float z = output.position_clip.z;
  // float w = output.position_clip.z;
  output.position_clip.z = 0.5;
  output.color = color;
  output.uv = input.uv;
  return output;
}

Texture2D meshTexture;

SamplerState MeshTextureSampler;

struct ps_out {
    float4 color : SV_TARGET0; // required output of VS
};

ps_out ps_main(vs_out input) {
  ps_out output = (ps_out)0;
  output.color = meshTexture.Sample(MeshTextureSampler, input.uv) * input.color;
  return output; // must return an RGBA colour
}