#include "rhi/d3d12/d3d12_mesh.h"
#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_context.h"
#include "rhi/d3d12/d3d12_translate.h"
#include "rhi/d3d12/d3d12_gpu_buffer.h"
#include "rhi/d3d_common/d3d_common_translate.h"
#include "kernel/context.h"
#include "math/hash.h"

SEEK_NAMESPACE_BEGIN
D3D12Mesh::D3D12Mesh(Context* context)
    :RHIMesh(context)
{
}
std::vector<D3D12_INPUT_ELEMENT_DESC> const& D3D12Mesh::InputElementDesc()
{
	if (m_vVertexElems.empty())
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> elems;
		elems.reserve(m_vVertexElems.size());

		for (uint32_t i = 0; i < this->NumVertexStream(); ++i)
		{
			std::vector<D3D12_INPUT_ELEMENT_DESC> stream_elems;
			VertexStream& vs = this->GetVertexStreamByIndex(i);
			D3D12Translate::TranslateVertexStream(stream_elems, i, m_vVertexStreams[i].offset, (bool)m_vVertexStreams[i].is_instance_stream, m_vVertexStreams[i].layouts);
			elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
		}
		m_vVertexElems.swap(elems);
	}
	return m_vVertexElems;
}
SResult D3D12Mesh::Active(RHIProgram* program)
{
	if (m_bDataDirty)
	{
		this->UpdateHashValue();
		m_bDataDirty = false;
	}

	D3D12Context& rc = (D3D12Context&)m_pContext->RHIContextInstance();
	ID3D12GraphicsCommandList* cmd_list = rc.D3DRenderCmdList();

	uint32_t const num_vertex_streams = this->NumVertexStream();

	for (uint32_t i = 0; i < num_vertex_streams; ++i)
	{
		VertexStream& vs = this->GetVertexStreamByIndex(i);
		D3D12GpuBuffer* d3dvb = (D3D12GpuBuffer*)(vs.render_buffer.get());
		m_vVertexBufferViews[i].BufferLocation = d3dvb->GpuVirtualAddress();
	}

	if (this->IsUseIndices())
	{
		D3D12GpuBuffer* ib = (D3D12GpuBuffer*)(this->GetIndexBuffer().get());
		m_IndexBufferView.BufferLocation = ib->GpuVirtualAddress();
	}

	if (num_vertex_streams != 0)
	{
		rc.IASetVertexBuffers(cmd_list, 0, m_vVertexBufferViews);
	}
	if (this->IsUseIndices())
	{
		rc.IASetIndexBuffer(cmd_list, m_IndexBufferView);
	}
	return S_Success;
}
size_t D3D12Mesh::PsoHashValue()
{
	if (m_bDataDirty)
	{
		this->UpdateHashValue();
		m_bDataDirty = false;
	}
	return m_iPsoHashValue;
}
void D3D12Mesh::UpdatePsoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc)
{
	if (m_bDataDirty)
	{
		this->UpdateHashValue();
		m_bDataDirty = false;
	}
	pso_desc.InputLayout.pInputElementDescs = this->InputElementDesc().data();
	pso_desc.InputLayout.NumElements = this->InputElementDesc().size();
	pso_desc.IBStripCutValue = m_eIndexBufferCutValue;
	pso_desc.PrimitiveTopologyType = m_ePrimitiveTopology;
}
void D3D12Mesh::UpdateHashValue()
{
	uint32_t const num_vertex_streams = this->NumVertexStream();

	m_vVertexBufferViews.resize(num_vertex_streams);
	for (uint32_t i = 0; i < num_vertex_streams; ++i)
	{
		VertexStream& vs = this->GetVertexStreamByIndex(i);
		D3D12GpuBuffer* d3dvb = (D3D12GpuBuffer*)(vs.render_buffer.get());
		m_vVertexBufferViews[i].BufferLocation = d3dvb->GpuVirtualAddress();
		m_vVertexBufferViews[i].SizeInBytes = d3dvb->GetSize();
		m_vVertexBufferViews[i].StrideInBytes = vs.stride;
	}

	if (this->IsUseIndices())
	{
		D3D12GpuBuffer* ib = (D3D12GpuBuffer*)(this->GetIndexBuffer().get());
		m_IndexBufferView.BufferLocation = ib->GpuVirtualAddress();
		m_IndexBufferView.SizeInBytes = ib->GetSize();
		m_IndexBufferView.Format = m_eIndexBufferType == IndexBufferType::UInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	}

	m_iPsoHashValue = 0;
	HashCombine(m_iPsoHashValue, 'I');
	auto const& input_elem_desc = this->InputElementDesc();
	if (!input_elem_desc.empty())
	{
		char const* p = reinterpret_cast<char const*>(&input_elem_desc[0]);
		HashRange(m_iPsoHashValue, p, p + input_elem_desc.size() * sizeof(input_elem_desc[0]));
	}
	HashCombine(m_iPsoHashValue, (uint32_t)m_eIndexBufferType);
	HashCombine(m_iPsoHashValue, (uint32_t)m_eTopoType);

	m_eIndexBufferCutValue = (IndexBufferType::UInt16 == m_eIndexBufferType)
		? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
	m_ePrimitiveTopology = D3D12Translate::TranslatePrimitiveTopologyType(m_eTopoType);
}

SEEK_NAMESPACE_END
