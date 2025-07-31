#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_texture.h"
#include "rhi/d3d12/d3d12_fence.h"
#include "rhi/d3d12/d3d12_context.h"
#include "rhi/d3d_common/d3d_common_translate.h"
#include "kernel/context.h"
#include "math/math_utility.h"
#include "math/hash.h"
#include "utils/log.h"
#include "utils/error.h"
#include "utils/buffer.h"

SEEK_NAMESPACE_BEGIN
/******************************************************************************
* D3D12Texture
*******************************************************************************/
D3D12Texture::D3D12Texture(Context* context, const RHITexture::Desc& tex_desc)
    :RHITexture(context, tex_desc), D3D12Resource((D3D12Context*)&(context->RHIContextInstance()))
{
    m_eDxgiFormat = D3DCommonTranslate::TranslateToPlatformFormat(tex_desc.format);
}

D3D12SrvPtr const& D3D12Texture::GetD3DSrv()
{
    return this->GetD3DSrv(0, 1, 0, 1);
}
D3D12SrvPtr const& D3D12Texture::GetD3DSrv(uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_GPU_READ);

    size_t hash_val = HashValue(first_array_index);
    HashCombine(hash_val, array_size);
    HashCombine(hash_val, first_level);
    HashCombine(hash_val, num_levels);

    auto iter = m_mD3dSrvs.find(hash_val);
    if (iter != m_mD3dSrvs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        this->FillSrvDesc(desc, first_array_index, array_size, first_level, num_levels);
        D3D12SrvPtr pSrv = MakeSharedPtr<D3D12Srv>(m_pContext, this, desc);
        return m_mD3dSrvs.emplace(hash_val, pSrv).first->second;
    }
}
D3D12SrvPtr const& D3D12Texture::GetD3DSrv(uint32_t array_index, CubeFaceType face, uint32_t first_level, uint32_t num_levels)
{
    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_GPU_READ);

    size_t hash_val = HashValue(array_index);
    HashCombine(hash_val, 1);
    HashCombine(hash_val, (uint32_t)face);
    HashCombine(hash_val, first_level);
    HashCombine(hash_val, num_levels);

    auto iter = m_mD3dSrvs.find(hash_val);
    if (iter != m_mD3dSrvs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        this->FillSrvDesc(desc, array_index, face, first_level, num_levels);
        D3D12SrvPtr pSrv = MakeSharedPtr<D3D12Srv>(m_pContext, this, desc);
        return m_mD3dSrvs.emplace(hash_val, pSrv).first->second;
    }
}

D3D12RtvPtr const& D3D12Texture::GetD3DRtv()
{
    return this->GetD3DRtv(0, 1, 0);
}
D3D12RtvPtr const& D3D12Texture::GetD3DRtv(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{

    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_GPU_WRITE);
    SEEK_ASSERT(first_array_index < m_desc.num_array);
    SEEK_ASSERT(first_array_index + array_size <= m_desc.num_array);

    size_t hash_val = HashValue(first_array_index);
    HashCombine(hash_val, array_size);
    HashCombine(hash_val, mip_level);
    HashCombine(hash_val, 0);
    HashCombine(hash_val, 0);

    auto iter = m_mD3dRtvs.find(hash_val);
    if (iter != m_mD3dRtvs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        this->FillRtvDesc(desc, first_array_index, array_size, mip_level);
        D3D12RtvPtr pRtv = MakeSharedPtr<D3D12Rtv>(m_pContext, this, desc);
        return m_mD3dRtvs.emplace(hash_val, pRtv).first->second;
    }
}
D3D12RtvPtr const& D3D12Texture::GetD3DRtv(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_GPU_WRITE);
    SEEK_ASSERT(0 == array_index);

    size_t hash_val = HashValue(array_index);
    HashCombine(hash_val, 1);
    HashCombine(hash_val, mip_level);
    HashCombine(hash_val, first_slice);
    HashCombine(hash_val, num_slices);

    auto iter = m_mD3dRtvs.find(hash_val);
    if (iter != m_mD3dRtvs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        this->FillRtvDesc(desc, array_index, first_slice, num_slices, mip_level);
        D3D12RtvPtr pRtv = MakeSharedPtr<D3D12Rtv>(m_pContext, this, desc);
        return m_mD3dRtvs.emplace(hash_val, pRtv).first->second;
    }
}
D3D12RtvPtr const& D3D12Texture::GetD3DRtv(uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_GPU_WRITE);

    size_t hash_val = HashValue(array_index * 6 + (uint32_t)face);
    HashCombine(hash_val, 1);
    HashCombine(hash_val, mip_level);
    HashCombine(hash_val, 0);
    HashCombine(hash_val, 0);

    auto iter = m_mD3dRtvs.find(hash_val);
    if (iter != m_mD3dRtvs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        this->FillRtvDesc(desc, array_index, face, mip_level);
        D3D12RtvPtr pRtv = MakeSharedPtr<D3D12Rtv>(m_pContext, this, desc);
        return m_mD3dRtvs.emplace(hash_val, pRtv).first->second;
    }
}

