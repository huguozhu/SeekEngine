#pragma once

#ifdef SEEK_CPP
typedef uint32_t uint;
#endif

#define WaterMarkType_Single 0
#define WaterMarkType_Repeat 1

struct WaterMarkDesc
{
	uint32_t src_width;
	uint32_t src_height;
	uint32_t dst_width;
	uint32_t dst_height;
	
    float radian;
    uint32_t normal_x;
    uint32_t normal_y;
    uint32_t watermark_type; 
}; 