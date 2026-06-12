#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer VignetteCB : register(b0)
{
    float4 vignetteColor;
    float intensity;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 baseColor = gTexture.Sample(gSampler, input.texcoord);

    // 周囲が0、中心が最大になる係数
    float2 correct = input.texcoord * (1.0f - input.texcoord.yx);

    // 中心の最大値が小さいので16倍
    float vignette = correct.x * correct.y * 16.0f;

    // 0.8乗でカーブ調整
    vignette = saturate(pow(vignette, 0.8f));

    // intensityを適用
    vignette = lerp(1.0f, vignette, intensity);

    // vignetteColorをvignetteの強さに応じてブレンド
    float3 coloredVignette = lerp(float3(1.0f, 1.0f, 1.0f), vignetteColor.rgb, 1.0f - vignette);

    // 最終色
    output.color.rgb = baseColor.rgb * vignette * coloredVignette;
    output.color.a = baseColor.a;

    return output;
}
