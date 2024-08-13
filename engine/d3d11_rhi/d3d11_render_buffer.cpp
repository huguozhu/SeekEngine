#include "rendering_d3d11/d3d11_render_buffer.h"
#include "rendering_d3d11/d3d11_render_context.h"
#include "rendering_d3d11/d3d11_translate.h"
#include "util/buffer.h"
#include "kernel/context.h"
#include "math/math_utility.h"

#define DVF_MACRO_FILE_UID 10     // this code is auto generated, don't touch it!!!

DVF_NAMESPACE_BEGIN

/******************************************************************************
 * D3D11RenderBuffer
 *******************************************************************************/
D3D11RenderBuffer::~D3D11RenderBuffer()
{
    m_pD3DBuffer.Reset();
    m_pD3DShaderResourceView.Reset();
    m_pD3DUnorderAccessView.Reset();
}

ID3D11Buffer* D3D11RenderBuffer::GetD3DBuffer()
{
    return m_pD3DBuffer.Get();
}

DVFResult D3D11RenderBuffer::Create(RenderBufferData* buffer_data)
{
    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_BUFFER_DESC desc;
    DVFResult ret = this->FillBufferDesc(desc);
    DVF_RETIF_FAIL(ret);

    D3D11_SUBRESOURCE_DATA* pData = nullptr;
    D3D11_SUBRESOURCE_DATA data = { 0 };
    if (buffer_data)
    {
        data.pSysMem = buffer_data->m_pData;
        data.SysMemPitch = buffer_data->m_iDataSize;
        data.SysMemSlicePitch = 0;
        pData = &data;
    }
    HRESULT hr = pDevice->CreateBuffer(&desc, pData, m_pD3DBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("Create D3D11 VertexBuffer Failed");
        return ERR_INVALID_ARG;
    }
    m_pD3DBuffer->GetDesc(&m_d3dBufferDesc);
    return DVF_Success;
}

DVFResult D3D11RenderBuffer::Update(RenderBufferData* buffer_data)
{
    if (!(m_iFlags & RESOURCE_FLAG_CPU_WRITE))
        return ERR_INVALID_ARG;

    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_MAPPED_SUBRESOURCE mapped_data = { 0 };
    if (FAILED(pDeviceContext->Map(m_pD3DBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data)))
        return ERR_INVALID_ARG;

    memcpy_s(mapped_data.pData, mapped_data.RowPitch, buffer_data->m_pData, buffer_data->m_iDataSize);
    pDeviceContext->Unmap(m_pD3DBuffer.Get(), 0);
    return DVF_Success;
}

DVFResult D3D11RenderBuffer::CopyBack(BufferPtr buffer, int start, int length)
{
    if (!(m_iFlags & RESOURCE_FLAG_COPY_BACK))
        return ERR_INVALID_ARG;

    if (!m_pD3DBuffer)
        return ERR_INVALID_ARG;

    bool bCPUReadAccess = m_d3dBufferDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ;
    if (!bCPUReadAccess)
        return ERR_INVALID_ARG;

    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_MAPPED_SUBRESOURCE mapped_data = { 0 };
    if (FAILED(pDeviceContext->Map(m_pD3DBuffer.Get(), 0, D3D11_MAP_READ, 0, &mapped_data)))
        return ERR_INVALID_ARG;

    if (start < 0 || start >= m_iSize)
    {
        LOG_ERROR("D3D11RenderBuffer::CopyBack() start(%d) >= m_iSize(%d) || start < 0", start, m_iSize);
        return ERR_INVALID_ARG;
    }

    if (length < 0 || length > m_iSize - start)
        length = m_iSize - start;

    buffer->Expand(length);
    memcpy_s(buffer->Data(), buffer->Size(), (uint8_t*)mapped_data.pData + start, length);
    pDeviceContext->Unmap(m_pD3DBuffer.Get(), 0);
    return DVF_Success;
}
ID3D11ShaderResourceView * D3D11RenderBuffer::GetD3DShaderResourceView()
 {
    if (m_pD3DShaderResourceView)
        return m_pD3DShaderResourceView.Get();

    if (!m_pD3DBuffer)
        return nullptr;

    D3D11RenderContext & rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    this->FillShaderResourceViewDesc(desc);
    HRESULT hr = rc.GetD3D11Device()->CreateShaderResourceView(m_pD3DBuffer.Get(), &desc, m_pD3DShaderResourceView.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("D3D11Texture::GetD3dShaderResourceView error");
        return nullptr;
    }
    return m_pD3DShaderResourceView.Get();
}
ID3D11UnorderedAccessView* D3D11RenderBuffer::GetD3DUnorderedAccessView()
{
    if (m_pD3DUnorderAccessView)
        return m_pD3DUnorderAccessView.Get();

    if (!m_pD3DBuffer)
        return nullptr;

    D3D11RenderContext& rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;

    this->FillUnorderedAccessViewDesc(desc);
    HRESULT hr = pDevice->CreateUnorderedAccessView(m_pD3DBuffer.Get(), &desc, m_pD3DUnorderAccessView.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("Create D3D11 UnorderedAccessView Failed");
        return nullptr;
    }
    return m_pD3DUnorderAccessView.Get();
}

