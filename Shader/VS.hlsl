#include "Struct.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
	vout.Color = vin.Color;
	
	return vout;
}
