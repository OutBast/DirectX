#include "Header.hlsli"

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D specularTexture : register(t2);
SamplerState samplerLinear : register(s0);

float4 main(PixelShaderInput IN) : SV_TARGET
{
	float4 AmbientColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float AmbientIntensity = 0.1f;
	float3 DiffuseLightDirection = float3(1.0f, 1.0f, 0.0f);

	float3 vNormal = normalize((normalTexture.Sample(samplerLinear, IN.TextureCoord).rgb) * 2 - 1);

	float4 vBaseColor = diffuseTexture.Sample(samplerLinear, IN.TextureCoord);
	
	float diffuse = saturate(dot(vNormal, DiffuseLightDirection));

	float4 outputColor = (vBaseColor * diffuse) + (vBaseColor * AmbientColor * AmbientIntensity);

	outputColor.a = 1.0f;

	return outputColor;
}