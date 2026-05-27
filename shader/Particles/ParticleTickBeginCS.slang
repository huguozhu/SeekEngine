//STAGE:cs
#include "ParticleCommon.slangh"
RWStructuredBuffer<Particle> particle_datas;
RWBuffer<uint> draw_args;
RWBuffer<uint> simulate_dispatch_args;
RWBuffer<uint> emit_dispatch_args;

cbuffer CTickBeginParam
{
	uint emit_count;
};

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	particle_counters[0].emit_count = min(emit_count, particle_counters[0].dead_count);
	particle_counters[0].simulate_count = particle_counters[0].alive_count[pre_sim_index] + particle_counters[0].emit_count;
	particle_counters[0].render_count = 0;
	particle_counters[0].alive_count[post_sim_index] = 0;

	draw_args[0] = 6;
	draw_args[1] = 0;
	draw_args[2] = 0;
	draw_args[3] = 0;

	emit_dispatch_args[0] = uint(ceil(float(particle_counters[0].emit_count) / float(PARTICLE_CS_X_SIZE)));
	emit_dispatch_args[1] = 1;
	emit_dispatch_args[2] = 1;

	simulate_dispatch_args[0] = uint(ceil(float(particle_counters[0].simulate_count) / float(PARTICLE_CS_X_SIZE)));
	simulate_dispatch_args[1] = 1;
	simulate_dispatch_args[2] = 1;
}