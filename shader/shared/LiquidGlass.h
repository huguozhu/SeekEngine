#pragma once

struct LiquidGlassParam
{
	float width;
	float height;

	// shape1
	float2 circle_center;
	float  circle_radius;
	
	// shape2
	float2 ellipse_center;
	float pad;
	float2 ellipse_radius;
	
	// refraction
	float refraction_power;
	float refraction_a;
	float refraction_b;
	
	float refraction_c;
	float refraction_d;
	
	// pad
	float pad_;
	
#ifdef SEEK_CPP
    LiquidGlassParam()
    {	
		// refraction
		refraction_power = 1.0;
        refraction_a = 0.7;
        refraction_b = 2.3;
        refraction_c = 2.2;
		refraction_d = 3.0;
    }
#endif
};