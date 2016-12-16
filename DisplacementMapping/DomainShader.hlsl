#include "Header.hlsli"

Texture2D displacementTexture : register(t0);
SamplerState samplerLinear : register(s0);

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	//Output.position = float4(
	//	patch[0].position*domain.x+patch[1].position*domain.y+patch[2].position*domain.z,1);

	//Output.position = 
	float3 vWorldPos =
		domain.x * patch[0].worldPosition +
		domain.y * patch[1].worldPosition +
		domain.z * patch[2].worldPosition;
	
	Output.TextureCoord =
		domain.x * patch[0].TextureCoord +
		domain.y * patch[1].TextureCoord +
		domain.z * patch[2].TextureCoord;

	float3 vNormal =
		domain.x * patch[0].normal +
		domain.y * patch[1].normal +
		domain.z * patch[2].normal;

	float fDisplacement = displacementTexture.SampleLevel(samplerLinear, Output.TextureCoord.xy, 0).r;

	fDisplacement *= factors.y; //scale
	fDisplacement += factors.z; //bias

	float3 direction = vNormal;

	vWorldPos += direction * fDisplacement;
	
	matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));

	Output.worldPosition = vWorldPos;

	Output.position = mul(mvp, float4(vWorldPos, 1.0));
	//Output.position = float4(vWorldPos, 1.0f);
/*
	Output.position =
		domain.x * patch[0].position +
		domain.y * patch[1].position +
		domain.z * patch[2].position;*/

	return Output;
}
