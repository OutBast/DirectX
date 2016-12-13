cbuffer PerApplication : register(b0)
{
	matrix projectionMatrix;
}

cbuffer PerFrame : register(b1)
{
	matrix viewMatrix;
}

cbuffer PerObject : register(b2)
{
	matrix worldMatrix;
}

cbuffer TessellationFactorBuffer : register(b3)
{
	float4 factors;
}

struct AppData
{
	float3 position : POSITION;
	float3 color: COLOR;
};

struct VertexShaderOutput
{
	float4 color : COLOR;
	float4 position : SV_POSITION;
};

struct PixelShaderInput
{
	float4 color : COLOR;
};

// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
	//float3 position : WORLDPOS;
	// TODO: change/add other stuff

	float4 color : COLOR;
	float4 position : SV_POSITION;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float4 color : COLOR;
	float4 position : SV_POSITION;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
												  // TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 3

struct DS_OUTPUT
{
	float4 color : COLOR;
	float4 position  : SV_POSITION;
	// TODO: change/add other stuff
};
