#include "Sprite.hlsli"

struct Material
{
    float32_t4 color;
    float32_t4x4 uvTransform;
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    // textureのa値が0のときにPixelを棄却
    if (textureColor.a < 0.5)
    {
        discard;
    }
    
    PixelShaderOutput output;
    output.color = gMaterial.color * textureColor;
    
    // output.colorのa値が0のときにPixelを棄却
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}