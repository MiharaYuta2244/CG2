#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct FisheyeParam
{
    float strength; // 歪みの強さ
    float3 padding;
};
ConstantBuffer<FisheyeParam> gParam : register(b0);

float4 main(VertexShaderOutput input) : SV_TARGET
{
    float2 ndc = input.texcoord * 2.0f - 1.0f;
    float r2 = dot(ndc, ndc);
    float k = gParam.strength;
    float2 distorted = ndc * (1.0 + k * r2);
    float2 uv = distorted * 0.5f + 0.5f;

    // 画面外は黒
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        return float4(0, 0, 0, 1);
    }

    return gTexture.Sample(gSampler, uv);
}
