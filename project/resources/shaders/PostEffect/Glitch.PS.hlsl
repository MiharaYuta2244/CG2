Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

cbuffer GlitchParam : register(b0)
{
    float time;
    float intensity;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// 疑似乱数生成
float rand(float2 n)
{
    return frac(sin(dot(n, float2(12.9898, 78.233))) * 43758.5453);
}

float4 main(VSOutput input) : SV_TARGET
{
    // 強度が0の場合はそのまま元のピクセルを返す
    if (intensity <= 0.0f)
    {
        return tex.Sample(smp, input.uv);
    }

    float2 uv = input.uv;

    // ブロック状のノイズを生成してUVをズラす
    float splitAmount = (1.0 + sin(time * 6.0)) * 0.5;
    splitAmount *= 1.0 + sin(time * 16.0) * 0.5;
    splitAmount *= 1.0 + sin(time * 19.0) * 0.5;
    splitAmount *= 1.0 + sin(time * 27.0) * 0.5;
    splitAmount = pow(splitAmount, 3.0);
    splitAmount *= (0.05 * intensity); // ズレ幅を調整

    float blockyNoise = rand(float2(floor(uv.y * 20.0), floor(time * 10.0)));
    float2 offset = float2(blockyNoise * splitAmount, 0.0);

    // RGBの色ズレを表現
    float r = tex.Sample(smp, uv + offset).r;
    float g = tex.Sample(smp, uv).g;
    float b = tex.Sample(smp, uv - offset).b;

    // 全体的なホワイトノイズを薄く乗せる
    float noise = (rand(uv * time) * 2.0 - 1.0) * 0.1 * intensity;

    return float4(r + noise, g + noise, b + noise, 1.0);
}