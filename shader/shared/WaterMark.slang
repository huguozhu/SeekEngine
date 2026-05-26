module WaterMark;


#ifdef SEEK_CPP
typedef uint32_t uint;
#endif

public static const int WaterMarkType_Single = 0;
public static const int WaterMarkType_Repeat = 1;

public static const int Gen_WaterMark_CS_SIZE = 32;

public struct WaterMarkDesc
{
	public uint32_t src_width;     // pixels
	public uint32_t src_height;    // pixels
	
    public float radian;
    public uint32_t offset_x;      // pixels
    public uint32_t offset_y;      // pixels
    public uint32_t watermark_type; 
}; 
