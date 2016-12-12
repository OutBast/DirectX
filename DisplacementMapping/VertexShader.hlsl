#include "Header.hlsli"

VertexShaderOutput main( VertexShaderInput input )
{
	VertexShaderOutput output;

	float4 worldPosition = mul(input.vPosition, World);
	float4 viewPosition = mul(worldPosition, View);

	output.vPosition = mul(viewPosition, Projection);

	return output;
}