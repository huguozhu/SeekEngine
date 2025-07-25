﻿#pragma once
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/d3d11/d3d11_predeclare.h"

#include "rhi/base/rhi_definition.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * D3D11RHIRenderBuffer
 *******************************************************************************/
class D3D11RHIRenderBuffer : public RHIRenderBuffer
{
public:
    D3D11RHIRenderBuffer(Context* context, uint32_t size, ResourceFlags flags)
        : RHIRenderBuffer(context, size, flags), m_pD3DBuffer(nullptr) {}
    D3D11RHIRenderBuffer(Context* context, uint32_t size, ResourceFlags flags, ID3D11Buffer* resource)
        : RHIRenderBuffer(context, size, flags), m_pD3DBuffer(resource) {}
    virtual ~D3D11RHIRenderBuffer();

    virtual SResult Create(RHIRenderBufferData* buffer_data) override;
    virtual SResult Update(RHIRenderBufferData* buffer_data) override;
    virtual SResult CopyBack(BufferPtr buffer, int start=0, int length=-1) override;

    ID3D11Buffer* GetD3DBuffer();
    ID3D11ShaderResourceView* GetD3DSrv();
    ID3D11UnorderedAccessView* GetD3DUav();

protected:
    virtual SResult FillBufferDesc(D3D11_BUFFER_DESC& desc);
    virtual SResult FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
    virtual SResult FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc);

protected:
    ID3D11BufferPtr m_pD3DBuffer = nullptr;
    D3D11_BUFFER_DESC m_d3dBufferDesc = {};
    ID3D11ShaderResourceViewPtr m_pD3DShaderResourceView = nullptr;
    ID3D11UnorderedAccessViewPtr m_pD3DUnorderAccessView = nullptr;
};
using D3D11RHIRenderBufferPtr = std::shared_ptr<D3D11RHIRenderBuffer>;

/******************************************************************************
 * D3D11VertexBuffer
 *******************************************************************************/
class D3D11VertexBuffer : public D3D11RHIRenderBuffer
{
public:
    D3D11VertexBuffer(Context* context, uint32_t size);
protected:
    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc) override;
};
using D3D11VertexBufferPtr = std::shared_ptr<D3D11VertexBuffer>;


/******************************************************************************
 * D3D11IndexBuffer
 *******************************************************************************/
class D3D11IndexBuffer : public D3D11RHIRenderBuffer
{
public:
    D3D11IndexBuffer(Context* context, uint32_t size);
protected:
    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc) override;
};
using D3D11IndexBufferPtr = std::shared_ptr<D3D11IndexBuffer>;


/******************************************************************************
 * D3D11ConstantBuffer
 *******************************************************************************/
class D3D11ConstantBuffer : public D3D11RHIRenderBuffer
{
public:
    D3D11ConstantBuffer(Context* context, uint32_t size, ResourceFlags flags);
protected:
    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc) override;
};
using D3D11ShaderBufferPtr = std::shared_ptr<D3D11ConstantBuffer>;


/******************************************************************************
 * D3D11StructuredBuffer
 *******************************************************************************/
class D3D11StructuredBuffer : public D3D11RHIRenderBuffer
{
public:
    D3D11StructuredBuffer(Context* context, uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride);

protected:
    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc) override;
    SResult FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) override;
    SResult FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc) override;

protected:
    uint32_t m_iStructureByteStride = 0;
};
using D3D11StructuredBufferPtr = std::shared_ptr<D3D11StructuredBuffer>;


/******************************************************************************
 * D3D11ByteAddressBuffer
 *******************************************************************************/
class D3D11ByteAddressBuffer : public D3D11RHIRenderBuffer
{
public:
    D3D11ByteAddressBuffer(Context* context, uint32_t size, ResourceFlags flags);

protected:
    SResult FillBufferDesc(D3D11_BUFFER_DESC& desc) override;
    SResult FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) override;
    SResult FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc) override;

};
using D3D11ByteAddressBufferPtr = std::shared_ptr<D3D11ByteAddressBuffer>;


SEEK_NAMESPACE_END
