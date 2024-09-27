#pragma once

#include "kernel/kernel.h"
#include "rhi/base/texture.h"

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

SEEK_NAMESPACE_END
