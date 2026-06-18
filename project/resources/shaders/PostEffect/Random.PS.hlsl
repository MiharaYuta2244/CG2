#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer RandomParam : register(b0)
{
    float time;
    float3 padding;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float rand2dTold(float2 texcoord)
{
    return frac(sin(dot(texcoord, float2(12.9898, 78.233))) * 43758.5453123);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float random = rand2dTold(input.texcoord * time);
    output.color = gTexture.Sample(gSampler, input.texcoord) * float4(random, random, random, 1.0f);
    return output;
}