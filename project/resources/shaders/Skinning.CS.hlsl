struct VertexInput
{
    float4 position;
    float2 texcoord;
    float3 normal;
};

struct VertexInfluence
{
    float4 weight;
    int4 index;
};

struct WellForGPU
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInverseTranspose;
};

struct VertexOutput
{
    float4 position;
    float2 texcoord;
    float3 normal;
};

StructuredBuffer<VertexInput> gInputVertices : register(t0);
StructuredBuffer<VertexInfluence> gInfluences : register(t1);
StructuredBuffer<WellForGPU> gMatrixPalette : register(t2);
RWStructuredBuffer<VertexOutput> gOutputVertices : register(u0);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint vertexIndex = DTid.x;
    VertexInput input = gInputVertices[vertexIndex];
    VertexInfluence influence = gInfluences[vertexIndex];
    
    float4x4 skinMatrix = (float4x4) 0;
    float4x4 skinInvTransMatrix = (float4x4) 0;
    
    // スキニング行列の計算
    for (int i = 0; i < 4; ++i)
    {
        float weight = influence.weight[i];
        int matrixIndex = influence.index[i];
        skinMatrix += weight * gMatrixPalette[matrixIndex].skeletonSpaceMatrix;
        skinInvTransMatrix += weight * gMatrixPalette[matrixIndex].skeletonSpaceInverseTranspose;
    }
    
    // 頂点の変換
    VertexOutput output;
    output.position = mul(input.position, skinMatrix);
    output.position.w = 1.0f;
    output.normal = normalize(mul(input.normal, (float3x3) skinInvTransMatrix));
    output.texcoord = input.texcoord;
    
    // UAVに書き込み
    gOutputVertices[vertexIndex] = output;
}