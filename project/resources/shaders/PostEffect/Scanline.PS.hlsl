#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct ScanlineParam
{
    float scanlineCount; // 線の数
    float intensity; // 暗さ
    float speed; // 走査線の移動速度
    float time;
};
ConstantBuffer<ScanlineParam> gParam : register(b0);

float4 main(VertexShaderOutput input) : SV_TARGET
{
    float4 color = gTexture.Sample(gSampler, input.texcoord);

    // timeを使って縦方向に動かす
    float y = input.texcoord.y + gParam.speed * gParam.time;

    float sineValue = sin(y * gParam.scanlineCount * 3.14159265f * 2.0f);

    float scanlineMultiplier = 1.0f - (gParam.intensity * (sineValue * 0.5f + 0.5f));

    color.rgb *= scanlineMultiplier;

    return color;
}
