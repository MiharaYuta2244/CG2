struct VertexShaderInput
{
    float4 position : POSITION0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 texcoord : TEXCOORD0; 
};

struct TransformationMatrix
{
    float4x4 WVP;
};

ConstantBuffer<TransformationMatrix> gTransform : register(b0);

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransform.WVP);
    output.position.z = output.position.w;
    output.texcoord = input.position.xyz;
    
    return output;
}