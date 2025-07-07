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

struct ViewParam
{
    PixelFormat     pixel_format = PixelFormat::Unknown;

    // for Textures
    RHITexturePtr   texture = nullptr;
    uint32_t        first_array_index = 0;
    uint32_t        num_arrays = 1;
    uint32_t        mip_level = 0;

    // for Texture3D
    uint32_t        first_slice = 0;
    uint32_t        num_slices = 1;

    // for TextureCube
    CubeFaceType    first_face = CubeFaceType::Num;
    uint32_t        num_faces = 1;
};

class RHIRenderTargetView            // Rtv
{
public:
    RHIRenderTargetView(Context* context)
        :m_pContext(context) {}

    uint32_t        Width()     const { return m_iWidth; }
    uint32_t        Height()    const { return m_iHeight; }
    PixelFormat     Format()    const { return m_Param.pixel_format; }
    RHITexturePtr&  Texture()         { return m_Param.texture; }

protected:
    Context*    m_pContext = nullptr;
    uint32_t    m_iWidth = 0;
    uint32_t    m_iHeight = 0;
    PixelFormat m_ePixelFormat;
    uint32_t    m_iNumSamples = 1;

    ViewParam    m_Param;
};

class RHIDepthStencilView      // Dsv
{
public:
    RHIDepthStencilView(Context* context)
        : m_pContext(context) {}

    uint32_t        Width()     const { return m_iWidth; }
    uint32_t        Height()    const { return m_iHeight; }
    PixelFormat     Format()    const { return m_Param.pixel_format; }
    RHITexturePtr&  Texture()         { return m_Param.texture; }

protected:
    Context*    m_pContext = nullptr;
    uint32_t    m_iWidth = 0;
    uint32_t    m_iHeight = 0;
    uint32_t    m_iNumSamples = 1;
    ViewParam   m_Param;
};

class RHIUnorderedAccessView  // Uav
{
public:
    RHIUnorderedAccessView(Context* context)
        : m_pContext(context) {}

protected:
    Context*    m_pContext = nullptr;
    ViewParam   m_Param;
};

SEEK_NAMESPACE_END
