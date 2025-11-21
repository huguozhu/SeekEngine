#pragma once

#define Num_Shapes 2
enum class ShapeType : int
{
	Circle = 0,
	Ellipse,
};

struct SdfShape
{
	ShapeType shape_type;
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
};