D3D12DsvPtr const& D3D12Texture::GetD3DDsv()
{
    return this->GetD3DDsv(0, 1, 0);
}
D3D12DsvPtr const& D3D12Texture::GetD3DDsv(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_GPU_WRITE);
    SEEK_ASSERT(first_array_index < m_desc.num_array);
    SEEK_ASSERT(first_array_index + array_size <= m_desc.num_array);

    size_t hash_val = HashValue(first_array_index);
    HashCombine(hash_val, array_size);
    HashCombine(hash_val, mip_level);
    HashCombine(hash_val, 0);
    HashCombine(hash_val, 0);

    auto iter = m_mD3dDsvs.find(hash_val);
    if (iter != m_mD3dDsvs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
        this->FillDsvDesc(desc, first_array_index, array_size, mip_level);
        D3D12DsvPtr pDsv = MakeSharedPtr<D3D12Dsv>(m_pContext, this, desc);
        return m_mD3dDsvs.emplace(hash_val, pDsv).first->second;
    }
}
D3D12DsvPtr const& D3D12Texture::GetD3DDsv(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_GPU_WRITE);
    SEEK_ASSERT(0 == array_index);

    size_t hash_val = HashValue(array_index);
    HashCombine(hash_val, 1);
    HashCombine(hash_val, mip_level);
    HashCombine(hash_val, first_slice);
    HashCombine(hash_val, num_slices);

    auto iter = m_mD3dDsvs.find(hash_val);
    if (iter != m_mD3dDsvs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
        this->FillDsvDesc(desc, array_index, first_slice, num_slices, mip_level);
        D3D12DsvPtr pDsv = MakeSharedPtr<D3D12Dsv>(m_pContext, this, desc);
        return m_mD3dDsvs.emplace(hash_val, pDsv).first->second;
    }
}
D3D12DsvPtr const& D3D12Texture::GetD3DDsv(uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_GPU_WRITE);

    size_t hash_val = HashValue(array_index * 6 + (uint32_t)face);
    HashCombine(hash_val, 1);
    HashCombine(hash_val, mip_level);
    HashCombine(hash_val, 0);
    HashCombine(hash_val, 0);

    auto iter = m_mD3dDsvs.find(hash_val);
    if (iter != m_mD3dDsvs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
        this->FillDsvDesc(desc, array_index, face, mip_level);
        D3D12DsvPtr pDsv = MakeSharedPtr<D3D12Dsv>(m_pContext, this, desc);
        return m_mD3dDsvs.emplace(hash_val, pDsv).first->second;
    }
}

