#pragma once

#ifdef SEEK_CPP
typedef uint32_t uint;
#endif

#define WaterMarkType_Single 0
#define WaterMarkType_Repeat 1

#define Gen_WaterMark_CS_SIZE 32

struct WaterMarkDesc
{
	uint32_t src_width;     // pixels
	uint32_t src_height;    // pixels
	
    float radian;
    uint32_t offset_x;      // pixels
    uint32_t offset_y;      // pixels
    uint32_t watermark_type; 
}; 