#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "rhi/d3d11_rhi/d3d11_texture.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"
#include "rhi/d3d11_rhi/d3d11_translate.h"
#include "rhi/base/format.h"
#include "kernel/context.h"

#include "math/math_utility.h"
#include "math/hash.h"
#include "utils/log.h"
#include "utils/error.h"
#include "utils/buffer.h"

#define SEEK_MACRO_FILE_UID 12     // this code is auto generated, don't touch it!!!


SEEK_NAMESPACE_BEGIN
/******************************************************************************
* D3D11Texture
*******************************************************************************/
D3D11Texture::D3D11Texture(Context* context, const RHITexture::Desc& tex_desc)
    : RHITexture(context, tex_desc)
{
    m_eDxgiFormat = D3D11Translate::TranslateToPlatformFormat(m_desc.format);
    uint32_t num_mips = tex_desc.num_mips;
    if (0 == num_mips)
    {
        num_mips = 1;
        uint32_t w = m_desc.width;
        uint32_t h = m_desc.height;
        while ((w != 1) || (h != 1))
        {
            ++num_mips;

            w = std::max(1U, w / 2);
            h = std::max(1U, h / 2);
        }
    }
    m_desc.num_mips = num_mips;
}

D3D11Texture::~D3D11Texture()
{

}

ID3D11RenderTargetView* D3D11Texture::GetD3DRtv()
{
    if (m_pD3DRenderTargetView)
        return m_pD3DRenderTargetView.Get();

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());

    D3D11_RENDER_TARGET_VIEW_DESC desc = {};
    this->FillRtvDesc(desc);

    HRESULT hr = rc.GetD3D11Device()->CreateRenderTargetView(m_pTexture.Get(), &desc, m_pD3DRenderTargetView.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("D3D11Texture::GetD3DRtv error, hr:0x%x", hr);
        return nullptr;
    }
    return m_pD3DRenderTargetView.Get();
}

ID3D11DepthStencilView* D3D11Texture::GetD3DDsv()
{
    if (m_pD3DDepthStencilView)
        return m_pD3DDepthStencilView.Get();

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());

    D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
    this->FillDsvDesc(desc);
    HRESULT hr = rc.GetD3D11Device()->CreateDepthStencilView(m_pTexture.Get(), &desc, m_pD3DDepthStencilView.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("D3D11Texture::GetD3DDsv error, hr:0x%x", hr);
        return nullptr;
    }
    return m_pD3DDepthStencilView.Get();
}

ID3D11ShaderResourceView* D3D11Texture::GetD3DSrv()
{
    if (m_pD3DShaderResourceView)
        return m_pD3DShaderResourceView.Get();

    // don't support multisample texture2d as srv
    if (m_desc.num_samples > 1 && !m_pResolvedTexture)
    {
        LOG_ERROR("msaa is enabled, but the resolved texture is null, call Resolve first");
        return nullptr;
    }

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());

    D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
    this->FillSrvDesc(desc);

    ID3D11Resource* res = m_pTexture.Get();
    if (m_pResolvedTexture)
        res = m_pResolvedTexture.Get();

    HRESULT hr = rc.GetD3D11Device()->CreateShaderResourceView(res, &desc, m_pD3DShaderResourceView.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("D3D11Texture::GetD3DSrv error, hr:0x%x", hr);
        return nullptr;
    }
    return m_pD3DShaderResourceView.Get();
}

ID3D11UnorderedAccessView* D3D11Texture::GetD3DUav()
{
    if (m_pD3DUnorderedAccessView)
        return m_pD3DUnorderedAccessView.Get();

    // don't support multisample texture2d as uav
    if (m_desc.num_samples > 1 && !m_pResolvedTexture)
    {
        LOG_ERROR("msaa is enabled, but the resolved texture is null, call Resolve first");
        return nullptr;
    }

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());

    D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
    this->FillUavDesc(desc);

    ID3D11Resource* res = m_pTexture.Get();
    if (m_pResolvedTexture)
        res = m_pResolvedTexture.Get();

    HRESULT hr = rc.GetD3D11Device()->CreateUnorderedAccessView(res, &desc, m_pD3DUnorderedAccessView.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("D3D11Texture::GetD3DUav error, hr:0x%x", hr);
        return nullptr;
    }
    return m_pD3DUnorderedAccessView.Get();
}
SResult D3D11Texture::GenerateMipMap()
{
    ID3D11ShaderResourceView* pSrv = this->GetD3DSrv();
    D3D11RHIContext* pRC = (D3D11RHIContext*)(&m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = pRC->GetD3D11DeviceContext();
    pDeviceContext->GenerateMips(pSrv);
    return S_Success;
}

ID3D11ShaderResourceViewPtr const& D3D11Texture::GetD3DSrv(uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
        this->FillSrvDesc(desc, first_array_index, array_size, first_level, num_levels);
        ID3D11ShaderResourceViewPtr d3d_srv;
        rc.GetD3D11Device()->CreateShaderResourceView(m_pTexture.Get(), &desc, d3d_srv.GetAddressOf());
        return m_mD3dSrvs.emplace(hash_val, std::move(d3d_srv)).first->second;
    }
}
ID3D11ShaderResourceViewPtr const& D3D11Texture::GetD3DSrv(uint32_t array_index, CubeFaceType face, uint32_t first_level, uint32_t num_levels)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
        this->FillSrvDesc(desc, array_index, face, first_level, num_levels);
        ID3D11ShaderResourceViewPtr d3d_srv = {};
        rc.GetD3D11Device()->CreateShaderResourceView(m_pTexture.Get(), &desc, d3d_srv.GetAddressOf());
        return m_mD3dSrvs.emplace(hash_val, std::move(d3d_srv)).first->second;
    }
}

