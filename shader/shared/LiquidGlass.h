#pragma once

#define MAX_Num_Shapes 6
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
	/*
	 *	.x .y : all shape's x-axis & y-asix Stretch Coefficient
	 *	.z: 
			Round_Rectangle: 	corner radius
			Super_Ellipse: 		shape power [0, 10]
	 */
	float3 shape_params;	
	float2 center;		
	float2 size;
};

struct LiquidGlassParam
{
	float width;
	float height;	
	float sdf_smooth_value;
	int shape_count;
	
	// shapes
	SdfShape shapes[MAX_Num_Shapes];
};