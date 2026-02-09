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
    float4 gradientTopColor;
    float4 gradientBottomColor;
    // x: scale (密度)
    // y: speed (変動速度)
    // z: intensity (強さ)
    // w: time (経過時間)
    float4 voronoiParams;
    float4 voronoiColor;
    int enableShine;
    int enableGradient;
    int enableVoronoi;
    float padding;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// 乱数生成用関数
float2 random2(float2 p)
{
    return frac(sin(float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)))) * 43758.5453);
}

// ボロノイノイズ生成関数
float voronoi(float2 uv, float time)
{
    float2 i = floor(uv);
    float2 f = frac(uv);
    float minDist = 1.0;

    // 近傍3x3のグリッドを探索
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 neighbor = float2(float(x), float(y));
            // ランダムな特徴点の位置を取得
            float2 randomPoint = random2(i + neighbor);
            
            // 時間経過で特徴点を動かす
            randomPoint = 0.5 + 0.5 * sin(time + 6.2831 * randomPoint);
            
            float2 diff = neighbor + randomPoint
            -f;
            float dist = length(diff);

            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

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
    
    float4 finalColor = gMaterial.color * textureColor;

    // グラデーション
    if (gMaterial.enableGradient)
    {
        float4 gradColor = lerp(gMaterial.gradientTopColor, gMaterial.gradientBottomColor, input.texcoord.y);
        finalColor *= gradColor;
    }

    // ボロノイノイズ処理
    if (gMaterial.enableVoronoi)
    {
        float scale = gMaterial.voronoiParams.x;
        float time = gMaterial.voronoiParams.w * gMaterial.voronoiParams.y;
        float intensity = gMaterial.voronoiParams.z;

        // ボロノイパターン計算 
        float noise = 1.0 - voronoi(input.texcoord * scale, time);
        // コントラストを少し強める
        noise = pow(noise, 2.0);

        // ノイズ色を計算 (元の色に加算合成する形)
        float3 voronoiEffect = gMaterial.voronoiColor.rgb * noise * intensity;
        
        finalColor.rgb += voronoiEffect;
    }

    // Shine処理
    float shineTime = gMaterial.shineParams.x;
    float shineWidth = gMaterial.shineParams.y;
    float shineIntensity = gMaterial.shineParams.z;
    float sheenPos = input.texcoord.x + input.texcoord.y;
    float currentPos = (shineTime * 3.0f) - 0.5f;
    float dist = abs(sheenPos - currentPos);
    float shineFactor = smoothstep(shineWidth, 0.0f, dist);
    float3 shineColor = float3(gMaterial.shineColor.x, gMaterial.shineColor.y, gMaterial.shineColor.z) * shineFactor * shineIntensity;

    PixelShaderOutput output;
    
    if (gMaterial.enableShine)
    {
        output.color.rgb = finalColor.rgb + shineColor;
    }
    else
    {
        output.color.rgb = finalColor.rgb;
    }
    
    output.color.a = finalColor.a;

    if (output.color.a == 0.0)
    {
        discard;
    }

    return output;
}