/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : d3d11_texture.h
**
**      Brief                  : d3d11 texture
**
**      Additional             : including 2D/3D/Cube texture
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-05-28  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "kernel/kernel.h"
#include "rendering/texture.h"
#include "rendering_d3d11/d3d11_predeclare.h"
#include "util/log.h"

DVF_NAMESPACE_BEGIN

/******************************************************************************
* D3D11Texture
*******************************************************************************/
class D3D11Texture : public Texture
{
public:
    D3D11Texture(Context* context, const Texture::Desc& tex_desc);
    virtual ~D3D11Texture() override;

    virtual DVFResult Resolve() { return DVF_Success; }
    ID3D11Resource* GetD3DTexture() { return m_pTexture.Get(); }
    ID3D11Resource* GetD3DResolvedTexture() { return m_pResolvedTexture.Get(); }

    virtual ID3D11RenderTargetView*     GetD3DRenderTargetView();
    virtual ID3D11DepthStencilView*     GetD3DDepthStencilView();
    virtual ID3D11ShaderResourceView*   GetD3DShaderResourceView();
    virtual ID3D11UnorderedAccessView*  GetD3DUnorderedAccessView();

    virtual void FillRenderTargetViewDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc) {}
    virtual void FillDepthStencilViewDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc) {}
    virtual void FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) {}
    virtual void FillUnorderedAccessViewDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc) {}
    virtual DVFResult GenerateMipMap() override;

    DXGI_FORMAT GetD3DFormat() { return m_eDxgiFormat; }

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
    D3D11Texture2D(Context* context, const Texture::Desc& tex_desc);
    D3D11Texture2D(Context* context, ID3D11Texture2DPtr const& tex);
    virtual ~D3D11Texture2D() override;

    DVFResult Create  (std::vector<BitmapBufferPtr> const& bitmap_datas) override;
    DVFResult Update  (std::vector<BitmapBufferPtr> const& bitmap_datas) override;
    DVFResult CopyBack(BitmapBufferPtr bitmap_data, Rect<uint32_t>* rect, CubeFaceType face = CubeFaceType::Num) override;
    DVFResult Resolve() override;

protected:
    void FillRenderTargetViewDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc) override;
    void FillDepthStencilViewDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc) override;
    void FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) override;
    void FillUnorderedAccessViewDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC& desc) override;

private:
    void FillStageTextureDesc(D3D11_TEXTURE2D_DESC& desc);

    D3D11_TEXTURE2D_DESC m_textureDesc = {};
};
using D3D11Texture2DPtr = std::shared_ptr<D3D11Texture2D>;

/******************************************************************************
* D3D11TextureCube
*******************************************************************************/
class D3D11TextureCube : public D3D11Texture
{
public:
    D3D11TextureCube(Context* context, const Texture::Desc& tex_desc);

    virtual ID3D11RenderTargetView* GetD3DRenderTargetView(CubeFaceType face, uint32_t lod = 0);
    virtual ID3D11DepthStencilView* GetD3DDepthStencilView(CubeFaceType face);

    void FillRenderTargetViewDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc, CubeFaceType face, uint32_t lod = 0);
    void FillDepthStencilViewDesc(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, CubeFaceType face);
    void FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) override;

    DVFResult Create(std::vector<BitmapBufferPtr> const& bitmap_datas) override { return DVF_Success; }
    DVFResult Update(std::vector<BitmapBufferPtr> const& bitmap_datas) override { return DVF_Success; }
    DVFResult CopyBack(BitmapBufferPtr bitmap_data, Rect<uint32_t>* rect, CubeFaceType face = CubeFaceType::Num);
    DVFResult CreateCube(std::vector<BitmapBufferPtr>* bitmap_datas);

private:
    virtual void FillTextureDesc(D3D11_TEXTURE2D_DESC& desc) override;
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
    D3D11Texture3D(Context* context, const Texture::Desc& tex_desc);

    DVFResult Create(std::vector<BitmapBufferPtr> const& bitmap_datas) override;
    DVFResult Update(std::vector<BitmapBufferPtr> const& bitmap_datas) override { return DVF_Success; }
    DVFResult CopyBack(BitmapBufferPtr bitmap_data, Rect<uint32_t>* rect, CubeFaceType face = CubeFaceType::Num) { return DVF_Success; }

private:
    virtual void FillTexture3DDesc(D3D11_TEXTURE3D_DESC& desc);
    virtual void FillRenderTargetViewDesc(D3D11_RENDER_TARGET_VIEW_DESC& desc) override;
    virtual void FillShaderResourceViewDesc(D3D11_SHADER_RESOURCE_VIEW_DESC& desc) override;

};
using D3D11Texture3DPtr = std::shared_ptr<D3D11Texture3D>;
DVF_NAMESPACE_END
