#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_texture.h"

SEEK_NAMESPACE_BEGIN
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

    // for gpu buffer
    RHIGpuBufferPtr buffer;
    uint32_t first_elem;
    uint32_t num_elem;
};


class RHIShaderResourceView  // Srv
{
public:
    RHIShaderResourceView(Context* context)
        :m_pContext(context){}

protected:
    Context*    m_pContext = nullptr;
    ViewParam   m_Param;
    uint32_t    m_iNumMipLevels = 1;

};
using RHIShaderResourceViewPtr = std::shared_ptr<RHIShaderResourceView>;

class RHIRenderTargetView
{
public:
    RHIRenderTargetView(Context* context)
        :m_pContext(context) {}

    uint32_t        Width()     const   { return m_iWidth; }
    uint32_t        Height()    const   { return m_iHeight; }
    PixelFormat     Format()    const   { return m_Param.pixel_format; }
    RHITexturePtr&  Texture()           { return m_Param.texture; }
    uint32_t        NumSamples() const { return m_iNumSamples; }

protected:
    Context*    m_pContext = nullptr;
    uint32_t    m_iWidth = 0;
    uint32_t    m_iHeight = 0;
    PixelFormat m_ePixelFormat;
    uint32_t    m_iNumSamples = 1;
    ViewParam   m_Param;
};
using RHIRenderTargetViewPtr = std::shared_ptr<RHIRenderTargetView>;

class RHIDepthStencilView
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
    PixelFormat m_ePixelFormat;
    uint32_t    m_iNumSamples = 1;
    ViewParam   m_Param;
};
using RHIDepthStencilViewPtr = std::shared_ptr<RHIDepthStencilView>;

class RHIUnorderedAccessView
{
public:
    RHIUnorderedAccessView(Context* context)
        : m_pContext(context) {}

    virtual void Clear(float4 const& v) {};
    virtual void Clear(uint4 const& v) {};

protected:
    Context*    m_pContext = nullptr;
    ViewParam   m_Param;
};
using RHIUnorderedAccessViewPtr = std::shared_ptr<RHIUnorderedAccessView>;
SEEK_NAMESPACE_END
