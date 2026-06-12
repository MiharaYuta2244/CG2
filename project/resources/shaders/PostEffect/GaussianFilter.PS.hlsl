#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer GaussianParam : register(b0)
{
    int radius; // 2 = 5x5
    float intensity;
    float sigma; // 標準偏差
    float2 texelSize;
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

static const float PI = 3.14159265f;

float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) * rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = float4(0, 0, 0, 1);

    float totalWeight = 0.0f;
    float3 sum = 0.0f;

    for (int y = -radius; y <= radius; ++y)
    {
        for (int x = -radius; x <= radius; ++x)
        {
            float2 offset = float2(x, y) * texelSize;
            float2 uv = input.texcoord + offset;

            float w = gauss(x, y, sigma);
            float3 c = gTexture.Sample(gSampler, uv).rgb;

            sum += c * w;
            totalWeight += w;
        }
    }

    // 正規化
    sum /= totalWeight;

    output.color.rgb = sum * intensity;
    return output;
}