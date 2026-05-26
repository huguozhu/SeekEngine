module ParticleCommon;

public struct Particle
{
	public float3 life_time;		// [0]:cur-life-time		[1]: duration-time		[2]: pre-tex-clip-change-time
	public uint   tex_index;
	public float4 velocity;
	public float4 position;
};
public struct ParticleCounters
{
	public uint dead_count;
	public uint alive_count[2];		// 0: pre_sim	1: post_sim
	public uint emit_count;

	public uint simulate_count;
	public uint render_count;
	public uint2 pad;
};
public struct GpuEmitParam
{
	public float3 	position;
	public uint 	max_particles;
	
	uint	emit_direction_type;
	float3	direction;

	float	direction_spread_percent;
	public float 	min_init_speed;
	float	max_init_speed;
	float	min_life_time;

	float	max_life_time;		
	public float3 	box_size;

	int		emit_shape;
	float	sphere_radius;	
	uint2	tex_rows_cols;

	uint	tex_time_sampling_type;
	float3	pad;
};
public struct GpuSimulateParam
{
	public float delta_time;
	public float3 gravity;

	uint	tex_time_sampling_type;
	uint2	tex_rows_cols;
	float	tex_frames_per_sec;
};
public struct GpuCullingParam
{
	public float4x4 view_matrix;
	public float4x4 proj_matrix;
};
public struct SortInfo
{
	public uint particle_index;
	public float particle_depth;
};
public struct GpuSortParam
{
	public uint sort_level;
	public uint descend_mask;
	public uint matrix_width;
	public uint matrix_height;
};
public struct GpuRenderParam
{
	public float4x4 view_matrix;
	public float4x4 proj_matrix;
	uint2	tex_rows_cols;
	uint2	pad;
};

public struct DispatchArgs
{
	public uint dispatch_num_x;
	public uint dispatch_num_y;
	public uint dispatch_num_z;
};

public struct ParticleDrawArgs
{
	public uint count;
	public uint instance_count;
	public uint first;
	public uint base_instance;
};

public static const int PARTICLE_CS_X_SIZE = 32;
public static const int BITONIC_BLOCK_SIZE = 512;
public static const int TRANSPOSE_BLOCK_SIZE = 16;
public static const int RANDOM_FLOAT_NUM = 512;
	
public static const int Emit_Direction_Type_Directional = 0;
public static const int Emit_Direction_Type_BiDirectional = 1;
public static const int Emit_Direction_Type_Uniform_2D = 2;
public static const int Emit_Direction_Type_Uniform_3D = 3;

public static const int Emit_Shape_Type_Sphere = 0;
public static const int Emit_Shape_Type_Box = 1;

public static const int Tex_Time_Sampling_Type_Play_Once = 0;
public static const int Tex_Time_Sampling_Type_Play_Loop = 1;
public static const int Tex_Time_Sampling_Type_Random_Still_Frame = 2;
public static const int Tex_Time_Sampling_Type_Random_Loop = 3;


public cbuffer CAliveIndices
{
	public uint pre_sim_index;
	public uint post_sim_index;
};
RWStructuredBuffer<ParticleCounters> particle_counters;
RWStructuredBuffer<uint>	dead_indices;
RWStructuredBuffer<uint>	alive_pre_simulate_indices;
RWStructuredBuffer<uint>	alive_post_simulate_indices;

public cbuffer CRandom_Floats
{
	public float4 random_floats[RANDOM_FLOAT_NUM/4];
}
public float _GetRandomFloat(uint particle_index, uint curent_float_index)
{
	public uint index = (particle_index + curent_float_index) % RANDOM_FLOAT_NUM;
	return random_floats[index/4][index%4];
}

#define GetRandomFloat() _GetRandomFloat(particle_index, cur_random_index++)