DVFResult D3D11RenderBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
{
    UINT cpu_access_flags = 0;
    D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
    D3D11Translate::TranslateResourceFlagsToD3D11Foramt(m_iFlags, usage, cpu_access_flags);

    zm_memset_s(&desc, sizeof(D3D11_BUFFER_DESC), 0, sizeof(D3D11_BUFFER_DESC));
    desc.ByteWidth = m_iSize;
    desc.Usage = usage;
    desc.CPUAccessFlags = cpu_access_flags;
    desc.BindFlags |= (m_iFlags & RESOURCE_FLAG_SHADER_RESOURCE     ? D3D11_BIND_SHADER_RESOURCE : 0);  ;
    desc.BindFlags |= (m_iFlags & RESOURCE_FLAG_GPU_WRITE           ? D3D11_BIND_UNORDERED_ACCESS           : 0);
    desc.MiscFlags |= (m_iFlags & RESOURCE_FLAG_DRAW_INDIRECT_ARGS  ? D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS : 0);
    desc.StructureByteStride = 0;
    return DVF_Success;
}

DVFResult D3D11RenderBuffer::FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = m_iSize / 16;
    return DVF_Success;
}
DVFResult D3D11RenderBuffer::FillUnorderedAccessViewDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
{
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = 0;
    desc.Buffer.Flags = 0;
    return DVF_Success;
}
/******************************************************************************
 * D3D11VertexBuffer
 *******************************************************************************/
D3D11VertexBuffer::D3D11VertexBuffer(Context* context, uint32_t size, ResourceFlags flags)
    :D3D11RenderBuffer(context, size, flags | RESOURCE_FLAG_CPU_WRITE)
{
}
DVFResult D3D11VertexBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
{
    D3D11RenderBuffer::FillBufferDesc(desc);
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    return DVF_Success;
}

/******************************************************************************
 * D3D11IndexBuffer
 *******************************************************************************/
D3D11IndexBuffer::D3D11IndexBuffer(Context* context, uint32_t size, ResourceFlags flags)
    :D3D11RenderBuffer(context, size, flags | RESOURCE_FLAG_CPU_WRITE)
{
}
DVFResult D3D11IndexBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
{
    D3D11RenderBuffer::FillBufferDesc(desc);
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    return DVF_Success;
}

/******************************************************************************
 * D3D11ConstantBuffer
 *******************************************************************************/
D3D11ConstantBuffer::D3D11ConstantBuffer(Context* context, uint32_t size, ResourceFlags flags)
    :D3D11RenderBuffer(context, size, flags)
{
}
DVFResult D3D11ConstantBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
{
    D3D11RenderBuffer::FillBufferDesc(desc);
    desc.ByteWidth = dvf_alignup(m_iSize, 16);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    return DVF_Success;
}


/******************************************************************************
 * D3D11StructuredBuffer
 *******************************************************************************/
D3D11StructuredBuffer::D3D11StructuredBuffer(Context* context, uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride)
    :D3D11RenderBuffer(context, size, flags), m_iStructureByteStride(structure_byte_stride)
{
}
DVFResult D3D11StructuredBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
{
    D3D11RenderBuffer::FillBufferDesc(desc);
    desc.ByteWidth = dvf_alignup(m_iSize, 16);
    desc.MiscFlags |= (m_iFlags & RESOURCE_FLAG_GPU_WRITE ?
        D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS : 
        D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);
    desc.StructureByteStride = m_iStructureByteStride;
    return DVF_Success;
}

DVFResult D3D11StructuredBuffer::FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
    desc.Format = (m_iFlags & RESOURCE_FLAG_GPU_WRITE ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN);
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = 0;
    desc.BufferEx.NumElements = m_iSize / sizeof(uint32_t);
    desc.BufferEx.Flags = (m_iFlags & RESOURCE_FLAG_GPU_WRITE ? D3D11_BUFFER_UAV_FLAG_RAW : 0);
    return DVF_Success;
}
DVFResult D3D11StructuredBuffer::FillUnorderedAccessViewDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
{
    desc.Format = (m_iFlags & RESOURCE_FLAG_GPU_WRITE ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN);
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = m_iSize / sizeof(uint32_t);
    desc.Buffer.Flags = (m_iFlags & RESOURCE_FLAG_GPU_WRITE ? D3D11_BUFFER_UAV_FLAG_RAW : 0);
    return DVF_Success;
}

/******************************************************************************
 * D3D11ByteAddressBuffer
 *******************************************************************************/
D3D11ByteAddressBuffer::D3D11ByteAddressBuffer(Context* context, uint32_t size, ResourceFlags flags)
    :D3D11RenderBuffer(context, size, flags)
{
}
DVFResult D3D11ByteAddressBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
{
    D3D11RenderBuffer::FillBufferDesc(desc);
    desc.ByteWidth = dvf_alignup(m_iSize, 16);
    desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    return DVF_Success;
}

DVFResult D3D11ByteAddressBuffer::FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = 0;
    desc.BufferEx.NumElements = m_iSize / 4;
    desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
    return DVF_Success;
}

DVFResult D3D11ByteAddressBuffer::FillUnorderedAccessViewDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
{
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = m_iSize / 4;
    desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
    return DVF_Success;
}

DVF_NAMESPACE_END

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
