struct Particle
{
	float3 life_time;		// [0]:cur-life-time		[1]: duration-time		[2]: pre-tex-clip-change-time
	uint   tex_index;
	float4 velocity;
	float4 position;
};
struct ParticleCounters
{
	uint dead_count;
	uint alive_count[2];		// 0: pre_sim	1: post_sim
	uint emit_count;

	uint simulate_count;
	uint render_count;
	uint2 pad;
};
struct GpuEmitParam
{
	float4	particle_color;

	float3 	position;
	uint 	max_particles;
	
	uint	emit_direction_type;
	float3	direction;

	float	direction_spread_percent;
	float 	min_init_speed;
	float	max_init_speed;
	float	min_life_time;

	float	max_life_time;		
	float3 	box_size;

	int		emit_shape;
	float	sphere_radius;	
	uint2	tex_rows_cols;

	uint	tex_time_sampling_type;
	float3	pad;
};
struct GpuSimulateParam
{
	float delta_time;
	float3 gravity;

	uint	tex_time_sampling_type;
	uint2	tex_rows_cols;
	float	tex_frames_per_sec;
};
struct GpuCullingParam
{
	float4x4 view_matrix;
	float4x4 proj_matrix;
};
struct SortInfo
{
	uint particle_index;
	float particle_depth;
};
struct GpuSortParam
{
	uint sort_level;
	uint descend_mask;
	uint matrix_width;
	uint matrix_height;
};
struct GpuRenderParam
{
	float4x4 view_matrix;
	float4x4 proj_matrix;
	uint2	tex_rows_cols;
	uint2	pad;
};

struct DispatchArgs
{
	uint dispatch_num_x;
	uint dispatch_num_y;
	uint dispatch_num_z;
};

struct ParticleDrawArgs
{
	uint count;
	uint instance_count;
	uint first;
	uint base_instance;
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

