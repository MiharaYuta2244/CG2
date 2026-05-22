#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer SepiaParam : register(b0)
{
    float intensity; // 0〜1
    float3 sepiaColor; // セピア色
    float toneStrength; // 色味の強さ
    float padding;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{    
    PixelShaderOutput output;
    float3 col = gTexture.Sample(gSampler, input.texcoord).rgb;

    float gray = dot(col, float3(0.2125, 0.7154, 0.0721));
    float3 sepia = gray * sepiaColor * toneStrength;

    output.color.rgb = lerp(col, sepia, intensity);
    output.color.a = 1.0;
    return output;
}