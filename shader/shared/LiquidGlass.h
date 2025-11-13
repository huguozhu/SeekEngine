#pragma once

struct LiquidGlassParam
{
	// shape
	float shape_power;
	
	// refraction
	float refraction_power;
	float refraction_a;
	float refraction_b;
	float refraction_c;
	float refraction_d;
	
	// pad
	float2 pad;
	
#ifdef SEEK_CPP
    LiquidGlassParam()
    {
		// shape
		shape_power = 2.0;
	
		// refraction
		refraction_power = 1.0;
        refraction_a = 0.7;
        refraction_b = 2.3;
        refraction_c = 2.2;
		refraction_d = 3.0;
		
    }
#endif
};