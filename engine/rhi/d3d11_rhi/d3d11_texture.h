#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/d3d11_rhi/d3d11_predeclare.h"
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

    virtual ID3D11RenderTargetView*     GetD3DRenderTargetView();
    virtual ID3D11DepthStencilView*     GetD3DDepthStencilView();
    virtual ID3D11ShaderResourceView*   GetD3DShaderResourceView();
    virtual ID3D11UnorderedAccessView*  GetD3DUnorderedAccessView();
    virtual SResult GenerateMipMap() override;

    virtual void FillRenderTargetViewDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc);
    virtual void FillDepthStencilViewDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc);
    virtual void FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
    virtual void FillUnorderedAccessViewDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc);

    virtual SResult CopySubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr);
    virtual SResult CopySubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Box<uint32_t>* box = nullptr);
    virtual SResult CopySubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr);

protected:
    void FillD3DTextureFlags(D3D11_USAGE& usage, UINT& bind_flags, UINT& cpu_access_flags, UINT& misc_flags);
    virtual void FillTextureDesc(D3D11_TEXTURE2D_DESC& desc);

protected:
    DXGI_FORMAT                     m_eDxgiFormat = DXGI_FORMAT_UNKNOWN;
    ID3D11ResourcePtr               m_pTexture = nullptr;
    ID3D11ResourcePtr               m_pResolvedTexture = nullptr;
    ID3D11RenderTargetViewPtr       m_pD3DRenderTargetView = nullptr;
    ID3D11DepthStencilViewPtr       m_pD3DDepthStencilView = nullptr;
    ID3D11ShaderResourceViewPtr     m_pD3DShaderResourceView = nullptr;
    ID3D11UnorderedAccessViewPtr    m_pD3DUnorderedAccessView = nullptr;
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
    SResult CopySubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr);


protected:
    void FillRenderTargetViewDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc) override;
    void FillDepthStencilViewDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc) override;
    void FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) override;
    void FillUnorderedAccessViewDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc) override;

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

    ID3D11RenderTargetView* GetD3DRenderTargetView(CubeFaceType face, uint32_t lod = 0);
    ID3D11DepthStencilView* GetD3DDepthStencilView(CubeFaceType face);

    SResult Create(std::span<BitmapBufferPtr> const& bitmap_datas);
    SResult Update(std::span<BitmapBufferPtr> const& bitmap_datas) override { return S_Success; }
    SResult CopySubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr);


private:
    void FillRenderTargetViewDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, CubeFaceType face, uint32_t mip_level = 0);
    void FillDepthStencilViewDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, CubeFaceType face);
    void FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) override;

private:
    void FillTextureDesc(D3D11_TEXTURE2D_DESC& desc) override;
    std::map<uint32_t, std::vector<ID3D11RenderTargetViewPtr>> m_mCubeRTV;
    std::vector<ID3D11DepthStencilViewPtr>   m_vCubeDSV = { nullptr };

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

    SResult CopySubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Box<uint32_t>* box = nullptr);

private:
    void FillTexture3DDesc(D3D11_TEXTURE3D_DESC& desc);
    void FillRenderTargetViewDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc) override;
    void FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) override;
    void FillStageTexture3DDesc(D3D11_TEXTURE3D_DESC& desc);
    D3D11_TEXTURE3D_DESC m_d3dTexture3DDesc = {};
};
using D3D11Texture3DPtr = std::shared_ptr<D3D11Texture3D>;
SEEK_NAMESPACE_END
