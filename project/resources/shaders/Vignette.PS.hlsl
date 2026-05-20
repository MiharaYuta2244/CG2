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
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    // 周囲を0に、中心になるほど明るくなるように計算で調整
    float2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    
    // correctだけで計算すると中心の最大値が0.0625で暗すぎるので調整。(16倍)
    float vignette = correct.x * correct.y * 16.0f;
    
    // 0.8乗
    vignette = saturate(pow(vignette, 0.8f));
    
    // 係数として乗算
    output.color.rgb *= vignette;
    
    return output;
}