ID3D11RenderTargetViewPtr const& D3D11Texture::GetD3DRtv(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_RENDER_TARGET_VIEW_DESC desc = {};
        this->FillRtvDesc(desc, first_array_index, array_size, mip_level);
        ID3D11RenderTargetViewPtr d3d_rtv;
        rc.GetD3D11Device()->CreateRenderTargetView(m_pTexture.Get(), &desc, d3d_rtv.GetAddressOf());
        return m_mD3dRtvs.emplace(hash_val, std::move(d3d_rtv)).first->second;
    }
}
ID3D11RenderTargetViewPtr const& D3D11Texture::GetD3DRtv(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_RENDER_TARGET_VIEW_DESC desc = {};
        this->FillRtvDesc(desc, array_index, first_slice, num_slices, mip_level);
        ID3D11RenderTargetViewPtr d3d_rtv = {};
        rc.GetD3D11Device()->CreateRenderTargetView(m_pTexture.Get(), &desc, d3d_rtv.GetAddressOf());
        return m_mD3dRtvs.emplace(hash_val, std::move(d3d_rtv)).first->second;
    }
}
ID3D11RenderTargetViewPtr const& D3D11Texture::GetD3DRtv(uint32_t array_index, CubeFaceType face, uint32_t mip_level)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_RENDER_TARGET_VIEW_DESC desc = {};
        this->FillRtvDesc(desc, array_index, face, mip_level);
        ID3D11RenderTargetViewPtr d3d_rtv;
        rc.GetD3D11Device()->CreateRenderTargetView(m_pTexture.Get(), &desc, d3d_rtv.GetAddressOf());
        return m_mD3dRtvs.emplace(hash_val, std::move(d3d_rtv)).first->second;
    }
}



ID3D11DepthStencilViewPtr const& D3D11Texture::GetD3DDsv(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
        this->FillDsvDesc(desc, first_array_index, array_size, mip_level);
        ID3D11DepthStencilViewPtr d3d_dsv;
        rc.GetD3D11Device()->CreateDepthStencilView(m_pTexture.Get(), &desc, d3d_dsv.GetAddressOf());
        return m_mD3dDsvs.emplace(hash_val, std::move(d3d_dsv)).first->second;
    }
}
ID3D11DepthStencilViewPtr const& D3D11Texture::GetD3DDsv(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
        this->FillDsvDesc(desc, array_index, first_slice, num_slices, mip_level);
        ID3D11DepthStencilViewPtr d3d_dsv;
        rc.GetD3D11Device()->CreateDepthStencilView(m_pTexture.Get(), &desc, d3d_dsv.GetAddressOf());
        return m_mD3dDsvs.emplace(hash_val, std::move(d3d_dsv)).first->second;
    }
}
ID3D11DepthStencilViewPtr const& D3D11Texture::GetD3DDsv(uint32_t array_index, CubeFaceType face, uint32_t mip_level)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
        this->FillDsvDesc(desc, array_index, face, mip_level);
        ID3D11DepthStencilViewPtr d3d_dsv;
        rc.GetD3D11Device()->CreateDepthStencilView(m_pTexture.Get(), &desc, d3d_dsv.GetAddressOf());
        return m_mD3dDsvs.emplace(hash_val, std::move(d3d_dsv)).first->second;
    }
}

ID3D11UnorderedAccessViewPtr const& D3D11Texture::GetD3DUav(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
        this->FillUavDesc(desc, first_array_index, array_size, mip_level);
        ID3D11UnorderedAccessViewPtr d3d_ua_view;
        rc.GetD3D11Device()->CreateUnorderedAccessView(m_pTexture.Get(), &desc, d3d_ua_view.GetAddressOf());
        return m_mD3dUavs.emplace(hash_val, std::move(d3d_ua_view)).first->second;
    }
}
ID3D11UnorderedAccessViewPtr const& D3D11Texture::GetD3DUav(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
        this->FillUavDesc(desc, array_index, first_slice, num_slices, mip_level);
        ID3D11UnorderedAccessViewPtr d3d_ua_view;
        rc.GetD3D11Device()->CreateUnorderedAccessView(m_pTexture.Get(), &desc, d3d_ua_view.GetAddressOf());
        return m_mD3dUavs.emplace(hash_val, std::move(d3d_ua_view)).first->second;
    }
}
ID3D11UnorderedAccessViewPtr const& D3D11Texture::GetD3DUav(uint32_t first_array_index, uint32_t array_size, CubeFaceType first_face, uint32_t num_faces, uint32_t mip_level)
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
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
        this->FillUavDesc(desc, first_array_index, array_size, first_face, num_faces, mip_level);
        ID3D11UnorderedAccessViewPtr d3d_ua_view;
        rc.GetD3D11Device()->CreateUnorderedAccessView(m_pTexture.Get(), &desc, d3d_ua_view.GetAddressOf());
        return m_mD3dUavs.emplace(hash_val, std::move(d3d_ua_view)).first->second;
    }
}

