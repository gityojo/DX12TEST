#include "Struct.hlsli"

float4 PS(VertexOut vout) : SV_Target
{
	return vout.Color;
}
