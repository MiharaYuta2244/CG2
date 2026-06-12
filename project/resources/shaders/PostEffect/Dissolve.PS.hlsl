#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gMaskTexture : register(t1);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer DissolveParam : register(b0)
{
    float threshold;
    float3 edgeColor;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float mask = gMaskTexture.Sample(gSampler, input.texcoord);
    
    if (mask <= threshold)
    {
        discard;
    }
    
    float edge = 1.0f - smoothstep(threshold, (threshold + 0.03f), mask);
    output.color = gTexture.Sample(gSampler, input.texcoord);
    output.color.rgb += edge * edgeColor;

    return output;
}