#pragma once

#define Shape_Type_Circle	0
#define Shape_Type_Ellipse	1
#define Num_Shapes 2


struct SdfShape
{
	int shape_type;
	float3 pad;
	float2 center;
	float2 radius;
};

struct LiquidGlassParam
{
	float width;
	float height;
	float2 pad;

	// shape1
	SdfShape shapes[Num_Shapes];

	//float2 circle_center;
	//float  circle_radius;
	//float pad2;

	// shape2
	//float2 ellipse_center;
	//float2 ellipse_radius;

};