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

struct LiquidGlassLighting
{	
	float shape_edge_size;
	float3 tint_color;	
	
	float ior;	
	float fresnel_scale;	
	
	float dispersion; // e.g. 0.002 - 0.02 (small)
	float height_min; // minimum height value (maps to edge)	
	
	float height_max; // maximum height value (maps to center)
	float reference_thickness; // in pixels
	
	float sunlight_intensity;	
	float shadow_intensity;
	float3 sunlight_dir;
	float shadow_expansion;
	float2 shadow_offset;
};