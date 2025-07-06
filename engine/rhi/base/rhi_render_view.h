#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_texture.h"

SEEK_NAMESPACE_BEGIN

class RHIRenderView
{
public:
    uint32_t        Width()     const { return m_iWidth; }
    uint32_t        Height()    const { return m_iHeight; }
    PixelFormat     Format()    const { return m_ePixelFormat; }
    RHITexturePtr&  Texture()   { return m_pTexture; }
    bool            Valid()     const { return m_pTexture != nullptr; }

protected:
    RHIRenderView(Context* context) : m_pContext(context) {}
    RHIRenderView(Context* context, RHITexturePtr texture, uint32_t lod = 0)
        : m_pContext(context), m_pTexture(texture), m_iLod(lod)
    {
        float lod_rate = (float)(1UL << lod);
        m_iWidth = uint32_t(texture->Width() / lod_rate);
        m_iHeight = uint32_t(texture->Height() / lod_rate);
        m_ePixelFormat = texture->Format();
    }
    virtual ~RHIRenderView() {}

    Context*        m_pContext = nullptr;
    uint32_t        m_iWidth = 0;
    uint32_t        m_iHeight = 0;
    uint32_t        m_iLod = 0;
    PixelFormat     m_ePixelFormat = PixelFormat::Unknown;
    RHITexturePtr   m_pTexture = nullptr;
    CubeFaceType    m_eCubeType = CubeFaceType::Num;    // usefull if TextureType = Cube
};

class RHIShaderResourceView     // Srv
{
    RHIShaderResourceView(Context* context)
        :m_pContext(context){}

protected:
    Context*    m_pContext = nullptr;
    PixelFormat m_ePixelFormat = PixelFormat::Unknown;

    // for Texture
    RHITexturePtr   m_pTexture = nullptr;
    uint32_t        m_iFirstArrayIndex = 0;
    uint32_t        m_iNumArrays = 1;
    uint32_t        m_iFirstMipLevel = 0;
    uint32_t        m_iNumMipLevels = 1;
};

struct ViewDesc
{
    PixelFormat     m_ePixelFormat = PixelFormat::Unknown;

    // for Texture2D
    RHITexturePtr   m_pTexture = nullptr;
    uint32_t        m_iFirstArrayIndex = 0;
    uint32_t        m_iNumArrays = 1;
    uint32_t        m_iMipLevel = 0;

    // for Texture3D
    uint32_t        m_iFirstSlice = 0;
    uint32_t        m_iNumSlices = 1;

    // for TextureCube
    CubeFaceType    m_eFirstFace = CubeFaceType::Num;
    uint32_t        m_iNumFaces = 1;
};

class RHIRenderTargetView            // Rtv
{
public:
    RHIRenderTargetView(Context* context)
        :m_pContext(context) {}

protected:
    Context*    m_pContext = nullptr;
    uint32_t    m_iWidth = 0;
    uint32_t    m_iHeight = 0;
    uint32_t    m_iNumSamples = 1;

    ViewDesc    m_desc;
};

class RHIDepthStencilView      // Dsv
{
public:
    RHIDepthStencilView(Context* context)
        : m_pContext(context) {}

protected:
    Context*    m_pContext = nullptr;
    uint32_t    m_iWidth = 0;
    uint32_t    m_iHeight = 0;
    uint32_t    m_iNumSamples = 1;
    ViewDesc    m_desc;
};

class RHIUnorderedAccessView  // Uav
{
public:
    RHIUnorderedAccessView(Context* context)
        : m_pContext(context) {}

protected:
    Context*    m_pContext = nullptr;
    ViewDesc    m_desc;
};

SEEK_NAMESPACE_END
