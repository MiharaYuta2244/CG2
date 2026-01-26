#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
    int enableFoging;
};

struct DirectionalLight
{
    float4 color; //!< ライトの色
    float3 direction; //!< ライトの向き
    float intensity; //!< 輝度
};

struct Camera
{
    float3 worldPosition;
};

struct FogParam
{
    float3 fogCenter;
    float radius;
    float3 fogColor;
    float fogIntensity;
};

struct TimeParam
{
    float time;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct PointLight
{
    float4 color; //ライトの色
    float3 position; //ライトの位置
    float intensity; //輝度
    float radius; //ライトの届く最大距離
    float decay; //減衰率
};

struct SpotLight
{
    float4 color; //ライトの色
    float3 position; //ライトの位置
    float intensity; //輝度
    float3 direction; //スポットライトの方向
    float distance; //ライトの届く最大距離
    float decay; //減衰率
    float cosAngle; //スポットライトの余弦
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<FogParam> gFogParam : register(b3);
ConstantBuffer<TimeParam> gTimeParam : register(b4);
ConstantBuffer<PointLight> gPointLight : register(b5);
ConstantBuffer<SpotLight> gSpotLight : register(b6);

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    // textureのa値が0のときにPixelを棄却
    if (textureColor.a < 0.5)
    {
        discard;
    }
    
    // Cameraへの方向を算出
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    
    float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
    float NDotH = dot(normalize(input.normal), halfVector);
    float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
    
    PixelShaderOutput output;
    // DirectionalLight計算
    float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
    float3 baseColor = (gMaterial.color.rgb * textureColor.rgb);
    
    // 拡散反射 directional
    float3 diffuseDirectionalLight = baseColor * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
    // 鏡面反射 directional
    float3 specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow;
        
    // ポイントライト計算
    float3 pointLightDirection = normalize(gPointLight.position - input.worldPosition);
    float pointNdotL = dot(normalize(input.normal), pointLightDirection);
    float pointCos = saturate(pointNdotL);
        
    // ポイントライトからの距離
    float distance = length(gPointLight.position - input.worldPosition);
    // 距離減衰（二乗則）
    float attenuation = pow(saturate(-distance / gPointLight.radius + 1.0), gPointLight.decay);
        
    // 拡散反射 point
    float3 diffusePointLight = baseColor * gPointLight.color.rgb * pointCos * 1.0f * attenuation;
        
    // PointLightの半ベクトル
    float3 pointHalfVector = normalize(pointLightDirection + toEye);
    float pointNDotH = dot(normalize(input.normal), pointHalfVector);
    float pointSpecularPow = pow(saturate(pointNDotH), gMaterial.shininess);
   
    // 鏡面反射 point
    float3 specularPointLight = baseColor * gPointLight.intensity * pointSpecularPow * attenuation;
        
    // スポットライト計算
    float3 spotLightDirection = normalize(gSpotLight.position - input.worldPosition);
    float spotNdotL = dot(normalize(input.normal), spotLightDirection);
    float spotCos = saturate(spotNdotL);
    
    // スポットライトからの距離
    float spotDistance = length(gSpotLight.position - input.worldPosition);
    // スポットライトの減衰
    float spotAttenuation = pow(saturate(1.0 - spotDistance / gSpotLight.distance), gSpotLight.decay);

    // スポットライトの角度減衰
    float spotDirectionCos = dot(-spotLightDirection, normalize(gSpotLight.direction));
    // cosAngleが0.707の場合、45度のコーンを形成
    float angleAttenuation = smoothstep(gSpotLight.cosAngle - 0.1, gSpotLight.cosAngle, spotDirectionCos);
 
    // 拡散反射 spot
    float3 diffuseSpotLight = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * spotCos * gSpotLight.intensity * spotAttenuation * angleAttenuation;
        
    // スポットライトの半ベクトル
    float3 spotHalfVector = normalize(spotLightDirection + toEye);
    float spotNDotH = dot(normalize(input.normal), spotHalfVector);
    float spotSpecularPow = pow(saturate(spotNDotH), gMaterial.shininess);
      
    // 鏡面反射 spot
    float3 specularSpotLight = gSpotLight.color.rgb * gSpotLight.intensity * spotSpecularPow * spotAttenuation * angleAttenuation;

    // Directionalの色
    float3 dirLightColor = diffuseDirectionalLight + specularDirectionalLight;
    
    // PointLightの色
    float3 pLightColor = diffusePointLight + specularPointLight;
    
    // SpotLightの色
    float3 sLightColor = diffuseSpotLight + specularSpotLight;

    // 最終色
    float3 lightFinalColor = dirLightColor + pLightColor + sLightColor;
    
    if (gMaterial.enableLighting)
    {
    // 拡散反射+鏡面反射
        output.color.rgb = dirLightColor;
    // アルファ
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    
    return output;
}