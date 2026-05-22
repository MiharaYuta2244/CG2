#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer GrayscaleParam : register(b0)
{
    float intensity; // 0〜1
    float3 luminanceWeight; // 輝度計算の重み
    float3 blendColor; // 混ぜる色
    float blendStrength; // 0〜1
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{    
    PixelShaderOutput output;
    float3 col = gTexture.Sample(gSampler, input.texcoord).rgb;

    float gray = dot(col, luminanceWeight);
    float3 grayCol = float3(gray, gray, gray);

    // 色を混ぜる
    grayCol = lerp(grayCol, blendColor, blendStrength);

    // 元の色とブレンド
    output.color.rgb = lerp(col, grayCol, intensity);
    output.color.a = 1.0;
    return output;
}