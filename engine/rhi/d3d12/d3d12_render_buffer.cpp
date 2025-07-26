#include "rhi/d3d12/d3d12_render_buffer.h"
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
//D3D12SrvPtr const& D3D12RenderBuffer::GetD3DSrv(uint32_t first_elem, uint32_t num_elems)
//{
//	size_t hash_val = HashValue(first_elem);
//	HashCombine(hash_val, num_elems);
//
//	auto iter = m_vD3dSrvs.find(hash_val);
//	if (iter != m_vD3dSrvs.end())
//	{
//		return iter->second;
//	}
//	else
//	{
//		bool is_structured = m_iFlags & RESOURCE_FLAG_GPU_STRUCTURED;
//		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
//		desc.Format = is_structured ? DXGI_FORMAT_UNKNOWN : D3DCommonTranslate::TranslateToPlatformFormat(m_eformat);
//		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
//		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//		desc.Buffer.FirstElement =  (is_structured ? d3d_resource_offset_ / structure_byte_stride_ : 0) + first_elem;
//		desc.Buffer.NumElements = num_elems;
//		desc.Buffer.StructureByteStride = (access_hint_ & EAH_GPU_Structured) ? structure_byte_stride_ : 0;
//		desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
//
//		auto sr_view = MakeSharedPtr<D3D12Srv>(this, desc);
//		return m_vD3dSrvs.emplace(hash_val, sr_view).first->second;
//	}
//}
//D3D12RtvPtr const& D3D12RenderBuffer::GetD3DRtv(uint32_t first_elem, uint32_t num_elems)
//{
//
//}
//D3D12UavPtr const& D3D12RenderBuffer::GetD3DUav(uint32_t first_elem, uint32_t num_elems)
//{
//
//}
//
//
//SResult D3D12RenderBuffer::Create(RHIGpuBufferData* buffer_data)
//{
//    return S_Success;
//}
//
//SResult D3D12RenderBuffer::Update(RHIGpuBufferData* buffer_data)
//{
//
//    return S_Success;
//}
//
//SResult D3D12RenderBuffer::CopyBack(BufferPtr buffer, int start, int length)
//{    
//    return S_Success;
//}



/******************************************************************************
 * D3D11VertexBuffer
 *******************************************************************************/
//D3D11VertexBuffer::D3D11VertexBuffer(Context* context, uint32_t size)
//    :D3D11GpuBuffer(context, size, RESOURCE_FLAG_CPU_WRITE)
//{
//}
//SResult D3D11VertexBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
//{
//    D3D11GpuBuffer::FillBufferDesc(desc);
//    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//    return S_Success;
//}

/******************************************************************************
 * D3D11IndexBuffer
 *******************************************************************************/
//D3D11IndexBuffer::D3D11IndexBuffer(Context* context, uint32_t size)
//    :D3D11GpuBuffer(context, size, RESOURCE_FLAG_CPU_WRITE)
//{
//}
//SResult D3D11IndexBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
//{
//    D3D11GpuBuffer::FillBufferDesc(desc);
//    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//    return S_Success;
//}
//
///******************************************************************************
// * D3D11ConstantBuffer
// *******************************************************************************/
//D3D11ConstantBuffer::D3D11ConstantBuffer(Context* context, uint32_t size, ResourceFlags flags)
//    :D3D11GpuBuffer(context, size, flags)
//{
//}
//SResult D3D11ConstantBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
//{
//    D3D11GpuBuffer::FillBufferDesc(desc);
//    desc.ByteWidth = seek_alignup(m_iSize, 16);
//    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//    return S_Success;
//}
//
//
///******************************************************************************
// * D3D11StructuredBuffer
// *******************************************************************************/
//D3D11StructuredBuffer::D3D11StructuredBuffer(Context* context, uint32_t size, ResourceFlags flags, uint32_t structure_byte_stride)
//    :D3D11GpuBuffer(context, size, flags), m_iStructureByteStride(structure_byte_stride)
//{
//}
//SResult D3D11StructuredBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
//{
//    D3D11GpuBuffer::FillBufferDesc(desc);
//    desc.ByteWidth = seek_alignup(m_iSize, 16);
//    desc.MiscFlags |= (m_iFlags & RESOURCE_FLAG_GPU_WRITE ?
//        D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS : 
//        D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);
//    desc.StructureByteStride = m_iStructureByteStride;
//    return S_Success;
//}
//
//SResult D3D11StructuredBuffer::FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
//{
//    desc.Format = (m_iFlags & RESOURCE_FLAG_GPU_WRITE ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN);
//    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
//    desc.BufferEx.FirstElement = 0;
//    desc.BufferEx.NumElements = m_iSize / sizeof(uint32_t);
//    desc.BufferEx.Flags = (m_iFlags & RESOURCE_FLAG_GPU_WRITE ? D3D11_BUFFER_UAV_FLAG_RAW : 0);
//    return S_Success;
//}
//SResult D3D11StructuredBuffer::FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
//{
//    desc.Format = (m_iFlags & RESOURCE_FLAG_GPU_WRITE ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN);
//    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//    desc.Buffer.FirstElement = 0;
//    desc.Buffer.NumElements = m_iSize / sizeof(uint32_t);
//    desc.Buffer.Flags = (m_iFlags & RESOURCE_FLAG_GPU_WRITE ? D3D11_BUFFER_UAV_FLAG_RAW : 0);
//    return S_Success;
//}
//
///******************************************************************************
// * D3D11ByteAddressBuffer
// *******************************************************************************/
//D3D11ByteAddressBuffer::D3D11ByteAddressBuffer(Context* context, uint32_t size, ResourceFlags flags)
//    :D3D11GpuBuffer(context, size, flags)
//{
//}
//SResult D3D11ByteAddressBuffer::FillBufferDesc(D3D11_BUFFER_DESC& desc)
//{
//    D3D11GpuBuffer::FillBufferDesc(desc);
//    desc.ByteWidth = seek_alignup(m_iSize, 16);
//    desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
//    return S_Success;
//}
//
//SResult D3D11ByteAddressBuffer::FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
//{
//    desc.Format = DXGI_FORMAT_R32_TYPELESS;
//    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
//    desc.BufferEx.FirstElement = 0;
//    desc.BufferEx.NumElements = m_iSize / 4;
//    desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
//    return S_Success;
//}
//
//SResult D3D11ByteAddressBuffer::FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
//{
//    desc.Format = DXGI_FORMAT_R32_TYPELESS;
//    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//    desc.Buffer.FirstElement = 0;
//    desc.Buffer.NumElements = m_iSize / 4;
//    desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
//    return S_Success;
//}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
