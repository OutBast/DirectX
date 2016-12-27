#include "Header.hlsli"

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// Insert code to compute Output here
	Output.EdgeTessFactor[0] = factors.x;
	Output.EdgeTessFactor[1] = factors.x;
	Output.EdgeTessFactor[2] = factors.x;
	Output.InsideTessFactor = factors.x; // e.g. could calculate dynamic tessellation factors instead

	return Output;
}

[domain("tri")]
[partitioning("pow2")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
//[maxtessfactor(14.0)]
HS_CONTROL_POINT_OUTPUT main(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONTROL_POINT_OUTPUT Output;

	// Insert code to compute Output here
	Output.worldPosition = ip[i].worldPosition;
	Output.TextureCoord = ip[i].TextureCoord;
	Output.normal = ip[i].normal;

	return Output;
}