D3D12UavPtr const& D3D12Texture::GetD3DUav()
{
    return this->GetD3DUav(0, 1, 0);
}
D3D12UavPtr const& D3D12Texture::GetD3DUav(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_UAV);

    size_t hash_val = HashValue(first_array_index);
    HashCombine(hash_val, array_size);
    HashCombine(hash_val, mip_level);
    HashCombine(hash_val, 0);
    HashCombine(hash_val, 0);

    auto iter = m_mD3dUavs.find(hash_val);
    if (iter != m_mD3dUavs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
        this->FillUavDesc(desc, first_array_index, array_size, mip_level);
        D3D12UavPtr pUav = MakeSharedPtr<D3D12Uav>(m_pContext, this, desc);
        return m_mD3dUavs.emplace(hash_val, pUav).first->second;
    }
}
D3D12UavPtr const& D3D12Texture::GetD3DUav(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_UAV);

    size_t hash_val = HashValue(array_index);
    HashCombine(hash_val, 1);
    HashCombine(hash_val, mip_level);
    HashCombine(hash_val, first_slice);
    HashCombine(hash_val, num_slices);

    auto iter = m_mD3dUavs.find(hash_val);
    if (iter != m_mD3dUavs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
        this->FillUavDesc(desc, array_index, first_slice, num_slices, mip_level);
        D3D12UavPtr pUav = MakeSharedPtr<D3D12Uav>(m_pContext, this, desc);
        return m_mD3dUavs.emplace(hash_val, pUav).first->second;
    }
}
D3D12UavPtr const& D3D12Texture::GetD3DUav(uint32_t first_array_index, uint32_t array_size, CubeFaceType first_face, uint32_t num_faces, uint32_t mip_level)
{
    SEEK_ASSERT(m_desc.flags & RESOURCE_FLAG_UAV);

    size_t hash_val = HashValue(first_array_index * 6 + (uint32_t)first_face);
    HashCombine(hash_val, array_size * 6 + num_faces);
    HashCombine(hash_val, mip_level);
    HashCombine(hash_val, 0);
    HashCombine(hash_val, 0);

    auto iter = m_mD3dUavs.find(hash_val);
    if (iter != m_mD3dUavs.end())
    {
        return iter->second;
    }
    else
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
        this->FillUavDesc(desc, first_array_index, array_size, first_face, num_faces, mip_level);
        D3D12UavPtr pUav = MakeSharedPtr<D3D12Uav>(m_pContext, this, desc);
        return m_mD3dUavs.emplace(hash_val, pUav).first->second;
    }
}

