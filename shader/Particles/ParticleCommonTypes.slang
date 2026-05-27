module ParticleCommonTypes;

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
	
	public uint	emit_direction_type;
	public float3	direction;

	public float	direction_spread_percent;
	public float 	min_init_speed;
	public float	max_init_speed;
	public float	min_life_time;

	public float	max_life_time;		
	public float3 	box_size;

	public int		emit_shape;
	public float	sphere_radius;	
	public uint2	tex_rows_cols;

	public uint	tex_time_sampling_type;
	public float3	pad;
};
public struct GpuSimulateParam
{
	public float delta_time;
	public float3 gravity;

	public uint	tex_time_sampling_type;
	public uint2	tex_rows_cols;
	public float	tex_frames_per_sec;
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
	public uint2	tex_rows_cols;
	public uint2	pad;
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

#define PARTICLE_CS_X_SIZE		32
#define BITONIC_BLOCK_SIZE 		512
#define TRANSPOSE_BLOCK_SIZE 	16
#define RANDOM_FLOAT_NUM		512
	
#define Emit_Direction_Type_Directional		0
#define Emit_Direction_Type_BiDirectional	1
#define Emit_Direction_Type_Uniform_2D		2
#define Emit_Direction_Type_Uniform_3D		3

#define Emit_Shape_Type_Sphere 	0
#define Emit_Shape_Type_Box 	1

#define Tex_Time_Sampling_Type_Play_Once			0
#define Tex_Time_Sampling_Type_Play_Loop			1
#define Tex_Time_Sampling_Type_Random_Still_Frame	2
#define Tex_Time_Sampling_Type_Random_Loop			3

