#include "Sprite.hlsli"

struct Material
{
    float4 color;
    float4x4 uvTransform;
    // x: shineTime
    // y: shineWidth
    // z: shineIntensity
    // w: 未使用
    float4 shineParams;
    float4 shineColor;
    int enableShine;
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
    // UV変換
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    // アルファテスト
    if (textureColor.a < 0.5)
    {
        discard;
    }
    
    // パラメータ展開
    float shineTime = gMaterial.shineParams.x;
    float shineWidth = gMaterial.shineParams.y;
    float shineIntensity = gMaterial.shineParams.z;

    // 斜めのラインを作るための座標計算
    // これで左上から右下への斜めのグラデーション座標ができます
    float sheenPos = input.texcoord.x + input.texcoord.y;

    // 時間経過による光の位置計算
    // shineTimeをマッピングして、画面外から画面外へ移動させる
    float currentPos = (shineTime * 3.0f) - 0.5f;

    // 現在位置とラインの距離を計算
    float dist = abs(sheenPos - currentPos);

    // 距離に基づいて光の強さを計算
    // smoothstepで、距離0のとき1.0、距離widthで0.0になる
    float shineFactor = smoothstep(shineWidth, 0.0f, dist);
    
    // 光の色を加算
    // 元の色に足し合わせる
    float3 shineColor = float3(gMaterial.shineColor.x, gMaterial.shineColor.y, gMaterial.shineColor.z) * shineFactor * shineIntensity;

    PixelShaderOutput output;
    if (gMaterial.enableShine)
    {
        // 元の色に乗算色を掛け、その上に光沢色を足す
        output.color.rgb = (gMaterial.color.rgb * textureColor.rgb) + shineColor;
    }
    else
    {
        output.color.rgb = (gMaterial.color.rgb * textureColor.rgb);
    }
    output.color.a = gMaterial.color.a * textureColor.a;

    // 最終的なアルファチェック
    if (output.color.a == 0.0)
    {
        discard;
    }

    return output;
}