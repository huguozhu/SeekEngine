#pragma once

#include "kernel/kernel.h"
#include "rhi/texture.h"

SEEK_NAMESPACE_BEGIN

class RenderView
{
public:
    uint32_t        Width()     const { return m_iWidth; }
    uint32_t        Height()    const { return m_iHeight; }
    PixelFormat     Format()    const { return m_ePixelFormat; }
    TexturePtr&     Texture()   { return m_pTexture; }
    bool            Valid()     const { return m_pTexture != nullptr; }

protected:
    RenderView(Context* context) : m_pContext(context) {}
    RenderView(Context* context, TexturePtr texture, uint32_t lod = 0)
        : m_pContext(context), m_pTexture(texture), m_iLod(lod)
    {
        float lod_rate = (float)(1UL << lod);
        m_iWidth = uint32_t(texture->Width() / lod_rate);
        m_iHeight = uint32_t(texture->Height() / lod_rate);
        m_ePixelFormat = texture->Format();
    }
    virtual ~RenderView() {}

    Context*        m_pContext = nullptr;
    uint32_t        m_iWidth = 0;
    uint32_t        m_iHeight = 0;
    uint32_t        m_iLod = 0;
    PixelFormat     m_ePixelFormat = PixelFormat::Unknown;
    TexturePtr      m_pTexture = nullptr;
    CubeFaceType    m_eCubeType = CubeFaceType::Num;    // usefull if TextureType = Cube
};

SEEK_NAMESPACE_END