/*
D3D11_USAGE	            CPU读	CPU写	GPU读	GPU写
D3D11_USAGE_DEFAULT 			        √	    √
D3D11_USAGE_IMMUTABLE			        √
D3D11_USAGE_DYNAMIC	    	    √	    √
D3D11_USAGE_STAGING	    √	    √	    √	    √
D3D11_USAGE_STAGING: 则完全允许在CPU和GPU之间的数据传输，但它只能作为一个类似中转站的资源，而不能绑定到渲染管线上，即你也不能用该纹理生成mipmaps
*/
void D3D11Texture::FillD3DTextureFlags(D3D11_USAGE& usage, UINT& bind_flags, UINT& cpu_access_flags, UINT& misc_flags)
{
    bool cpu_read       = m_desc.flags & RESOURCE_FLAG_CPU_READ;
    bool cpu_write      = m_desc.flags & RESOURCE_FLAG_CPU_WRITE;
    bool gpu_read       = m_desc.flags & RESOURCE_FLAG_GPU_READ;
    bool gpu_write      = m_desc.flags & RESOURCE_FLAG_GPU_WRITE;   // m_desc.flags& RESOURCE_FLAG_UAV || m_desc.flags & RESOURCE_FLAG_GPU_WRITE || m_desc.flags & RESOURCE_FLAG_GPU_WRITE;
    
    bool is_uav         = m_desc.flags & RESOURCE_FLAG_UAV;
    bool generate_mips  = m_desc.flags & RESOURCE_FLAG_GENERATE_MIPS;
    bool is_draw_indict = m_desc.flags & RESOURCE_FLAG_DRAW_INDIRECT_ARGS;
    bool is_append      = m_desc.flags & RESOURCE_FLAG_APPEND;
    bool is_counter     = m_desc.flags & RESOURCE_FLAG_COUNTER;

    if (cpu_write && gpu_read)
    {
        usage = D3D11_USAGE_DYNAMIC;
    }
    else
    {
        if (cpu_write)
            usage = D3D11_USAGE_DYNAMIC;
        else
        {
            if (cpu_write)
            {
                if (m_desc.num_mips != 1 || m_desc.type == TextureType::Cube)
                    usage = D3D11_USAGE_STAGING;
                else
                    usage = D3D11_USAGE_DYNAMIC;
            }
            else
            {
                if (!cpu_read && !cpu_write)
                    usage = D3D11_USAGE_DEFAULT;
                else
                    usage = D3D11_USAGE_STAGING;
            }
        }
    }

    bind_flags = 0;
    if (gpu_read || usage == D3D11_USAGE_DYNAMIC)
    {
        bind_flags |= D3D11_BIND_SHADER_RESOURCE;
    }
    if (gpu_write)
    {
        if (Formatutil::IsDepthFormat(m_desc.format))
            bind_flags |= D3D11_BIND_DEPTH_STENCIL;
        else
            bind_flags |= D3D11_BIND_RENDER_TARGET;
    }
    if (is_uav)
        bind_flags |= D3D11_BIND_UNORDERED_ACCESS;

    cpu_access_flags = 0;
    if (cpu_read || usage == D3D11_USAGE_STAGING)
    {
        cpu_access_flags |= D3D11_CPU_ACCESS_READ;
    }
    if (cpu_write || D3D11_USAGE_DYNAMIC == usage || usage == D3D11_USAGE_STAGING)
    {
        cpu_access_flags |= D3D11_CPU_ACCESS_WRITE;
    }

    misc_flags = 0;
    if (generate_mips)
    {
        bind_flags |= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        misc_flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }
}
void D3D11Texture::FillTextureDesc(D3D11_TEXTURE2D_DESC& desc)
{
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
    UINT bind_flags = 0;
    UINT cpu_access_flags = 0;
    UINT misc_flags = 0;
    this->FillD3DTextureFlags(usage, bind_flags, cpu_access_flags, misc_flags);

    ZeroMemory(&desc, sizeof(desc));
    desc.Width = m_desc.width;
    desc.Height = m_desc.height;
    desc.MipLevels = m_desc.num_mips;
    desc.ArraySize = m_desc.depth;
    desc.SampleDesc.Count = m_desc.num_samples;
    desc.SampleDesc.Quality = rc.GetDxgiSampleDesc(m_desc.num_samples).Quality;
    desc.Usage = usage;
    desc.CPUAccessFlags = cpu_access_flags;
    desc.BindFlags = bind_flags;
    desc.MiscFlags = misc_flags;

    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_TYPELESS;     break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;   break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_TYPELESS;     break;
    default:                    desc.Format = m_eDxgiFormat;                break;
    }
}
/******************************************************************************
* D3D11Texture2D
*******************************************************************************/
D3D11Texture2D::D3D11Texture2D(Context* context, const RHITexture::Desc& tex_desc)
    : D3D11Texture(context, tex_desc)
{
}

D3D11Texture2D::~D3D11Texture2D()
{
}

