#include "rhi/d3d12/d3d12_gpu_buffer.h"
#include "rhi/d3d12/d3d12_rhi_context.h"
#include "rhi/d3d12/d3d12_translate.h"
#include "rhi/d3d_common/d3d_common_translate.h"
#include "utils/buffer.h"
#include "kernel/context.h"
#include "math/math_utility.h"
#include "math/hash.h"

#define SEEK_MACRO_FILE_UID 10     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * D3D12RenderBuffer
 *******************************************************************************/
D3D12GpuBuffer::D3D12GpuBuffer(Context* context, uint32_t size, ResourceFlags flags, uint32_t structure_stride)
	: RHIGpuBuffer(context, size, flags, structure_stride), D3D12Resource((D3D12Context*)& context->RHIContextInstance())
{
	m_vCurrStates.resize(1, D3D12_RESOURCE_STATE_GENERIC_READ);
}
D3D12GpuBuffer::~D3D12GpuBuffer()
{
	m_vD3dSrvs.clear();
	m_vD3dRtvs.clear();
	m_vD3dUavs.clear();
}
D3D12SrvPtr const& D3D12GpuBuffer::GetD3DSrv(PixelFormat format, uint32_t first_elem, uint32_t num_elems)
{
	size_t hash_val = HashValue((uint32_t)format);
	HashCombine(hash_val, first_elem);
	HashCombine(hash_val, num_elems);

	auto iter = m_vD3dSrvs.find(hash_val);
	if (iter != m_vD3dSrvs.end())
	{
		return iter->second;
	}
	else
	{
		bool is_structured = m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED;
		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = is_structured ? DXGI_FORMAT_UNKNOWN : D3DCommonTranslate::TranslateToPlatformFormat(format);
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Buffer.FirstElement =  (is_structured ? m_iD3dResourceOffset / m_iStructureStride : 0) + first_elem;
		desc.Buffer.NumElements = num_elems;
		desc.Buffer.StructureByteStride = is_structured ? m_iStructureStride : 0;
		desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		auto srv = MakeSharedPtr<D3D12Srv>(m_pContext, this, desc);
		return m_vD3dSrvs.emplace(hash_val, srv).first->second;
	}
}
D3D12RtvPtr const& D3D12GpuBuffer::GetD3DRtv(PixelFormat format, uint32_t first_elem, uint32_t num_elems)
{
	size_t hash_val = HashValue((uint32_t)format);
	HashCombine(hash_val, first_elem);
	HashCombine(hash_val, num_elems);

	auto iter = m_vD3dRtvs.find(hash_val);
	if (iter != m_vD3dRtvs.end())
	{
		return iter->second;
	}
	else
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = D3DCommonTranslate::TranslateToPlatformFormat(format);
		desc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = (m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED ? m_iD3dResourceOffset / m_iStructureStride : 0) + first_elem;
		desc.Buffer.NumElements = num_elems;
		auto rtv = MakeSharedPtr<D3D12Rtv>(m_pContext, this, desc);
		return m_vD3dRtvs.emplace(hash_val, rtv).first->second;
	}
}
D3D12UavPtr const& D3D12GpuBuffer::GetD3DUav(PixelFormat format, uint32_t first_elem, uint32_t num_elems)
{
	size_t hash_val = HashValue((uint32_t)format);
	HashCombine(hash_val, first_elem);
	HashCombine(hash_val, num_elems);

	auto iter = m_vD3dUavs.find(hash_val);
	if (iter != m_vD3dUavs.end())
	{
		return iter->second;
	}
	else
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		if (m_iFlags & RESOURCE_FLAG_RAW)
		{
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.Buffer.StructureByteStride = 0;
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		}
		else if (m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED)
		{
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.Buffer.StructureByteStride = m_iStructureStride;
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		}
		else
		{
			desc.Format = D3DCommonTranslate::TranslateToPlatformFormat(format);
			desc.Buffer.StructureByteStride = 0;
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		}
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = (m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED ? m_iD3dResourceOffset / m_iStructureStride : 0) + first_elem;
		desc.Buffer.NumElements = num_elems;
		desc.Buffer.CounterOffsetInBytes = m_iCounterOffset;

		auto uav = MakeSharedPtr<D3D12Uav>(m_pContext, this, desc);
		return m_vD3dUavs.emplace(hash_val, uav).first->second;
	}
}

