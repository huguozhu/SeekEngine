#include "rhi/d3d12/d3d12_resource.h"
#include "rhi/d3d12/d3d12_context.h"
#include "kernel/context.h"
#include <algorithm>
SEEK_NAMESPACE_BEGIN

D3D12Resource::D3D12Resource(D3D12Context* rhi_context)
	:m_pRHIContext(rhi_context)
{
}

D3D12Resource::~D3D12Resource()
{
	m_pRHIContext->AddStallResource(m_pD3dResource);
}

void D3D12Resource::UpdateResourceBarrier(ID3D12GraphicsCommandList* cmd_list, uint32_t sub_res, D3D12_RESOURCE_STATES target_state)
{
	if (!m_pD3dResource)
		return;

	D3D12Context& rc = *m_pRHIContext;

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	
	bool state_changed = false;
	if (sub_res == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
	{
		auto const first_state = m_vCurrStates[0];
		bool const same_state = std::all_of(m_vCurrStates.begin(), m_vCurrStates.end(), 
			[first_state](D3D12_RESOURCE_STATES state) { return state == first_state; });
		if (same_state)
		{
			if (m_vCurrStates[0] != target_state)
			{
				barrier.Transition.pResource = m_pD3dResource.Get();
				barrier.Transition.StateBefore = m_vCurrStates[0];
				barrier.Transition.StateAfter = target_state;
				barrier.Transition.Subresource = sub_res;
				std::fill(m_vCurrStates.begin(), m_vCurrStates.end(), target_state);

				rc.AddResourceBarrier(cmd_list, std::span<D3D12_RESOURCE_BARRIER, 1>(&barrier, 1));

				state_changed = true;
			}
		}
		else
		{
			for (uint32_t i = 0; i < m_vCurrStates.size(); ++i)
			{
				if (m_vCurrStates[i] != target_state)
				{
					barrier.Transition.pResource = m_pD3dResource.Get();
					barrier.Transition.StateBefore = m_vCurrStates[i];
					barrier.Transition.StateAfter = target_state;
					barrier.Transition.Subresource = i;
					m_vCurrStates[i] = target_state;

					rc.AddResourceBarrier(cmd_list, std::span<D3D12_RESOURCE_BARRIER, 1>(&barrier, 1));

					state_changed = true;
				}
			}
		}
	}
	else
	{
		if (m_vCurrStates[sub_res] != target_state)
		{
			barrier.Transition.pResource = m_pD3dResource.Get();
			barrier.Transition.StateBefore = m_vCurrStates[sub_res];
			barrier.Transition.StateAfter = target_state;
			barrier.Transition.Subresource = sub_res;
			m_vCurrStates[sub_res] = target_state;

			rc.AddResourceBarrier(cmd_list, std::span<D3D12_RESOURCE_BARRIER, 1>(&barrier, 1));
			state_changed = true;
		}
	}

	if (!state_changed && (target_state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
	{
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = m_pD3dResource.Get();

		rc.AddResourceBarrier(cmd_list, std::span<D3D12_RESOURCE_BARRIER, 1>(&barrier, 1));
	}
}



SEEK_NAMESPACE_END
