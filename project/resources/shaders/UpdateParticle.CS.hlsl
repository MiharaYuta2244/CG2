#include "Particle.hlsli"

static const uint kMaxParticles = 1024;
RWStructuredBuffer<Particle> gParticles : register(u0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<int> gFreeList : register(u2);
[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;
    // alphaが0のparticleは死んでいるとみなして更新しない
    if (particleIndex < kMaxParticles)
    {
        if (gParticles[particleIndex].color.a != 0)
        {
            gParticles[particleIndex].translate += gParticles[particleIndex].velocity;
            gParticles[particleIndex].currentTime += gPerFrame.deltaTime;
            float alpha = 1.0f - (gParticles[particleIndex].currentTime / gParticles[particleIndex].lifeTime);
            gParticles[particleIndex].color.a = saturate(alpha);
        }
        else
        {
            gParticles[particleIndex].scale = float3(0.0f, 0.0f, 0.0f);
            int freeListIndex;
            InterlockedAdd(gFreeListIndex[0], 1, freeListIndex);
            
            if ((freeListIndex + 1) < kMaxParticles)
            {
                gFreeList[freeListIndex + 1] = particleIndex;
            }
            else
            {
                InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
            }
        }
    }
}