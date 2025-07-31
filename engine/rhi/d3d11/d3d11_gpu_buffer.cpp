#include "rhi/d3d11/d3d11_gpu_buffer.h"
#include "rhi/d3d11/d3d11_context.h"
#include "rhi/d3d11/d3d11_translate.h"
#include "rhi/d3d_common/d3d_common_translate.h"
#include "utils/buffer.h"
#include "kernel/context.h"
#include "math/math_utility.h"
#include "math/hash.h"

#define SEEK_MACRO_FILE_UID 10     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * D3D11GpuBuffer
 *******************************************************************************/
D3D11GpuBuffer::~D3D11GpuBuffer()
{
    m_pD3DBuffer.Reset();
    m_vD3dSrvs.clear();
    m_vD3dUavs.clear();
}
SResult D3D11GpuBuffer::Create(RHIGpuBufferData* buffer_data)
{
    D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_BUFFER_DESC desc = {};
    SResult ret = this->FillBufferDesc(desc);
    
    SEEK_RETIF_FAIL(ret);

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
    return S_Success;
}

SResult D3D11GpuBuffer::Update(RHIGpuBufferData* buffer_data)
{
    if (!(m_iFlags & RESOURCE_FLAG_CPU_WRITE))
        return ERR_INVALID_ARG;

    D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_MAPPED_SUBRESOURCE mapped_data = { 0 };
    if (FAILED(pDeviceContext->Map(m_pD3DBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data)))
        return ERR_INVALID_ARG;

    memcpy_s(mapped_data.pData, mapped_data.RowPitch, buffer_data->m_pData, buffer_data->m_iDataSize);
    pDeviceContext->Unmap(m_pD3DBuffer.Get(), 0);
    return S_Success;
}