SResult D3D12Texture::DoCreate(D3D12_RESOURCE_DIMENSION dim, uint32_t width, uint32_t height, uint32_t depth, 
    uint32_t array_size, std::span<BitmapBufferPtr> const& bitmap_datas)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12Device* pDevice = rc.GetD3D12Device();

    D3D12_RESOURCE_DESC tex_desc;
    tex_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    tex_desc.Alignment = 0;
    tex_desc.Width = width;
    tex_desc.Height = height;
    tex_desc.DepthOrArraySize = m_desc.num_array;
    tex_desc.MipLevels = m_desc.num_mips;
    tex_desc.Format = m_eDxgiFormat;
    tex_desc.SampleDesc.Count = m_desc.num_samples;
    tex_desc.SampleDesc.Quality = 0;
    tex_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    tex_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    switch (dim)
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:    tex_desc.DepthOrArraySize = array_size; break;
    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:    tex_desc.DepthOrArraySize = depth;
        break;
    }

    if (m_desc.flags & RESOURCE_FLAG_UAV)
        tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    if (m_desc.flags & RESOURCE_FLAG_GPU_WRITE)
    {
        if (Formatutil::IsDepthFormat(m_desc.format))
        {
            tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }
        else
        {
            tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
    }
    D3D12_HEAP_PROPERTIES heap_prop;
    heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
    heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_prop.CreationNodeMask = 0;
    heap_prop.VisibleNodeMask = 0;

    D3D12_RESOURCE_STATES init_state = D3D12_RESOURCE_STATE_COMMON;
    if (Formatutil::IsDepthFormat(m_desc.format) && (m_desc.flags & RESOURCE_FLAG_GPU_WRITE))
    {
        init_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        std::fill(m_vCurrStates.begin(), m_vCurrStates.end(), init_state);
    }
    pDevice->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
        &tex_desc, init_state, nullptr, IID_PPV_ARGS(m_pD3dResource.ReleaseAndGetAddressOf()));

    if (!bitmap_datas.empty())
    {
        uint32_t const num_subres = m_desc.num_array * m_desc.num_mips;
        std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(num_subres);
        std::vector<uint64_t> row_sizes_in_bytes(num_subres);
        std::vector<uint32_t> num_rows(num_subres);

        uint64_t required_size = 0;
        pDevice->GetCopyableFootprints(&tex_desc, 0, num_subres, 0, &layouts[0], &num_rows[0], &row_sizes_in_bytes[0], &required_size);

        D3D12GpuMemoryBlock upload_mem_block =
            rc.AllocUploadMemBlock(static_cast<uint32_t>(required_size), D3D12GpuMemoryAllocator::TextureDataAligment);
        ID3D12Resource* const& upload_buff = upload_mem_block.GetResource();
        uint32_t const upload_buff_offset = upload_mem_block.GetOffset();

        uint8_t* p = upload_mem_block.GetCpuAddress<uint8_t>();
        for (uint32_t i = 0; i < num_subres; ++i)
        {
            D3D12_SUBRESOURCE_DATA src_data;
            src_data.pData = bitmap_datas[i]->Data();
            src_data.RowPitch = bitmap_datas[i]->RowPitch();
            src_data.SlicePitch = bitmap_datas[i]->SlicePitch();

            D3D12_MEMCPY_DEST dest_data;
            dest_data.pData = p + layouts[i].Offset;
            dest_data.RowPitch = layouts[i].Footprint.RowPitch;
            dest_data.SlicePitch = layouts[i].Footprint.RowPitch * num_rows[i];

            for (UINT z = 0; z < layouts[i].Footprint.Depth; ++z)
            {
                uint8_t const* src_slice = static_cast<uint8_t const*>(src_data.pData) + src_data.SlicePitch * z;
                uint8_t* dest_slice = static_cast<uint8_t*>(dest_data.pData) + dest_data.SlicePitch * z;
                for (UINT y = 0; y < num_rows[i]; ++y)
                {
                    memcpy(dest_slice, src_slice, static_cast<size_t>(row_sizes_in_bytes[i]));

                    src_slice += src_data.RowPitch;
                    dest_slice += dest_data.RowPitch;
                }
            }
        }

        {
            rc.ResetLoadCmd();
            ID3D12GraphicsCommandList* cmd_list = rc.D3DLoadCmdList();

            this->UpdateResourceBarrier(cmd_list, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_DEST);
            rc.FlushResourceBarriers(cmd_list);

            for (uint32_t i = 0; i < num_subres; ++i)
            {
                layouts[i].Offset += upload_buff_offset;

                D3D12_TEXTURE_COPY_LOCATION src;
                src.pResource = upload_buff;
                src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                src.PlacedFootprint = layouts[i];

                D3D12_TEXTURE_COPY_LOCATION dst;
                dst.pResource = m_pD3dResource.Get();
                dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dst.SubresourceIndex = i;

                cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
            }

            rc.CommitLoadCmd();
        }
        rc.DeallocUploadMemBlock(std::move(upload_mem_block));
    }
    return S_Success;
}
/******************************************************************************
* D3D12Texture2D
*******************************************************************************/
D3D12Texture2D::D3D12Texture2D(Context* context, const RHITexture::Desc& tex_desc)
    :D3D12Texture(context, tex_desc)
{
}
D3D12Texture2D::D3D12Texture2D(Context* context, ID3D12ResourcePtr const& d3d_tex)
    :D3D12Texture(context, RHITexture::Desc{})
{
    D3D12_HEAP_PROPERTIES heap_prop;
    d3d_tex->GetHeapProperties(&heap_prop, nullptr);
    D3D12_RESOURCE_DESC const desc = d3d_tex->GetDesc();

    m_desc.num_mips = desc.MipLevels;
    m_desc.num_array = desc.DepthOrArraySize;
    m_desc.format = D3DCommonTranslate::TranslateFromPlatformFormat(desc.Format);
    m_desc.num_samples = desc.SampleDesc.Count;
    m_desc.width = (uint32_t)desc.Width;
    m_desc.height = (uint32_t)desc.Height;
    m_eDxgiFormat = desc.Format;

    m_desc.flags = 0;
    switch (heap_prop.Type)
    {
    case D3D12_HEAP_TYPE_DEFAULT:
        m_desc.flags |= RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE | RESOURCE_FLAG_UAV;
        break;

    case D3D12_HEAP_TYPE_UPLOAD:
        m_desc.flags |= RESOURCE_FLAG_CPU_READ | RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_GPU_READ;
        break;

    case D3D12_HEAP_TYPE_READBACK:
        m_desc.flags |= RESOURCE_FLAG_CPU_READ;
        break;

    case D3D12_HEAP_TYPE_CUSTOM:
        m_desc.flags |= RESOURCE_FLAG_CPU_READ | RESOURCE_FLAG_CPU_WRITE;
        break;

    default:
        ErrUnreachable("Invalid heap type");
    }

    m_pD3dResource = d3d_tex;

    m_vCurrStates.assign(m_desc.num_array * m_desc.num_mips, D3D12_RESOURCE_STATE_COMMON);
}
SResult D3D12Texture2D::Create(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    return this->DoCreate(D3D12_RESOURCE_DIMENSION_TEXTURE2D, m_desc.width, m_desc.height, m_desc.depth, 1, bitmap_datas);
}
SResult D3D12Texture2D::Update(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    return S_Success;
}
SResult D3D12Texture2D::Resolve()
{
    return S_Success;
}
SResult D3D12Texture2D::DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index, uint32_t mip_level, Rect<uint32_t>* rect)
{
    return S_Success;
}

