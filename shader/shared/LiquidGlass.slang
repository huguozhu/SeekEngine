module LiquidGlass;


public static const int MAX_Num_Shapes = 6;
public enum class ShapeType : int
{
	Unknown = -1,
	Circle = 0,
	Ellipse,
	Round_Rectangle,
	Super_Ellipse
};

public struct SdfShape
{
	public ShapeType shape_type;
	/*
	 *	.x .y : all shape's x-axis & y-asix Stretch Coefficient
	 *	.z: 
			Round_Rectangle: 	corner radius
			Super_Ellipse: 		shape power [0, 10]
	 */
	public float3 shape_params;	
	public float2 center;		
	public float2 size;
};

public struct LiquidGlassParam
{
	public float width;
	public float height;	
	public float sdf_smooth_value;
	public int shape_count;
	
	// shapes
	public SdfShape shapes[MAX_Num_Shapes];
};

public struct LiquidGlassLighting
{	
	public float shape_edge_size;
	public float3 tint_color;	
	
	public float ior;	
	public float fresnel_scale;	
	
	public float dispersion; // e.g. 0.002 - 0.02 (small)
	public float height_min; // minimum height value (maps to edge)	
	
	public float height_max; // maximum height value (maps to center)
	public float reference_thickness; // in pixels
	
	public float sunlight_intensity;	
	public float shadow_intensity;
	public float3 sunlight_dir;
	public float shadow_expansion;
	public float2 shadow_offset;
};
