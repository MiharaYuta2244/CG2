#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer DeathEffectParam : register(b0)
{
    float intensity; // エフェクトの適用強度
    float3 padding;
};

float4 main(VertexShaderOutput input) : SV_TARGET
{
    float4 texColor = gTexture.Sample(gSampler, input.texcoord);
    
    // カラーを輝度に変換
    float luminance = dot(texColor.rgb, float3(0.2125, 0.7154, 0.0721));
    
    float3 black = float3(0.01, 0.01, 0.01); // 背景などの暗い部分
    float3 red = float3(0.6, 0.0, 0.0); // 壁などの中間輝度部分
    float3 white = float3(0.9, 0.9, 0.9); // プレイヤーなどの明るい部分
    
    // 輝度に応じて3色に分ける
    float3 effectColor;
    if (luminance < 0.05)
    {
        effectColor = black;
    }
    else if (luminance < 0.3)
    {
        effectColor = red;
    }
    else
    {
        effectColor = white;
    }
    
    // intensityに応じて元の画面の色と補間する
    float3 finalColor = lerp(texColor.rgb, effectColor, intensity);
    
    return float4(finalColor, texColor.a);
}