void D3D12Texture2D::FillSrvDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    if (m_desc.num_array > 1)
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = first_array_index;
            desc.Texture2DMSArray.ArraySize = array_size;
        }
        else
        {
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MostDetailedMip = first_level;
            desc.Texture2DArray.MipLevels = num_levels;
            desc.Texture2DArray.FirstArraySlice = first_array_index;
            desc.Texture2DArray.ArraySize = array_size;
            desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;
            desc.Texture2DArray.ResourceMinLODClamp = 0;
        }
    }
    else
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MostDetailedMip = first_level;
            desc.Texture2D.MipLevels = num_levels;
            desc.Texture2D.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;
            desc.Texture2D.ResourceMinLODClamp = 0;
        }
    }
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
}
void D3D12Texture2D::FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    if (m_desc.num_array > 1)
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = first_array_index;
            desc.Texture2DMSArray.ArraySize = array_size;
        }
        else
        {
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice = mip_level;
            desc.Texture2DArray.FirstArraySlice = first_array_index;
            desc.Texture2DArray.ArraySize = array_size;
            desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;
        }
    }
    else
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = mip_level;
            desc.Texture2D.PlaneSlice = (desc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;
        }
    }
}
void D3D12Texture2D::FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_D16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;        break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_D32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    if (m_desc.num_array > 1)
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = first_array_index;
            desc.Texture2DMSArray.ArraySize = array_size;
        }
        else
        {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice = mip_level;
            desc.Texture2DArray.FirstArraySlice = first_array_index;
            desc.Texture2DArray.ArraySize = array_size;
        }
    }
    else
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = mip_level;
        }
    }
}
void D3D12Texture2D::FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    if (m_desc.num_array > 1)
    {
        desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.MipSlice = mip_level;
        desc.Texture2DArray.FirstArraySlice = first_array_index;
        desc.Texture2DArray.ArraySize = array_size;
        desc.Texture2DArray.PlaneSlice = 0;
    }
    else
    {
        desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = mip_level;
        desc.Texture2D.PlaneSlice = 0;
    }
}

/******************************************************************************
* D3D12TextureCube
*******************************************************************************/
D3D12TextureCube::D3D12TextureCube(Context* context, const RHITexture::Desc& tex_desc)
    :D3D12Texture(context, tex_desc)
{
}