D3D11Texture2D::D3D11Texture2D(Context* context, ID3D11Texture2DPtr const& tex)
    : D3D11Texture(context, RHITexture::Desc{})
{
    D3D11_TEXTURE2D_DESC desc = {};
    tex->GetDesc(&desc);
    m_desc.type = TextureType::Tex2D;
    m_desc.width = desc.Width;
    m_desc.height = desc.Height;
    m_desc.depth = desc.ArraySize;
    m_desc.num_mips = desc.MipLevels;
    m_desc.format = D3D11Translate::TranslateFromPlatformFormat(desc.Format);
    m_desc.num_samples = desc.SampleDesc.Count;
    m_eDxgiFormat = desc.Format;
    m_pTexture = tex;

    UINT bindFlags = desc.BindFlags;
    UINT cpuAccessFlags = desc.CPUAccessFlags;
    D3D11_USAGE usage = desc.Usage;
    ResourceFlags flags = RESOURCE_FLAG_NONE;
    if (usage == D3D11_USAGE_DEFAULT)
    {
        flags |= RESOURCE_FLAG_GPU_WRITE;
    }
    else if (usage == D3D11_USAGE_DYNAMIC)
    {
        flags |= RESOURCE_FLAG_CPU_WRITE;
    }
    else if (usage == D3D11_USAGE_STAGING)
    {
        flags |= RESOURCE_FLAG_CPU_WRITE;
        flags |= RESOURCE_FLAG_GPU_WRITE;
    }

    if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
        flags |= RESOURCE_FLAG_GPU_READ;
    if (bindFlags & D3D11_BIND_RENDER_TARGET)
        flags |= RESOURCE_FLAG_GPU_WRITE;
    m_desc.flags = flags;
}

void D3D11Texture2D::FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
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
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = first_array_index;
            desc.Texture2DMSArray.ArraySize = array_size;
        }
        else
        {
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MostDetailedMip = first_level;
            desc.Texture2DArray.MipLevels = num_levels;
            desc.Texture2DArray.FirstArraySlice = first_array_index;
            desc.Texture2DArray.ArraySize = array_size;
        }
    }
    else
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MostDetailedMip = first_level;
            desc.Texture2D.MipLevels = num_levels;
        }
    }
}
void D3D11Texture2D::FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
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
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = first_array_index;
            desc.Texture2DMSArray.ArraySize = array_size;
        }
        else
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice = mip_level;
            desc.Texture2DArray.FirstArraySlice = first_array_index;
            desc.Texture2DArray.ArraySize = array_size;
        }
    }
    else
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = mip_level;
        }
    }
}
void D3D11Texture2D::FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
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
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = first_array_index;
            desc.Texture2DMSArray.ArraySize = array_size;
        }
        else
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice = mip_level;
            desc.Texture2DArray.FirstArraySlice = first_array_index;
            desc.Texture2DArray.ArraySize = array_size;
        }
    }
    else
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = mip_level;
        }
    }
}
void D3D11Texture2D::FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
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
        desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.MipSlice = mip_level;
        desc.Texture2DArray.FirstArraySlice = first_array_index;
        desc.Texture2DArray.ArraySize = array_size;
    }
    else
    {
        desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = mip_level;
    }
}

void D3D11Texture2D::FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc)
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
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = 0;
            desc.Texture2DMSArray.ArraySize = m_desc.num_array;
        }
        else
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice = 0;
            desc.Texture2DArray.FirstArraySlice = 0;
            desc.Texture2DArray.ArraySize = m_desc.num_array;
        }
    }
    else
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
        }
    }
}
void D3D11Texture2D::FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_D16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;        break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_D32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.Flags = 0; // not read only
    if (m_desc.num_array > 1)
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = 0;
            desc.Texture2DMSArray.ArraySize = m_desc.num_array;
        }
        else
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice = 0;
            desc.Texture2DArray.FirstArraySlice = 0;
            desc.Texture2DArray.ArraySize = m_desc.depth;
        }
    }
    else
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
        }
    }
}
void D3D11Texture2D::FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
    // don't support multisample texture2d as srv
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
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = 0;
            desc.Texture2DMSArray.ArraySize = m_desc.num_array;
        }
        else
        {
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MostDetailedMip = 0;
            desc.Texture2DArray.MipLevels = m_desc.num_mips;
            desc.Texture2DArray.FirstArraySlice = 0;
            desc.Texture2DArray.ArraySize = m_desc.num_array;
        }
    }
    else
    {
        if (m_desc.num_samples > 1)
        {
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MostDetailedMip = 0;
            desc.Texture2DArray.MipLevels = m_desc.num_mips;
            desc.Texture2DArray.FirstArraySlice = 0;
            desc.Texture2DArray.ArraySize = m_desc.depth;
        }
        else
        {
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MostDetailedMip = 0;
            desc.Texture2D.MipLevels = m_desc.num_mips;
        }
    }
}
void D3D11Texture2D::FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
{
    // don't support multisample texture2d as uav
    desc.Format = m_eDxgiFormat;
    if (m_desc.num_array > 1)
    {
        desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.MipSlice = 0;
        desc.Texture2DArray.FirstArraySlice = 0;
        desc.Texture2DArray.ArraySize = m_desc.num_array;
    }
    else
    {
        desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;
    }
}

