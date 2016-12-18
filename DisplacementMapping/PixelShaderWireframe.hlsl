#include "Header.hlsli"

Texture2D diffuseTexture : register(t0);

float4 main(PixelShaderInput IN) : SV_TARGET
{
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}