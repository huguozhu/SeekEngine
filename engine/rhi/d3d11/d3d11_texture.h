#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/d3d11/d3d11_predeclare.h"
#include "utils/log.h"
#include "utils/util.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
* D3D11Texture
*******************************************************************************/
class D3D11Texture : public RHITexture
{
public:
    D3D11Texture(Context* context, const RHITexture::Desc& tex_desc);
    virtual ~D3D11Texture() override;

    virtual SResult Resolve() { return S_Success; }
    ID3D11Resource* GetD3DTexture() { return m_pTexture.Get(); }
    ID3D11Resource* GetD3DResolvedTexture() { return m_pResolvedTexture.Get(); }
    DXGI_FORMAT GetD3DFormat() { return m_eDxgiFormat; }
    virtual SResult GenerateMipMap() override;

    ID3D11ShaderResourceViewPtr const& GetD3DSrv();
    ID3D11ShaderResourceViewPtr const& GetD3DSrv(uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels);
    ID3D11ShaderResourceViewPtr const& GetD3DSrv(uint32_t array_index, CubeFaceType face,  uint32_t first_level, uint32_t num_levels);

    ID3D11RenderTargetViewPtr const& GetD3DRtv();
    ID3D11RenderTargetViewPtr const& GetD3DRtv(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    ID3D11RenderTargetViewPtr const& GetD3DRtv(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
    ID3D11RenderTargetViewPtr const& GetD3DRtv(uint32_t array_index, CubeFaceType face, uint32_t mip_level);

    ID3D11DepthStencilViewPtr const& GetD3DDsv();
    ID3D11DepthStencilViewPtr const& GetD3DDsv(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    ID3D11DepthStencilViewPtr const& GetD3DDsv(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
    ID3D11DepthStencilViewPtr const& GetD3DDsv(uint32_t array_index, CubeFaceType face, uint32_t mip_level);

    ID3D11UnorderedAccessViewPtr const& GetD3DUav();
    ID3D11UnorderedAccessViewPtr const& GetD3DUav(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    ID3D11UnorderedAccessViewPtr const& GetD3DUav(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
    ID3D11UnorderedAccessViewPtr const& GetD3DUav(uint32_t first_array_index, uint32_t array_size, CubeFaceType first_face, uint32_t num_faces, uint32_t mip_level);

    virtual void FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)         { ErrUnreachable("Can't be called."); }
    virtual void FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t first_level, uint32_t num_levels)                 { ErrUnreachable("Can't be called."); }

    virtual void FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)                                  { ErrUnreachable("Can't be called."); }
    virtual void FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)                  { ErrUnreachable("Can't be called."); }
    virtual void FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level)                                          { ErrUnreachable("Can't be called."); }

    virtual void FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)                                  { ErrUnreachable("Can't be called."); }
    virtual void FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)                  { ErrUnreachable("Can't be called."); }
    virtual void FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level)                                          { ErrUnreachable("Can't be called."); }

    virtual void FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)                               { ErrUnreachable("Can't be called."); }
    virtual void FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)               { ErrUnreachable("Can't be called."); }
    virtual void FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, CubeFaceType first_face, uint32_t num_faces, uint32_t mip_level)  { ErrUnreachable("Can't be called."); }

    virtual SResult DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr)                        { ErrUnreachable("Can't be called."); return S_Success; } 
    virtual SResult DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Box<uint32_t>* box = nullptr)                          { ErrUnreachable("Can't be called."); return S_Success; } 
    virtual SResult DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr)   { ErrUnreachable("Can't be called."); return S_Success; } 

protected:
    void FillD3DTextureFlags(D3D11_USAGE& usage, UINT& bind_flags, UINT& cpu_access_flags, UINT& misc_flags);
    virtual void FillTextureDesc(D3D11_TEXTURE2D_DESC& desc);

protected:
    DXGI_FORMAT                     m_eDxgiFormat = DXGI_FORMAT_UNKNOWN;
    ID3D11ResourcePtr               m_pTexture = nullptr;
    ID3D11ResourcePtr               m_pResolvedTexture = nullptr;

    std::unordered_map<size_t, ID3D11ShaderResourceViewPtr>     m_mD3dSrvs;
    std::unordered_map<size_t, ID3D11RenderTargetViewPtr>       m_mD3dRtvs;
    std::unordered_map<size_t, ID3D11DepthStencilViewPtr>       m_mD3dDsvs;
    std::unordered_map<size_t, ID3D11UnorderedAccessViewPtr>    m_mD3dUavs;
};
using D3D11TexturePtr = std::shared_ptr<D3D11Texture>;

/******************************************************************************
* D3D11Texture2D
*******************************************************************************/
class D3D11Texture2D : public D3D11Texture
{
public:
    D3D11Texture2D(Context* context, const RHITexture::Desc& tex_desc);
    D3D11Texture2D(Context* context, ID3D11Texture2DPtr const& tex);
    virtual ~D3D11Texture2D() override;

    SResult Create  (std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult Update  (std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult Resolve() override;
    SResult DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr);

protected:
    void FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc,     uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) override;
    void FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc,       uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) override;
    void FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc,       uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) override;
    void FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc,    uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) override;

private:
    void FillStageTexture2DDesc(D3D11_TEXTURE2D_DESC& desc);

    D3D11_TEXTURE2D_DESC m_d3dTexture2DDesc = {};
};
using D3D11Texture2DPtr = std::shared_ptr<D3D11Texture2D>;

/******************************************************************************
* D3D11TextureCube
*******************************************************************************/
class D3D11TextureCube : public D3D11Texture
{
public:
    D3D11TextureCube(Context* context, const RHITexture::Desc& tex_desc);

    SResult Create(std::span<BitmapBufferPtr> const& bitmap_datas);
    SResult Update(std::span<BitmapBufferPtr> const& bitmap_datas) override { return S_Success; }
    SResult DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr);

private:
    void FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels);
    void FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t first_level, uint32_t num_levels);
    void FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    void FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level);
    void FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    void FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level);
    void FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    void FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, CubeFaceType first_face, uint32_t num_faces, uint32_t mip_level);

private:
    void FillTextureDesc(D3D11_TEXTURE2D_DESC& desc) override;
};
using D3D11TextureCubePtr = std::shared_ptr<D3D11TextureCube>;


/******************************************************************************
* D3D11Texture3D
*******************************************************************************/
class D3D11Texture3D : public D3D11Texture
{
public:
    D3D11Texture3D(Context* context, const RHITexture::Desc& tex_desc);

    SResult Create(std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult Update(std::span<BitmapBufferPtr> const& bitmap_datas) override { return S_Success; }
    SResult DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Box<uint32_t>* box = nullptr);

private:
    void FillSrvDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels);
    void FillRtvDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
    void FillDsvDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
    void FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    void FillUavDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level) ;

    void FillTexture3DDesc(D3D11_TEXTURE3D_DESC& desc);
    void FillStageTexture3DDesc(D3D11_TEXTURE3D_DESC& desc);
    D3D11_TEXTURE3D_DESC m_d3dTexture3DDesc = {};
};
using D3D11Texture3DPtr = std::shared_ptr<D3D11Texture3D>;
SEEK_NAMESPACE_END