SResult D3D11Texture2D::Create(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_TEXTURE2D_DESC desc = { 0 };
    this->FillTextureDesc(desc);

    std::vector<D3D11_SUBRESOURCE_DATA> subr_data;

    if (bitmap_datas.size() != 0 && bitmap_datas.size() == m_desc.num_mips)
    {
        for (uint32_t i = 0; i < bitmap_datas.size(); i++)
        {
            if (bitmap_datas[i])
            {
                subr_data.resize(subr_data.size() + 1);
                subr_data[i].pSysMem = bitmap_datas[i]->Data();
                subr_data[i].SysMemPitch = bitmap_datas[i]->RowPitch();
                subr_data[i].SysMemSlicePitch = m_desc.height * subr_data[i].SysMemPitch;
            }
        }
    }

    ID3D11Texture2DPtr _texture2d;
    HRESULT hr = pDevice->CreateTexture2D(&desc, subr_data.data(), (ID3D11Texture2D**)_texture2d.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("Create D3D11 Texture Failed");
        return ERR_INVALID_ARG;
    }

    _texture2d->GetDesc(&m_d3dTexture2DDesc);
    m_pTexture = std::move(_texture2d);
    return S_Success;
}
SResult D3D11Texture2D::Update(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    if (!(m_desc.flags & RESOURCE_FLAG_CPU_WRITE))
        return ERR_INVALID_ARG;

    if (!m_pTexture)
        return ERR_INVALID_ARG;

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_MAPPED_SUBRESOURCE mapped_data = { 0 };
    D3D11_MAP mapFlag = D3D11_MAP_WRITE;
    if (m_d3dTexture2DDesc.Usage == D3D11_USAGE_DYNAMIC)
        mapFlag = D3D11_MAP_WRITE_DISCARD;
    if (FAILED(pDeviceContext->Map(m_pTexture.Get(), 0, mapFlag, 0, &mapped_data)))
        return ERR_INVALID_ARG;

    for (size_t bi = 0; bi < bitmap_datas.size(); bi++)
    {
        if (bitmap_datas[bi])
        {
            uint8_t* src = bitmap_datas[bi]->Data();
            uint8_t* dst = (uint8_t*)mapped_data.pData;
            for (int i = 0; i < m_desc.height; i++)
            {
                memcpy_s(dst, m_desc.width * Formatutil::NumComponentBytes(m_desc.format), src, m_desc.width * Formatutil::NumComponentBytes(m_desc.format));
                src += bitmap_datas[bi]->RowPitch();
                dst += mapped_data.RowPitch;
            }
        }
    }

    pDeviceContext->Unmap(m_pTexture.Get(), 0);
    // D3D_USAGE_DEFAULT can also be updated by UpdateSubResource, but it has no CPU access!
    // Now, we disable this usage
    return S_Success;
}

void D3D11Texture2D::FillStageTexture2DDesc(D3D11_TEXTURE2D_DESC& desc)
{
    desc.Width = m_desc.width;
    desc.Height = m_desc.height;
    desc.MipLevels = m_desc.num_mips;
    desc.ArraySize = m_desc.depth;
    desc.Format = m_eDxgiFormat;
    desc.SampleDesc.Count = 1; // FIXME: always 1
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    if (m_desc.num_mips > 1 && (m_desc.flags & RESOURCE_FLAG_GENERATE_MIPS))
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
}