SResult D3D12GpuBuffer::Create(RHIGpuBufferData* buffer_data)
{
	D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
	ID3D12GraphicsCommandList* cmd_list = rc.D3DRenderCmdList();

	uint32_t total_size = m_iSize;
	if ((m_iFlags & RESOURCE_FLAG_GPU_WRITE) && !((m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED) || (m_iFlags & RESOURCE_FLAG_UAV)))
	{
		total_size = ((m_iSize + sizeof(uint64_t) - 1) & ~(sizeof(uint64_t) - 1)) + sizeof(uint64_t);
	}
	else if ((m_iFlags & RESOURCE_FLAG_UAV) && (m_iStructureStride != 0)
		&& ((m_iFlags & RESOURCE_FLAG_APPEND) || (m_iFlags & RESOURCE_FLAG_COUNTER)))
	{
		total_size = ((m_iSize + D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT - 1))
			+ sizeof(uint64_t);
	}

	if ((0 == m_iFlags) || (RESOURCE_FLAG_CPU_WRITE == m_iFlags) || ((RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_GPU_READ) == m_iFlags))
	{
		m_GpuMemoryBlock = rc.AllocUploadMemBlock(m_iSize, D3D12GpuMemoryAllocator::ConstantDataAligment);
		m_pD3dResource = m_GpuMemoryBlock.GetResource();
		m_iD3dResourceOffset = m_GpuMemoryBlock.GetOffset();
		m_GpuVAddr = m_GpuMemoryBlock.GetGpuAddress();

		if (buffer_data != nullptr)
		{
			memcpy(m_GpuMemoryBlock.GetCpuAddress(), buffer_data->m_pData, m_iSize);
		}

		m_vCurrStates[0] = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else
	{
		m_GpuMemoryBlock.Reset();

		D3D12_RESOURCE_STATES init_state;
		D3D12_HEAP_PROPERTIES heap_prop;
		if (RESOURCE_FLAG_CPU_READ == m_iFlags)
		{
			init_state = D3D12_RESOURCE_STATE_COPY_DEST;
			heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
		}
		else if ((0 == m_iFlags) || (m_iFlags & RESOURCE_FLAG_CPU_READ) || (m_iFlags & RESOURCE_FLAG_CPU_WRITE))
		{
			init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
			heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		}
		else
		{
			init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
			heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		}
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.CreationNodeMask = 0;
		heap_prop.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC res_desc;
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Alignment = 0;
		res_desc.Width = total_size;
		res_desc.Height = 1;
		res_desc.DepthOrArraySize = 1;
		res_desc.MipLevels = 1;
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.SampleDesc.Count = 1;
		res_desc.SampleDesc.Quality = 0;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if (m_iFlags & RESOURCE_FLAG_UAV)
		{
			res_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		SEEK_THROW_IFFAIL(rc.GetD3D12Device()->CreateCommittedResource(
			&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, init_state, nullptr, IID_PPV_ARGS(m_pD3dResource.ReleaseAndGetAddressOf())));

		m_iD3dResourceOffset = 0;
		m_GpuVAddr = m_pD3dResource->GetGPUVirtualAddress();

		m_vCurrStates[0] = init_state;

		if (buffer_data != nullptr)
		{
			auto upload_mem_block = rc.AllocUploadMemBlock(m_iSize, D3D12GpuMemoryAllocator::StructuredDataAligment);
			memcpy(upload_mem_block.GetCpuAddress(), buffer_data, m_iSize);

			{
				rc.ResetLoadCmd();
				ID3D12GraphicsCommandList* cmd_list = rc.D3DLoadCmdList();

				this->UpdateResourceBarrier(cmd_list, 0, D3D12_RESOURCE_STATE_COPY_DEST);
				rc.FlushResourceBarriers(cmd_list);

				cmd_list->CopyBufferRegion(
					m_pD3dResource.Get(), m_iD3dResourceOffset, upload_mem_block.GetResource(), upload_mem_block.GetOffset(), m_iSize);

				m_vCurrStates[0] = init_state;

				rc.CommitLoadCmd();
			}

			rc.DeallocUploadMemBlock(std::move(upload_mem_block));
		}
	}

	if ((m_iFlags & RESOURCE_FLAG_GPU_WRITE)
		&& !((m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED) || (m_iFlags & RESOURCE_FLAG_UAV)))
	{
		m_iCounterOffset = (m_iSize + sizeof(uint64_t) - 1) & ~(sizeof(uint64_t) - 1);
	}
	else if ((m_iFlags & RESOURCE_FLAG_UAV) && (m_iStructureStride != 0))
	{
		if ((m_iFlags & RESOURCE_FLAG_APPEND) || (m_iFlags & RESOURCE_FLAG_COUNTER))
		{
			m_iCounterOffset = (m_iSize + D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT - 1)
				& ~(D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT - 1);
		}
		else
		{
			m_iCounterOffset = 0;
		}
	}
	return S_Success;
}

SResult D3D12GpuBuffer::Update(RHIGpuBufferData* buffer_data)
{
    return S_Success;
}

SResult D3D12GpuBuffer::CopyBack(BufferPtr buffer, int start, int length)
{    
    return S_Success;
}



/******************************************************************************
 * D3D12VertexBuffer
 *******************************************************************************/
D3D12VertexBuffer::D3D12VertexBuffer(Context* context, uint32_t size)
    :D3D12GpuBuffer(context, size, RESOURCE_FLAG_CPU_WRITE)
{
	
}

/******************************************************************************
 * D3D12IndexBuffer
 *******************************************************************************/
D3D12IndexBuffer::D3D12IndexBuffer(Context* context, uint32_t size)
    :D3D12GpuBuffer(context, size, RESOURCE_FLAG_CPU_WRITE)
{
}

/******************************************************************************
 * D3D12ConstantBuffer
 *******************************************************************************/
D3D12ConstantBuffer::D3D12ConstantBuffer(Context* context, uint32_t size, ResourceFlags flags)
    :D3D12GpuBuffer(context, size, flags)
{
}



SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
