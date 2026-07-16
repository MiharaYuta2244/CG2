#include "Particle.hlsli"

struct PerView
{
    float4x4 viewProjection;
    float4x4 billboardMatrix;
};

// リソースのバインド
StructuredBuffer<Particle> gParticles : register(t0);
ConstantBuffer<PerView> gPerView : register(b0);

VertexShaderOutput main(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    
    // 6頂点分のローカル座標
    static const float4 positions[6] =
    {
        float4(-0.5f, -0.5f, 0.0f, 1.0f), // 左下
        float4(-0.5f, 0.5f, 0.0f, 1.0f), // 左上
        float4(0.5f, -0.5f, 0.0f, 1.0f), // 右下
        float4(-0.5f, 0.5f, 0.0f, 1.0f), // 左上
        float4(0.5f, 0.5f, 0.0f, 1.0f), // 右上
        float4(0.5f, -0.5f, 0.0f, 1.0f) // 右下
    };

    // 6頂点分のUV座標
    static const float2 texcoords[6] =
    {
        float2(0.0f, 1.0f), // 左下
        float2(0.0f, 0.0f), // 左上
        float2(1.0f, 1.0f), // 右下
        float2(0.0f, 0.0f), // 左上
        float2(1.0f, 0.0f), // 右上
        float2(1.0f, 1.0f) // 右下
    };

    // インスタンスごとのデータを取得
    Particle particle = gParticles[instanceId];
    
    // 頂点インデックスからローカル座標とUVを取得
    float4 localPosition = positions[vertexId];
    float2 uv = texcoords[vertexId];

    // ビルボード行列を取得してワールド行列を作成
    float4x4 worldMatrix = gPerView.billboardMatrix;
    
    // スケールを適用
    worldMatrix[0].xyz *= particle.scale.x;
    worldMatrix[1].xyz *= particle.scale.y;
    worldMatrix[2].xyz *= particle.scale.z;
    
    // 平行移動を適用
    worldMatrix[3].xyz = particle.translate;

    // ローカル座標をワールド座標に変換
    float4 worldPosition = mul(localPosition, worldMatrix);

    // 最終的な出力パラメータの計算
    output.position = mul(worldPosition, gPerView.viewProjection);
    output.texcoord = uv;
    output.color = particle.color;
    output.worldPosition = worldPosition.xyz;

    return output;
}