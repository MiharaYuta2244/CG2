#include "Object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};

struct WellForGPU
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInverseTranspose;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
StructuredBuffer<WellForGPU> gMatrixPalette : register(t3);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 weight : WEIGHT0;
    int4 index : INDEX0;
};

struct Skinning
{
    float4x4 Mat;
    float4x4 InverseTransposeMatrix;
};

Skinning ComputeSkinMatrix(VertexShaderInput input)
{
    Skinning skinning;
    
    skinning.Mat = (float4x4) 0;
    skinning.InverseTransposeMatrix = (float4x4) 0;
    
    for (int i = 0; i < 4; ++i)
    {
        skinning.Mat += input.
        weight[i] * gMatrixPalette[input.index[i]].skeletonSpaceMatrix;
        skinning.InverseTransposeMatrix += input.weight[i] * gMatrixPalette[input.index[i]].skeletonSpaceInverseTranspose;
    }
    
    return skinning;
}

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    Skinning skinning = ComputeSkinMatrix(input);
    
    VertexShaderInput skinned;
    skinned.position = mul(input.position, skinning.Mat);
    skinned.position.w = 1.0f;
    
    skinned.normal = mul(input.normal, (float3x3) skinning.InverseTransposeMatrix);
    skinned.normal = normalize(skinned.normal);
    skinned.texcoord = input.texcoord;
    output.position = mul(skinned.position, gTransformationMatrix.WVP);
    output.texcoord = skinned.texcoord;
    output.normal = normalize(mul(skinned.normal, (float3x3) gTransformationMatrix.WorldInverseTranspose));
    output.worldPosition = mul(skinned.position, gTransformationMatrix.World).xyz;
    
    return output;
}