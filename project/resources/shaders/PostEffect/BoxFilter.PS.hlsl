#include "FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer SmoothingParam : register(b0)
{
    int radius;
    float intensity;
    float2 texelSize;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = float4(0, 0, 0, 1);

    int size = radius * 2 + 1;
    float weight = 1.0f / (size * size);

    for (int y = -radius; y <= radius; ++y)
    {
        for (int x = -radius; x <= radius; ++x)
        {
            float2 offset = float2(x, y) * texelSize;
            float2 texcoord = input.texcoord + offset;

            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;

            output.color.rgb += fetchColor * weight;
        }
    }

    output.color.rgb *= intensity;
    return output;
}
