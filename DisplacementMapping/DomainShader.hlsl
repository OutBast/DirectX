#include "Header.hlsli"


[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	//Output.position = float4(
	//	patch[0].position*domain.x+patch[1].position*domain.y+patch[2].position*domain.z,1);

	Output.position = 
		domain.x * patch[0].position +
		domain.y * patch[1].position +
		domain.z * patch[2].position;

	Output.color =
		domain.x * patch[0].color +
		domain.y * patch[1].color +
		domain.z * patch[2].color;

	return Output;
}
