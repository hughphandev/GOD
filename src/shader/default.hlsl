

/* vertex attributes go here to input to the vertex shader */
struct vs_in {
    float3 position_local : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out {
    float4 position : SV_POSITION; // required output of VS
    float3 normal : NORMAL;
    float2 uv: TEXCOORD;
};

cbuffer cbuf
{
  matrix mvp;
  float4 color;
  float4 diffuse;
  float4 ambient;
  float3 lightDirection;
};

vs_out vs_main(vs_in input) {
  vs_out output = (vs_out)0; // zero the memory first
  output.position = mul(float4(input.position_local, 1.0), mvp);
  output.uv = input.uv;
  output.normal = input.normal;
  return output;
}

Texture2D meshTexture;

SamplerState MeshTextureSampler;

struct ps_out {
    float4 color : SV_TARGET0; // required output of VS
};

ps_out ps_main(vs_out input) {
  ps_out output = (ps_out)0;
  float intensity = dot(input.normal, -lightDirection);
  if(intensity < 0.3) 
  {
    intensity = 0;
  }
  else if(intensity < 0.7)
  {
    intensity = 0.5;
  }
  else
  {
    intensity = 1;
  }
  float4 tint = diffuse * intensity + ambient;
  output.color = meshTexture.Sample(MeshTextureSampler, input.uv) * tint;
  return output; // must return an RGBA colour
}