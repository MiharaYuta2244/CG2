struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 texcoord : TEXCOORD0;
};

TextureCube<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 main(VertexShaderOutput input) : SV_TARGET
{
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    return textureColor;
}