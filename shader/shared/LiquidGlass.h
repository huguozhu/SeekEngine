#pragma once

#define Num_Shapes 2
enum class ShapeType : int
{
	Unknown = -1,
	Circle = 0,
	Ellipse,
	Round_Rectangle,
	Super_Ellipse
};

struct SdfShape
{
	ShapeType shape_type;
	float3 shape_params;
	float2 center;
	float2 size;
};

struct LiquidGlassParam
{
	float width;
	float height;
	float2 pad;
	
	// shape1
	SdfShape shapes[Num_Shapes];
};