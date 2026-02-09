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
    float4 gradientTopColor; // 上端の色
    float4 gradientBottomColor; // 下端の色
    int enableShine;
    int enableGradient; // グラデーション有効フラグ
    float2 padding; // パディング
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
    
    float4 finalColor = gMaterial.color * textureColor; // ベースの色

    if (gMaterial.enableGradient)
    {
        // UVのY座標に基づいて色を補間する (0.0が上、1.0が下)
        // input.texcoord (UV) を使うことで、テクスチャの切り出し範囲に関わらず
        // スプライト全体に対してグラデーションがかかるようになります。
        // もし「切り出した画像の中でのグラデーション」にしたい場合は transformedUV.y を使ってください。
        // ここでは一般的な「スプライトの上から下へのグラデーション」として input.texcoord.y を使用します。
        float4 gradColor = lerp(gMaterial.gradientTopColor, gMaterial.gradientBottomColor, input.texcoord.y);
        
        // 元の色に乗算合成
        finalColor *= gradColor;
    }

    // パラメータ展開
    float shineTime = gMaterial.shineParams.x;
    float shineWidth = gMaterial.shineParams.y;
    float shineIntensity = gMaterial.shineParams.z;

    // 斜めのラインを作るための座標計算
    float sheenPos = input.texcoord.x + input.texcoord.y;
    // 時間経過による光の位置計算
    float currentPos = (shineTime * 3.0f) - 0.5f;
    // 現在位置とラインの距離を計算
    float dist = abs(sheenPos - currentPos);
    // 距離に基づいて光の強さを計算
    float shineFactor = smoothstep(shineWidth, 0.0f, dist);
    // 光の色を加算
    float3 shineColor = float3(gMaterial.shineColor.x, gMaterial.shineColor.y, gMaterial.shineColor.z) * shineFactor * shineIntensity;

    PixelShaderOutput output;
    
    // Shineの合成
    if (gMaterial.enableShine)
    {
        // 計算済みの色に光沢色を足す
        output.color.rgb = finalColor.rgb + shineColor;
    }
    else
    {
        output.color.rgb = finalColor.rgb;
    }
    
    output.color.a = finalColor.a;

    // 最終的なアルファチェック
    if (output.color.a == 0.0)
    {
        discard;
    }

    return output;
}