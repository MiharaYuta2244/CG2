#include "Particle.hlsli"

struct Material
{
    float4 color;
    float4x4 uvTransform;
    float alphaCutoff;
    float3 padding;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    if (textureColor.a <= gMaterial.alphaCutoff)
    {
        discard;
    }
    
    PixelShaderOutput output;   
    output.color = gMaterial.color * textureColor * input.color;
    
    return output;
}