SResult D3D11Texture2D::Resolve()
{
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    ID3D11Device* pDevice = rc.GetD3D11Device();

    bool bMsaaEnabled = m_desc.num_samples > 1;
    if (bMsaaEnabled)
    {
        if (!m_pResolvedTexture)
        {
            D3D11_TEXTURE2D_DESC ms_desc = {};
            ms_desc.Width = m_desc.width;
            ms_desc.Height = m_desc.height;
            ms_desc.MipLevels = m_desc.num_mips;
            ms_desc.ArraySize = m_desc.depth;
            ms_desc.Format = m_eDxgiFormat;
            ms_desc.SampleDesc.Count = 1;
            ms_desc.SampleDesc.Quality = 0;
            ms_desc.Usage = D3D11_USAGE_DEFAULT; // TODO: use D3D11_USAGE_STAGING when copyback is enabled?
            ms_desc.CPUAccessFlags = 0;
            ms_desc.BindFlags = 0;
            ms_desc.MiscFlags = 0;
            if (m_desc.num_mips > 1 && (m_desc.flags & RESOURCE_FLAG_GENERATE_MIPS))
                ms_desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
            if (m_desc.flags & RESOURCE_FLAG_GPU_READ)
            {
                if (m_desc.flags & RESOURCE_FLAG_UAV)
                    ms_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
                else
                    ms_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }

            ID3D11Texture2DPtr _texture2d;
            if (FAILED(pDevice->CreateTexture2D(&ms_desc, nullptr, _texture2d.GetAddressOf())))
                return ERR_INVALID_ARG;
            m_pResolvedTexture = std::move(_texture2d);
        }

        pDeviceContext->ResolveSubresource(m_pResolvedTexture.Get(), 0, m_pTexture.Get(), 0, m_eDxgiFormat);
    }
    return S_Success;
}
SResult D3D11Texture2D::DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index, uint32_t mip_level, Rect<uint32_t>* rect)
{    
    if (!m_pTexture)
        return ERR_INVALID_ARG;

    // don't support copyback the multisample texture2d
    if (m_desc.num_samples > 1 && !m_pResolvedTexture)
    {
        LOG_ERROR("msaa is enabled, but the resolved texture is null, call Resolve first");
        return ERR_INVALID_ARG;
    }

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    ID3D11Device* pDevice = rc.GetD3D11Device();

    bool bCPUAccessRead = m_d3dTexture2DDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ;
    ID3D11ResourcePtr pSrcRes = m_pTexture;
    if (m_pResolvedTexture)
    {
        D3D11_TEXTURE2D_DESC resolvedTextureDesc = {};
        ID3D11Texture2DPtr tex = nullptr;
        m_pResolvedTexture.As(&tex);
        tex->GetDesc(&resolvedTextureDesc);

        bCPUAccessRead = resolvedTextureDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ;
        pSrcRes = m_pResolvedTexture;
    }

    ID3D11ResourcePtr pCopyRes = nullptr;
    if (!bCPUAccessRead)
    {
        D3D11_TEXTURE2D_DESC stage_desc;
        FillStageTexture2DDesc(stage_desc);

        ID3D11Texture2DPtr _texture2d;
        if (FAILED(pDevice->CreateTexture2D(&stage_desc, nullptr, _texture2d.GetAddressOf())))
            return ERR_INVALID_ARG;
        pCopyRes = std::move(_texture2d);

        pDeviceContext->CopyResource(pCopyRes.Get(), pSrcRes.Get());
    }
    else
    {
        pCopyRes = pSrcRes;
    }

    D3D11_MAPPED_SUBRESOURCE mapped_data = { 0 };
    HRESULT hr = pDeviceContext->Map(pCopyRes.Get(), D3D11CalcSubresource(mip_level, array_index, m_desc.num_mips), D3D11_MAP_READ, 0, &mapped_data);
    if (FAILED(hr))
        return ERR_INVALID_ARG;

    Rect<uint32_t> copyRect(0, 0, m_desc.width, m_desc.height);
    if (rect)
        copyRect = *rect;
    if (!bitmap_data->Expand(copyRect.width, copyRect.height, m_desc.format))
        return ERR_NO_MEM;

    uint8_t* dst = bitmap_data->Data();
    uint8_t* src = (uint8_t*)mapped_data.pData + copyRect.y * mapped_data.RowPitch + copyRect.x * Formatutil::NumComponentBytes(m_desc.format);
    for (int i = 0; i < copyRect.height; i++)
    {
        memcpy_s(dst, copyRect.width * Formatutil::NumComponentBytes(m_desc.format), src, copyRect.width * Formatutil::NumComponentBytes(m_desc.format));
        dst += bitmap_data->RowPitch();
        src += mapped_data.RowPitch;
    }
    pDeviceContext->Unmap(pCopyRes.Get(), D3D11CalcSubresource(mip_level, array_index, m_desc.num_mips));
    return S_Success;
}
/******************************************************************************
* D3D11TextureCube
*******************************************************************************/
D3D11TextureCube::D3D11TextureCube(Context* context, const RHITexture::Desc& tex_desc)
    :D3D11Texture(context, tex_desc)
{
    m_vCubeDSV.resize((uint32_t)CubeFaceType::Num, nullptr);
}
ID3D11RenderTargetView* D3D11TextureCube::GetD3DRtv(CubeFaceType face, uint32_t mip_level)
{
    if (!m_mCubeRTV[mip_level].empty() && m_mCubeRTV[mip_level][(uint32_t)face] )
        return m_mCubeRTV[mip_level][(uint32_t)face].Get();

    if (m_mCubeRTV[mip_level].empty())
        m_mCubeRTV[mip_level].resize((uint32_t)CubeFaceType::Num, nullptr);
        
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_RENDER_TARGET_VIEW_DESC desc = {};
    this->FillRtvDesc(desc, face, mip_level);
    ID3D11RenderTargetViewPtr rtv = nullptr;
    HRESULT hr = pDevice->CreateRenderTargetView(m_pTexture.Get(), &desc, m_mCubeRTV[mip_level][(uint32_t)face].GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("D3D11TextureCube::GetD3DRtv error");
        return nullptr;
    }

    return m_mCubeRTV[mip_level][(uint32_t)face].Get();
}
ID3D11DepthStencilView* D3D11TextureCube::GetD3DDsv(CubeFaceType face)
{
    if (m_vCubeDSV[(uint32_t)face])
        return m_vCubeDSV[(uint32_t)face].Get();

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
    this->FillDsvDesc(desc, face);
    HRESULT hr = pDevice->CreateDepthStencilView(m_pTexture.Get(), &desc, m_vCubeDSV[(uint32_t)face].GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("D3D11TextureCube::GetD3DRtv error");
        return nullptr;
    }

    return m_vCubeDSV[(uint32_t)face].Get();
}
void D3D11TextureCube::FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) 
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    desc.TextureCube.MostDetailedMip = first_level;
    desc.TextureCube.MipLevels = num_levels;
}
void D3D11TextureCube::FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t first_level, uint32_t num_levels)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    desc.Texture2DArray.MostDetailedMip = first_level;
    desc.Texture2DArray.MipLevels = num_levels;
    desc.Texture2DArray.FirstArraySlice = array_index * 6 + (uint32_t)face;
    desc.Texture2DArray.ArraySize = 1;
}
void D3D11TextureCube::FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) 
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
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
    desc.Texture2DArray.ArraySize = array_size * 6;
}
void D3D11TextureCube::FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level) 
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
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = array_index * 6 + (uint32_t)face;
    desc.Texture2DArray.ArraySize = 1;
}
void D3D11TextureCube::FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) 
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_D16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;        break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_D32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.Flags = 0;
    if (m_desc.num_samples > 1)
    {
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
    desc.Texture2DArray.ArraySize = array_size * 6;
}
void D3D11TextureCube::FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_D16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;        break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_D32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.Flags = 0;
    if (m_desc.num_samples > 1)
    {
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = array_index * 6 + (uint32_t)face;
    desc.Texture2DArray.ArraySize = 1;
}
void D3D11TextureCube::FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) 
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
void D3D11TextureCube::FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, CubeFaceType first_face, uint32_t num_faces, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = first_array_index * 6 + (uint32_t)first_face;
    desc.Texture2DArray.ArraySize = array_size * 6 + num_faces;
}

