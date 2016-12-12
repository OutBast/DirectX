float4x4 World;
float4x4 View;
float4x4 Projection;

float4 AmbientColor = float4(1, 1, 1, 1);
float AmbientIntensity = 0.1;

struct VertexShaderInput
{
	float4 vPosition : WORLDPOS;
};

struct VertexShaderOutput
{
	float4 vPosition : SV_POSITION0;
};