//#include "Header.hlsli"
//
//float4 main() : SV_TARGET
//{
//	return AmbientColor * AmbientIntensity;
//}

struct PixelShaderInput
{
	float4 color : COLOR;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
	return IN.color;
}