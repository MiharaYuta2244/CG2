#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct DistortionParam
{
    float time; // 経過時間
    float speed; // 揺れの速度
    float amplitude; // 揺れの幅
    float frequency; // 揺れの細かさ
};
ConstantBuffer<DistortionParam> gParam : register(b0);

float4 main(VertexShaderOutput input) : SV_TARGET
{
    float2 uv = input.texcoord;
    
    // Y座標と時間に基づいて、X座標をどれくらいズラすかを計算
    float offset = sin(uv.y * gParam.frequency + gParam.time * gParam.speed) * gParam.amplitude;
    
    // UVを歪ませる
    uv.x += offset;
    
    // サンプリング
    return gTexture.Sample(gSampler, uv);
}