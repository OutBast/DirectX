#include "Header.hlsli"

float4 main() : SV_TARGET
{
	return AmbientColor * AmbientIntensity;
}