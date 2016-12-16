#include "Header.hlsli"

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D specularTexture : register(t2);
SamplerState samplerLinear : register(s0);

float4 main(PixelShaderInput IN) : SV_TARGET
{
	return diffuseTexture.Sample(samplerLinear, IN.TextureCoord);
}