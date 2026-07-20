#include "Particle.hlsli"

struct EmitterSphere
{
    float3 translate;
    float radius;
    uint count;
    float frequency;
    float frequencyTime;
    uint emit;
    uint seed;
};

float3 rand3dTo3d(float3 p)
{
    p = frac(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return frac((p.xxy + p.yzz) * p.zyx);
}

class RandomGenerator
{
    float3 seed;
    float3 Generate3d()
    {
        seed = rand3dTo3d(seed);
        return seed;
    }
    float Generate1d()
    {
        seed = rand3dTo3d(seed);
        return seed.x;
    }
};

static const uint kMaxParticles = 1024;

ConstantBuffer<EmitterSphere> gEmitter : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int> gFreeCounter : register(u1);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (gEmitter.emit == 0)
    {
        return;
    }

    for (uint countIndex = 0; countIndex < gEmitter.count; ++countIndex)
    {
        int freeListIndex;
        InterlockedAdd(gFreeCounter[0], -1, freeListIndex);

        if (0 <= freeListIndex && freeListIndex < kMaxParticles)
        {
            uint particleIndex = gFreeCounter[freeListIndex];
            
            // countIndexごとに異なる乱数になるようseedを分ける
            RandomGenerator generator;
            generator.seed = float3(particleIndex, gEmitter.seed, gPerFrame.time) * 0.6180339887f + float(countIndex);

            float3 randomDir = normalize(generator.Generate3d() * 2.0f - 1.0f);
            float randomRadius = generator.Generate1d() * gEmitter.radius;

            // Particleの初期化
            gParticles[particleIndex].translate = gEmitter.translate + (randomDir * randomRadius);
            gParticles[particleIndex].scale = float3(0.3f, 0.3f, 0.3f);
            gParticles[particleIndex].velocity = randomDir * (1.0f + generator.Generate1d());
            gParticles[particleIndex].lifeTime = 1.0f + generator.Generate1d() * 2.0f;
            gParticles[particleIndex].currentTime = 0.0f;
            gParticles[particleIndex].color = float4(generator.Generate3d(), 1.0f);
        }
        else
        {
            InterlockedAdd(gFreeCounter[0], 1);
            break;
        }
    }
}