#pragma once
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_gpu_descriptor_allocator.h"
#include "rhi/d3d12/d3d12_gpu_memory_allocator.h"
#include "rhi/d3d12/d3d12_render_view.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * D3D12RenderBuffer
 *******************************************************************************/
//class D3D12RenderBuffer : public RHIGpuBuffer
//{
//public:
//    D3D12RenderBuffer(Context* context, uint32_t size, ResourceFlags flags)
//        : RHIGpuBuffer(context, size, flags) {}
//    virtual ~D3D12RenderBuffer();
//
//    D3D12SrvPtr const& GetD3DSrv(uint32_t first_elem, uint32_t num_elems);
//    D3D12RtvPtr const& GetD3DRtv(uint32_t first_elem, uint32_t num_elems);
//    D3D12UavPtr const& GetD3DUav(uint32_t first_elem, uint32_t num_elems);
//
//
//    virtual SResult Create(RHIGpuBufferData* buffer_data) override;
//    virtual SResult Update(RHIGpuBufferData* buffer_data) override;
//    virtual SResult CopyBack(BufferPtr buffer, int start=0, int length=-1) override;
//
//
//protected:
//    D3D12GpuMemoryBlock gpu_mem_block_;
//    uint32_t counter_offset_{ 0 };
//    D3D12_GPU_VIRTUAL_ADDRESS gpu_vaddr_;
//
//    std::unordered_map<size_t, D3D12SrvPtr> m_vD3dSrvs;
//    std::unordered_map<size_t, D3D12RtvPtr> m_vD3dRtvs;
//    std::unordered_map<size_t, D3D12UavPtr> m_vD3dUavs;
//
//};
//using D3D12RenderBufferPtr = std::shared_ptr<D3D12RenderBuffer>;

/******************************************************************************
 * D3D11VertexBuffer
 *******************************************************************************/
//class D3D12VertexBuffer : public D3D12RHIGpuBuffer
//{
//public:
//    D3D12VertexBuffer(Context* context, uint32_t size);
//protected:
//    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc) override;
//};
//using D3D12VertexBufferPtr = std::shared_ptr<D3D12VertexBuffer>;
//
//
///******************************************************************************
// * D3D11IndexBuffer
// *******************************************************************************/
//class D3D12ndexBuffer : public D3D12RHIGpuBuffer
//{
//public:
//    D3D12ndexBuffer(Context* context, uint32_t size);
//protected:
//    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc) override;
//};
//using D3D12IndexBufferPtr = std::shared_ptr<D3D12ndexBuffer>;
//
//
///******************************************************************************
// * D3D11ConstantBuffer
// *******************************************************************************/
//class D3D12ConstantBuffer : public D3D12RHIGpuBuffer
//{
//public:
//    D3D12ConstantBuffer(Context* context, uint32_t size, ResourceFlags flags);
//protected:
//    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc) override;
//};
//using D3D12ConstantBufferPtr = std::shared_ptr<D3D12ConstantBuffer>;
//
//
///******************************************************************************
// * D3D11StructuredBuffer
// *******************************************************************************/
//class D3D12StructuredBuffer : public D3D12RHIGpuBuffer
//{
//public:
//    D3D12StructuredBuffer(Context* context, uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride);
//
//protected:
//    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc) override;
//    SResult FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) override;
//    SResult FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc) override;
//
//protected:
//    uint32_t m_iStructureByteStride = 0;
//};
//using D3D12StructuredBufferPtr = std::shared_ptr<D3D12StructuredBuffer>;
//
//
///******************************************************************************
// * D3D11ByteAddressBuffer
// *******************************************************************************/
//class D3D12ByteAddressBuffer : public D3D12RHIGpuBuffer
//{
//public:
//    D3D12ByteAddressBuffer(Context* context, uint32_t size, ResourceFlags flags);
//
//protected:
//    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc) override;
//    SResult FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) override;
//    SResult FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc) override;
//
//};
//using D3D12ByteAddressBufferPtr = std::shared_ptr<D3D12ByteAddressBuffer>;


SEEK_NAMESPACE_END
