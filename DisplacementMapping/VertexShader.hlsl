#include "Header.hlsli"

VS_CONTROL_POINT_OUTPUT main(AppData IN)
{
	VS_CONTROL_POINT_OUTPUT OUT;

	matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));
	OUT.position = mul(mvp, float4(IN.worldPosition, 1.0f));
	OUT.worldPosition = IN.worldPosition;
	OUT.TextureCoord = IN.TextureCoord;
	OUT.normal = IN.normal;

	return OUT;
}