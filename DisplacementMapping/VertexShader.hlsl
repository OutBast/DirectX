#include "Header.hlsli"

VS_CONTROL_POINT_OUTPUT main(AppData IN)
{
	VS_CONTROL_POINT_OUTPUT OUT;

	matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));
	OUT.position = mul(mvp, float4(IN.position, 1.0f));
	OUT.color = float4(IN.color, 1.0f);

	return OUT;
}