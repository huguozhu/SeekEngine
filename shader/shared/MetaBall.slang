module MetaBall;


public struct Metaball
{
    public float3 position;
    public float radius;
    public float3 velocity;
    public float intensity;
#ifdef SEEK_CPP
    public Metaball()
        : position(0, 0, 0), velocity(0, 0, 0), radius(0.3f), intensity(1.0f)
    {
    }
#endif
};

public static const int MAX_METABALL_NUM = 6;
public struct MetaballParam
{
	public float width;
	public float height;
	
    public float surfaceThreshold;	
	public int metaballCount;
	
	public Metaball balls[MAX_METABALL_NUM];
};
