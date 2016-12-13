#include "Header.hlsli"

float4 main(PixelShaderInput IN) : SV_TARGET
{
	return float4(1, 1, 1, 1);
	//return IN.color;
}