void D3D11TextureCube::FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC & desc, CubeFaceType face, uint32_t mip_level)
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
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
        desc.Texture2DMSArray.ArraySize = 1;
        desc.Texture2DMSArray.FirstArraySlice = (uint32_t)face;
    }
    else
    {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.MipSlice = mip_level;
        desc.Texture2DArray.FirstArraySlice = (uint32_t)face;
        desc.Texture2DArray.ArraySize = 1;
    }
}
void D3D11TextureCube::FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, CubeFaceType face)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_D16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;     break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_D32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.Flags = 0; // not read only
    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    desc.Texture2DArray.MipSlice = 0;
    desc.Texture2DArray.ArraySize = 1;
    desc.Texture2DArray.FirstArraySlice = (uint32_t)face;
}
void D3D11TextureCube::FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
    // don't support multisample texture2d as srv
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }

    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; 
    desc.TextureCube.MostDetailedMip = 0;
    desc.TextureCube.MipLevels = 1;
}
void D3D11TextureCube::FillTextureDesc(D3D11_TEXTURE2D_DESC& desc)
{
    D3D11Texture::FillTextureDesc(desc);
    desc.ArraySize = 6;
    desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
}
SResult D3D11TextureCube::Create(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    if (bitmap_datas.size() != (uint32_t)CubeFaceType::Num &&
        bitmap_datas.size() != 0)
        return ERR_INVALID_ARG;

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    D3D11_TEXTURE2D_DESC desc = { 0 };
    this->FillTextureDesc(desc);
    
    std::vector<D3D11_SUBRESOURCE_DATA> subr_data;
    if (!bitmap_datas.empty())
    {
        subr_data.resize((uint32_t)bitmap_datas.size());
        for (uint32_t i = 0; i < (uint32_t)bitmap_datas.size(); i++)
        {
            if (bitmap_datas[i])
            {
                subr_data[i].pSysMem = bitmap_datas[i]->Data();
                subr_data[i].SysMemPitch = bitmap_datas[i]->RowPitch();
                subr_data[i].SysMemSlicePitch = m_desc.height * subr_data[i].SysMemPitch;
            }
        }
    }
    HRESULT hr = pDevice->CreateTexture2D(&desc, subr_data.data(), (ID3D11Texture2D**)m_pTexture.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("Create D3D11 Texture Failed");
        return ERR_INVALID_ARG;
    }
    return S_Success;
}
SResult D3D11TextureCube::DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index, uint32_t mip_level, Rect<uint32_t>* rect)
{

    return true;
}
/******************************************************************************
* D3D11Texture3D
*******************************************************************************/
D3D11Texture3D::D3D11Texture3D(Context* context, const RHITexture::Desc& tex_desc)
    :D3D11Texture(context, tex_desc)
{

}
SResult D3D11Texture3D::Create(std::span<BitmapBufferPtr> const& bitmap_datas)
{
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();

    D3D11_TEXTURE3D_DESC desc = { 0 };
    this->FillTexture3DDesc(desc);

    std::vector<D3D11_SUBRESOURCE_DATA> subr_data;

    if (bitmap_datas.size() != 0 && bitmap_datas.size() == m_desc.num_mips)
    {
        if (bitmap_datas.size() != m_desc.num_mips)
        {
            LOG_ERROR("D3D11Texture3D::Create() bitmap_datas.size() != num_mips");
            return ERR_INVALID_ARG;
        }
        for (uint32_t i = 0; i < bitmap_datas.size(); i++)
        {
            if (bitmap_datas[i])
            {
                subr_data.resize(subr_data.size() + 1);
                subr_data[i].pSysMem = bitmap_datas[i]->Data();
                subr_data[i].SysMemPitch = bitmap_datas[i]->RowPitch();
                subr_data[i].SysMemSlicePitch = m_desc.height * subr_data[i].SysMemPitch;
            }
        }
    }

    ID3D11Texture3DPtr _texture3d;
    HRESULT hr = pDevice->CreateTexture3D(&desc, subr_data.data(), (ID3D11Texture3D**)_texture3d.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("Create D3D11 Texture Failed");
        return ERR_INVALID_ARG;
    }
    _texture3d->GetDesc(&m_d3dTexture3DDesc);
    m_pTexture = std::move(_texture3d);
    return S_Success;
}   
SResult D3D11Texture3D::DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index, uint32_t mip_level, Box<uint32_t>* box)
{
    if (!m_pTexture)
        return ERR_INVALID_ARG;

    // don't support copyback the multisample texture2d
    if (m_desc.num_samples > 1 && !m_pResolvedTexture)
    {
        LOG_ERROR("msaa is enabled, but the resolved texture is null, call Resolve first");
        return ERR_INVALID_ARG;
    }

    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    ID3D11Device* pDevice = rc.GetD3D11Device();

    bool bCPUAccessRead = m_d3dTexture3DDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ;
    ID3D11ResourcePtr pSrcRes = m_pTexture;
    if (m_pResolvedTexture)
    {
        D3D11_TEXTURE3D_DESC resolvedTextureDesc = {};
        ID3D11Texture3DPtr tex = nullptr;
        m_pResolvedTexture.As(&tex);
        tex->GetDesc(&resolvedTextureDesc);

        bCPUAccessRead = resolvedTextureDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ;
        pSrcRes = m_pResolvedTexture;
    }

    ID3D11ResourcePtr pCopyRes = nullptr;
    if (!bCPUAccessRead)
    {
        D3D11_TEXTURE3D_DESC stage_desc = {};
        FillStageTexture3DDesc(stage_desc);

        ID3D11Texture3DPtr _texture3d;
        if (FAILED(pDevice->CreateTexture3D(&stage_desc, nullptr, _texture3d.GetAddressOf())))
            return ERR_INVALID_ARG;
        pCopyRes = std::move(_texture3d);

        pDeviceContext->CopyResource(pCopyRes.Get(), pSrcRes.Get());
    }
    else
    {
        pCopyRes = pSrcRes;
    }

    D3D11_MAPPED_SUBRESOURCE mapped_data = { 0 };
    HRESULT hr = pDeviceContext->Map(pCopyRes.Get(), D3D11CalcSubresource(mip_level, array_index, m_desc.num_mips), D3D11_MAP_READ, 0, &mapped_data);
    if (FAILED(hr))
        return ERR_INVALID_ARG;

    Box<uint32_t> copyBox(0, 0, 0, m_desc.width, m_desc.height, m_desc.depth);
    if (box)
        copyBox = *box;
    if (!bitmap_data->Expand(copyBox.width, copyBox.height, m_desc.format, copyBox.depth))
        return ERR_NO_MEM;

    for (uint32_t j = 0; j < copyBox.depth; j++)
    {
        uint8_t* dst = bitmap_data->Data() + bitmap_data->SlicePitch() * j;
        uint8_t* src = (uint8_t*)mapped_data.pData + mapped_data.DepthPitch * (j + copyBox.z) + copyBox.y * mapped_data.RowPitch + copyBox.x * Formatutil::NumComponentBytes(m_desc.format);
        for (int i = 0; i < copyBox.height; i++)
        {
            memcpy_s(dst, copyBox.width * Formatutil::NumComponentBytes(m_desc.format), src, copyBox.width * Formatutil::NumComponentBytes(m_desc.format));
            dst += bitmap_data->RowPitch();
            src += mapped_data.RowPitch;
        }
    }
    pDeviceContext->Unmap(pCopyRes.Get(), D3D11CalcSubresource(mip_level, array_index, m_desc.num_mips));
    return S_Success;
}
void D3D11Texture3D::FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MostDetailedMip = first_level;
    desc.Texture3D.MipLevels = num_levels;
}
void D3D11Texture3D::FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MipSlice = mip_level;
    desc.Texture3D.FirstWSlice = first_slice;
    desc.Texture3D.WSize = num_slices;
}
void D3D11Texture3D::FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_D16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;     break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_D32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.Flags = 0;
    if (m_desc.num_samples > 1)
    {
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = mip_level;
    desc.Texture2DArray.FirstArraySlice = first_slice;
    desc.Texture2DArray.ArraySize = num_slices;
}
void D3D11Texture3D::FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
{
    return this->FillUavDesc(desc, first_array_index, 0, m_desc.depth >> mip_level, mip_level);
}
void D3D11Texture3D::FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MipSlice = mip_level;
    desc.Texture3D.FirstWSlice = first_slice;
    desc.Texture3D.WSize = num_slices;
}
void D3D11Texture3D::FillTexture3DDesc(D3D11_TEXTURE3D_DESC& desc)
{
    D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
    D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
    UINT bind_flags = 0;
    UINT cpu_access_flags = 0;
    UINT misc_flags = 0;
    this->FillD3DTextureFlags(usage, bind_flags, cpu_access_flags, misc_flags);

    ZeroMemory(&desc, sizeof(desc));
    desc.Width = m_desc.width;
    desc.Height = m_desc.height;
    desc.Depth = m_desc.depth;
    desc.MipLevels = m_desc.num_mips;
    desc.Usage = usage;
    desc.BindFlags = bind_flags;
    desc.CPUAccessFlags = cpu_access_flags;    
    desc.MiscFlags |= misc_flags;

    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_TYPELESS;     break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;   break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_TYPELESS;     break;
    default:                    desc.Format = m_eDxgiFormat;                break;
    }
}
void D3D11Texture3D::FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }

    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MipSlice = 0;
    desc.Texture3D.FirstWSlice = 0;
    desc.Texture3D.WSize = m_desc.depth;
 
}
void D3D11Texture3D::FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
    switch (m_desc.format)
    {
    case PixelFormat::D16:      desc.Format = DXGI_FORMAT_R16_UNORM;                break;
    case PixelFormat::D24S8:    desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
    case PixelFormat::D32F:     desc.Format = DXGI_FORMAT_R32_FLOAT;                break;
    default:                    desc.Format = m_eDxgiFormat;                        break;
    }
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MostDetailedMip = 0;
    desc.Texture3D.MipLevels = m_desc.num_mips;
}
void D3D11Texture3D::FillStageTexture3DDesc(D3D11_TEXTURE3D_DESC& desc)
{
    desc.Width = m_desc.width;
    desc.Height = m_desc.height;
    desc.Depth = m_desc.depth;
    desc.MipLevels = m_desc.num_mips;
    desc.Format = m_eDxgiFormat;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    if (m_desc.num_mips > 1 && (m_desc.flags & RESOURCE_FLAG_GENERATE_MIPS))
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
