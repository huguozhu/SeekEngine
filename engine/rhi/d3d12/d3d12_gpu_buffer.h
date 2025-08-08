#pragma once
#include "rhi/base/rhi_gpu_buffer.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/d3d12/d3d12_resource.h"
#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_gpu_descriptor_allocator.h"
#include "rhi/d3d12/d3d12_gpu_memory_allocator.h"
#include "rhi/d3d12/d3d12_render_view.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * D3D12GpuBuffer
 *******************************************************************************/
class D3D12GpuBuffer : public RHIGpuBuffer, public D3D12Resource
{
public:
    D3D12GpuBuffer(Context* context, uint32_t size, ResourceFlags flags, uint32_t structure_stride = 0);
    virtual ~D3D12GpuBuffer();

    D3D12SrvPtr const& GetD3DSrv(PixelFormat format, uint32_t first_elem, uint32_t num_elems);
    D3D12RtvPtr const& GetD3DRtv(PixelFormat format, uint32_t first_elem, uint32_t num_elems);
    D3D12UavPtr const& GetD3DUav(PixelFormat format, uint32_t first_elem, uint32_t num_elems);

    virtual SResult Create(RHIGpuBufferData* buffer_data) override;
    virtual SResult Update(RHIGpuBufferData* buffer_data) override;
    virtual SResult CopyBack(BufferPtr buffer, int start=0, int length=-1) override;

    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress() const { return m_GpuVAddr; }

protected:
    D3D12GpuMemoryBlock m_GpuMemoryBlock;
    uint32_t m_iCounterOffset = 0;
    D3D12_GPU_VIRTUAL_ADDRESS m_GpuVAddr;

    std::unordered_map<size_t, D3D12SrvPtr> m_vD3dSrvs;
    std::unordered_map<size_t, D3D12RtvPtr> m_vD3dRtvs;
    std::unordered_map<size_t, D3D12UavPtr> m_vD3dUavs;

};
using D3D12GpuBufferPtr = std::shared_ptr<D3D12GpuBuffer>;

/*****************************************************************************
 D3D12VertexBuffer
******************************************************************************/
class D3D12VertexBuffer : public D3D12GpuBuffer
{
public:
    D3D12VertexBuffer(Context* context, uint32_t size);
};
using D3D12VertexBufferPtr = std::shared_ptr<D3D12VertexBuffer>;


/******************************************************************************
 * D3D12IndexBuffer
 *******************************************************************************/
class D3D12IndexBuffer : public D3D12GpuBuffer
{
public:
    D3D12IndexBuffer(Context* context, uint32_t size);
};
using D3D12IndexBufferPtr = std::shared_ptr<D3D12IndexBuffer>;


/******************************************************************************
 * D3D12ConstantBuffer
 *******************************************************************************/
class D3D12ConstantBuffer : public D3D12GpuBuffer
{
public:
    D3D12ConstantBuffer(Context* context, uint32_t size, ResourceFlags flags);
};
using D3D12ConstantBufferPtr = std::shared_ptr<D3D12ConstantBuffer>;



SEEK_NAMESPACE_END