SResult D3D12TextureCube::Create(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    return this->DoCreate(D3D12_RESOURCE_DIMENSION_TEXTURE2D, m_desc.width, m_desc.height, 1, m_desc.num_array * 6, bitmap_datas);
}
SResult D3D12TextureCube::Update(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    return S_Success; 
}
SResult D3D12TextureCube::DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index, uint32_t mip_level, Rect<uint32_t>* rect)
{
    return S_Success;
}
void D3D12TextureCube::FillSrvDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    desc.TextureCube.MostDetailedMip = first_level;
    desc.TextureCube.MipLevels = num_levels;
    desc.TextureCube.ResourceMinLODClamp = 0;
}
void D3D12TextureCube::FillSrvDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t first_level, uint32_t num_levels)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
    desc.Texture2DArray.MostDetailedMip = first_level;
    desc.Texture2DArray.MipLevels = num_levels;
    desc.Texture2DArray.FirstArraySlice = array_index * 6 + (uint32_t)face;
    desc.Texture2DArray.ArraySize = 1;
    desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_R24_UNORM_X8_TYPELESS) ? 1 : 0;
    desc.Texture2DArray.ResourceMinLODClamp = 0;
}
void D3D12TextureCube::FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    if (m_desc.num_samples > 1)
    {
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
    desc.Texture2DArray.ArraySize = array_size * 6;
    desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_R24_UNORM_X8_TYPELESS) ? 1 : 0;
}
void D3D12TextureCube::FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    if (m_desc.num_samples > 1)
    {
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = array_index * 6 + (uint32_t)face;
    desc.Texture2DArray.ArraySize = 1;
    desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_R24_UNORM_X8_TYPELESS) ? 1 : 0;
}
void D3D12TextureCube::FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_D16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;        break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_D32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.Flags = D3D12_DSV_FLAG_NONE;
    if (m_desc.num_samples > 1)
    {
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
    desc.Texture2DArray.ArraySize = array_size * 6;
}
void D3D12TextureCube::FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_D16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;        break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_D32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.Flags = D3D12_DSV_FLAG_NONE;
    if (m_desc.num_samples > 1)
    {
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = array_index * 6 + (uint32_t)face;
    desc.Texture2DArray.ArraySize = 1;
}
void D3D12TextureCube::FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    return this->FillUavDesc(desc, first_array_index, array_size, CubeFaceType::Positive_X, 6, mip_level);
}
void D3D12TextureCube::FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, CubeFaceType first_face, uint32_t num_faces, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = first_array_index * 6 + (uint32_t)first_face;
    desc.Texture2DArray.ArraySize = array_size * 6 + num_faces;
    desc.Texture2DArray.PlaneSlice = (desc.Format == DXGI_FORMAT_R24_UNORM_X8_TYPELESS) ? 1 : 0;
}

/******************************************************************************
* D3D12Texture3D
*******************************************************************************/
D3D12Texture3D::D3D12Texture3D(Context* context, const RHITexture::Desc& tex_desc)
    :D3D12Texture(context, tex_desc)
{

}
SResult D3D12Texture3D::Create(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    return this->DoCreate(D3D12_RESOURCE_DIMENSION_TEXTURE3D, m_desc.width, m_desc.height, m_desc.depth, m_desc.num_array, bitmap_datas);
}
SResult D3D12Texture3D::Update(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    return S_Success;
}
SResult D3D12Texture3D::DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index, uint32_t mip_level, Box<uint32_t>* box)
{
    return S_Success;
}


void D3D12Texture3D::FillSrvDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MostDetailedMip = first_level;
    desc.Texture3D.MipLevels = num_levels;
    desc.Texture3D.ResourceMinLODClamp = 0;
}
void D3D12Texture3D::FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MipSlice = mip_level;
    desc.Texture3D.FirstWSlice = first_slice;
    desc.Texture3D.WSize = num_slices;
}
void D3D12Texture3D::FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_D16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;     break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_D32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.Flags = D3D12_DSV_FLAG_NONE;
    if (m_desc.num_samples > 1)
    {
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = first_slice;
    desc.Texture2DArray.ArraySize = num_slices;
}
void D3D12Texture3D::FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    return this->FillUavDesc(desc, first_array_index, 0, m_desc.depth >> mip_level, mip_level);
}
void D3D12Texture3D::FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MipSlice = mip_level;
    desc.Texture3D.FirstWSlice = first_slice;
    desc.Texture3D.WSize = num_slices;
}

SEEK_NAMESPACE_END