SResult D3D11GpuBuffer::CopyBack(BufferPtr buffer, int start, int length)
{
    if (!m_pD3DBuffer)
        return ERR_INVALID_ARG;

    D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    ID3D11Device* pDevice = rc.GetD3D11Device();

    ID3D11BufferPtr pCopyRes = nullptr;
    bool bCPUReadAccess = m_d3dBufferDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ;
    if (!bCPUReadAccess)
    {
        D3D11_BUFFER_DESC desc = {};
        this->FillStageBufferDesc(desc);

        ID3D11BufferPtr buf;
        if (FAILED(pDevice->CreateBuffer(&desc, nullptr, buf.GetAddressOf())))
            return ERR_INVALID_ARG;
        pCopyRes = std::move(buf);
        pDeviceContext->CopyResource(pCopyRes.Get(), m_pD3DBuffer.Get());
    }
    else
        pCopyRes = m_pD3DBuffer;    

    D3D11_MAPPED_SUBRESOURCE mapped_data = { 0 };
    if (FAILED(pDeviceContext->Map(pCopyRes.Get(), 0, D3D11_MAP_READ, 0, &mapped_data)))
        return ERR_INVALID_ARG;

    if (start < 0 || start >= m_iSize)
    {
        LOG_ERROR("D3D11GpuBuffer::CopyBack() start(%d) >= m_iSize(%d) || start < 0", start, m_iSize);
        return ERR_INVALID_ARG;
    }

    if (length < 0 || length > m_iSize - start)
        length = m_iSize - start;

    buffer->Expand(length);
    memcpy_s(buffer->Data(), buffer->Size(), (uint8_t*)mapped_data.pData + start, length);
    pDeviceContext->Unmap(pCopyRes.Get(), 0);
    return S_Success;
}
ID3D11ShaderResourceView* D3D11GpuBuffer::GetD3DSrv(PixelFormat format, uint32_t elem_offset, uint32_t num_elems)
 {
    size_t hash_val = HashValue((uint32_t)format);
    HashCombine(hash_val, elem_offset);
    HashCombine(hash_val, num_elems);

    auto iter = m_vD3dSrvs.find(hash_val);
    if (iter != m_vD3dSrvs.end())
    {
        return iter->second.Get();
    }
    else
    {
        D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
        ID3D11Device* pDevice = rc.GetD3D11Device();

        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        if (m_iFlags & RESOURCE_FLAG_RAW)
            desc.Format = DXGI_FORMAT_R32_TYPELESS;
        else if (m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED)
            desc.Format = DXGI_FORMAT_UNKNOWN;
        else
            desc.Format = D3DCommonTranslate::TranslateToPlatformFormat(format);
        desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        desc.BufferEx.FirstElement = elem_offset;
        desc.BufferEx.NumElements = num_elems;
        if (m_iFlags & RESOURCE_FLAG_RAW)
            desc.BufferEx.Flags |= D3D11_BUFFEREX_SRV_FLAG_RAW;

        ID3D11ShaderResourceViewPtr d3d_srv;
        SEEK_THROW_IFFAIL(pDevice->CreateShaderResourceView(m_pD3DBuffer.Get(), &desc, d3d_srv.ReleaseAndGetAddressOf()));
        return m_vD3dSrvs.emplace(hash_val, std::move(d3d_srv)).first->second.Get();
    }
}
ID3D11UnorderedAccessView* D3D11GpuBuffer::GetD3DUav(PixelFormat format, uint32_t elem_offset, uint32_t num_elems)
{
    size_t hash_val = HashValue((uint32_t)format);
    HashCombine(hash_val, elem_offset);
    HashCombine(hash_val, num_elems);

    auto iter = m_vD3dUavs.find(hash_val);
    if (iter != m_vD3dUavs.end())
    {
        return iter->second.Get();
    }
    else
    {
        D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
        ID3D11Device* pDevice = rc.GetD3D11Device();

        D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
        if (m_iFlags & RESOURCE_FLAG_RAW)
            desc.Format = DXGI_FORMAT_R32_TYPELESS;
        else if (m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED)
            desc.Format = DXGI_FORMAT_UNKNOWN;
        else
            desc.Format = D3DCommonTranslate::TranslateToPlatformFormat(format);
        desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement = elem_offset;
        desc.Buffer.NumElements = num_elems;
        desc.Buffer.Flags = 0;
        if (m_iFlags & RESOURCE_FLAG_RAW)
            desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
        else if (m_iFlags & RESOURCE_FLAG_APPEND)
            desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
        else if (m_iFlags & RESOURCE_FLAG_COUNTER)
            desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;

        ID3D11UnorderedAccessViewPtr d3d_uav;
        SEEK_THROW_IFFAIL(pDevice->CreateUnorderedAccessView(m_pD3DBuffer.Get(), &desc, d3d_uav.ReleaseAndGetAddressOf()));
        return m_vD3dUavs.emplace(hash_val, std::move(d3d_uav)).first->second.Get();
    }
}
void D3D11GpuBuffer::FillStageBufferDesc(D3D11_BUFFER_DESC& desc)
{
    desc.ByteWidth = m_iSize;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;
    desc.StructureByteStride = m_iStructureStride;
}
SResult D3D11GpuBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
{
    desc.ByteWidth = m_iSize;
    desc.StructureByteStride = m_iStructureStride;
    D3D11_USAGE& usage = desc.Usage;
    UINT& bind_flags = desc.BindFlags;
    UINT& cpu_access_flags = desc.CPUAccessFlags;
    UINT& misc_flags = desc.MiscFlags;

    if ((RESOURCE_FLAG_CPU_WRITE & m_iFlags) && (RESOURCE_FLAG_GPU_WRITE & m_iFlags))
        usage = D3D11_USAGE_STAGING;
    else if (!(RESOURCE_FLAG_CPU_WRITE & m_iFlags) && !(RESOURCE_FLAG_GPU_WRITE & m_iFlags))
        usage = D3D11_USAGE_IMMUTABLE;
    else if (RESOURCE_FLAG_CPU_WRITE & m_iFlags)
        usage = D3D11_USAGE_DYNAMIC;
    else
        usage = D3D11_USAGE_DEFAULT;
   
    cpu_access_flags = 0;
    if (m_iFlags & RESOURCE_FLAG_CPU_READ)
    {
        cpu_access_flags |= D3D11_CPU_ACCESS_READ;
    }
    if (m_iFlags & RESOURCE_FLAG_CPU_WRITE)
    {
        cpu_access_flags |= D3D11_CPU_ACCESS_WRITE;
    }
    if (D3D11_USAGE_STAGING == usage)
    {
        bind_flags = 0;
    }
    else
    {
        bind_flags = (UINT)m_eType;
    }
    if (bind_flags != D3D11_BIND_CONSTANT_BUFFER)
    {
        if (m_iFlags & RESOURCE_FLAG_GPU_READ)
        {
            bind_flags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (m_iFlags & RESOURCE_FLAG_GPU_WRITE)
        {
            if (!((m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED) || (m_iFlags & RESOURCE_FLAG_UAV)))
            {
                bind_flags |= D3D11_BIND_STREAM_OUTPUT;
            }
        }
        if (m_iFlags & RESOURCE_FLAG_UAV)
        {
            bind_flags |= D3D11_BIND_UNORDERED_ACCESS;
        }
    }

    misc_flags = 0;
    if (m_iFlags & RESOURCE_FLAG_UAV || m_iFlags & RESOURCE_FLAG_RAW)
    {
        misc_flags |= (m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED)
            ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    }
    if (m_iFlags & RESOURCE_FLAG_DRAW_INDIRECT_ARGS)
    {
        misc_flags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
    }
    return S_Success;
}

SResult D3D11GpuBuffer::FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = m_iSize / 16;
    return S_Success;
}
SResult D3D11GpuBuffer::FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
{
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = 0;
    if (m_iFlags & RESOURCE_FLAG_APPEND)
        desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
    if (m_iFlags & RESOURCE_FLAG_COUNTER)
        desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;
    return S_Success;
}
/******************************************************************************
 * D3D11VertexBuffer
 *******************************************************************************/
D3D11VertexBuffer::D3D11VertexBuffer(Context* context, uint32_t size)
    :D3D11GpuBuffer(context, size, RESOURCE_FLAG_CPU_WRITE, GpuBufferType::VERTEX_BUFFER)
{
}

/******************************************************************************
 * D3D11IndexBuffer
 *******************************************************************************/
D3D11IndexBuffer::D3D11IndexBuffer(Context* context, uint32_t size)
    :D3D11GpuBuffer(context, size, RESOURCE_FLAG_CPU_WRITE, GpuBufferType::INDEX_BUFFER)
{
}

/******************************************************************************
 * D3D11ConstantBuffer
 *******************************************************************************/
D3D11ConstantBuffer::D3D11ConstantBuffer(Context* context, uint32_t size, ResourceFlags flags)
    :D3D11GpuBuffer(context, size, flags, GpuBufferType::CONSTANT_BUFFER)
{
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
