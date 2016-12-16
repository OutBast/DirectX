cbuffer PerApplication				: register(b0)
{
	matrix projectionMatrix;
}

cbuffer PerFrame					: register(b1)
{
	matrix viewMatrix;
}

cbuffer PerObject					: register(b2)
{
	matrix worldMatrix;
}

cbuffer TessellationFactorBuffer	: register(b3)
{
	float4 factors;
}

struct AppData
{
	float3 worldPosition			: POSITION;
	float3 normal					: NORMAL0;
	float2 TextureCoord				: TexCOORD0;
};

// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
	float4 position					: SV_POSITION;
	float3 worldPosition			: POSITION;
	float2 TextureCoord				: TexCOORD0;
	float3 normal					: TexCOORD1;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float3 worldPosition			: POSITION;
	float2 TextureCoord				: TexCOORD0;
	float3 normal					: TexCOORD1;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
												  // TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 3

struct DS_OUTPUT
{
	float4 position					: SV_POSITION;
	float3 worldPosition			: POSITION;
	float2 TextureCoord				: TexCOORD0;
};

struct PixelShaderInput
{
	float4 position					: SV_POSITION;
	float3 worldPosition			: POSITION;
	float2 TextureCoord				: TexCOORD0;
};