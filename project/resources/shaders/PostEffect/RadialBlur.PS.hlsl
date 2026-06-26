#include "FullScreen.hlsli"

Texture2D gTexture : register(t0);
SamplerState gSamplerLinear : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer RadialBlurParam : register(b0)
{
    float2 center; // 中心点
    float blurWidth; // ぼかしの幅
    float numSamples; // サンプリング数
};

PixelShaderOutput main(VertexShaderOutput input)
{
    // ぼかす方向をとりあえず単位ベクトルを計算
    float2 direction = input.texcoord - center;

    float3 outputColor = float3(0.0f, 0.0f, 0.0f);

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        // 現在の中心からぼかす方向にサンプリング点を進めながらサンプリングしていく
        float2 texcoord = input.texcoord + direction * blurWidth * float(sampleIndex);
        outputColor += gTexture.Sample(gSamplerLinear, texcoord).rgb;
    }

    // 平均化する
    outputColor *= rcp(numSamples);
    
    PixelShaderOutput output;
    output.color.rgb = outputColor;
    output.color.a = 1.0f;
    
    return output;
}
