#pragma once
#include "rhi/base/rhi_gpu_buffer.h"
#include "rhi/d3d11/d3d11_predeclare.h"

#include "rhi/base/rhi_definition.h"

SEEK_NAMESPACE_BEGIN

enum class GpuBufferType : uint32_t
{
    COMMON_BUFFER   = 0,  
    VERTEX_BUFFER   = D3D11_BIND_VERTEX_BUFFER,
    INDEX_BUFFER    = D3D11_BIND_INDEX_BUFFER,
    CONSTANT_BUFFER = D3D11_BIND_CONSTANT_BUFFER,
};

/******************************************************************************
 * D3D11GpuBuffer
 *******************************************************************************/
class D3D11GpuBuffer : public RHIGpuBuffer
{
public:
    D3D11GpuBuffer(Context* context, uint32_t size, ResourceFlags flags, GpuBufferType type, uint32_t structure_stride = 0)
        : RHIGpuBuffer(context, size, flags, structure_stride), m_pD3DBuffer(nullptr), m_eType(type) {}
    D3D11GpuBuffer(Context* context, uint32_t size, ResourceFlags flags, ID3D11Buffer* resource, uint32_t structure_stride = 0)
        : RHIGpuBuffer(context, size, flags, structure_stride), m_pD3DBuffer(resource) {}
    virtual ~D3D11GpuBuffer();

    SResult Create(RHIGpuBufferData* buffer_data) override;
    SResult Update(RHIGpuBufferData* buffer_data) override;
    SResult CopyBack(BufferPtr buffer, int start=0, int length=-1) override;

    ID3D11Buffer* GetD3DBuffer() { return m_pD3DBuffer.Get(); }

    ID3D11ShaderResourceView* GetD3DSrv(PixelFormat format, uint32_t elem_offset, uint32_t num_elems);
    ID3D11UnorderedAccessView* GetD3DUav(PixelFormat format, uint32_t elem_offset, uint32_t num_elems);

protected:
    void FillStageBufferDesc(D3D11_BUFFER_DESC& desc);
    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc);    
    SResult FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
    SResult FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc);
    

protected:
    ID3D11BufferPtr m_pD3DBuffer = nullptr;
    GpuBufferType m_eType = GpuBufferType::COMMON_BUFFER;
    D3D11_BUFFER_DESC m_d3dBufferDesc = {};
    
    std::unordered_map<size_t, ID3D11ShaderResourceViewPtr> m_vD3dSrvs;
    std::unordered_map<size_t, ID3D11UnorderedAccessViewPtr> m_vD3dUavs;
};
using D3D11GpuBufferPtr = std::shared_ptr<D3D11GpuBuffer>;

/******************************************************************************
 * D3D11VertexBuffer
 *******************************************************************************/
class D3D11VertexBuffer : public D3D11GpuBuffer
{
public:
    D3D11VertexBuffer(Context* context, uint32_t size);
};
using D3D11VertexBufferPtr = std::shared_ptr<D3D11VertexBuffer>;


/******************************************************************************
 * D3D11IndexBuffer
 *******************************************************************************/
class D3D11IndexBuffer : public D3D11GpuBuffer
{
public:
    D3D11IndexBuffer(Context* context, uint32_t size);
};
using D3D11IndexBufferPtr = std::shared_ptr<D3D11IndexBuffer>;


/******************************************************************************
 * D3D11ConstantBuffer
 *******************************************************************************/
class D3D11ConstantBuffer : public D3D11GpuBuffer
{
public:
    D3D11ConstantBuffer(Context* context, uint32_t size, ResourceFlags flags);
};
using D3D11ConstantBufferPtr = std::shared_ptr<D3D11ConstantBuffer>;



SEEK_NAMESPACE_END
