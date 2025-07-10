#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_definition.h"
#include "math/rect.h"
#include "math/box.h"
#include "utils/buffer.h"
#include "utils/util.h"
#include "math/math_utility.h"
#include <span>

SEEK_NAMESPACE_BEGIN

enum class TextureType : uint8_t
{
    None,
    Tex2D,
    Tex3D,
    Cube,
};

enum class CubeFaceType : uint8_t
{
    Positive_X,
    Negative_X,
    Positive_Y,
    Negative_Y,
    Positive_Z,
    Negative_Z,
    Num,
};

class RHITexture
{
public:
    struct Desc
    {
        TextureType type = TextureType::None;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
        uint32_t num_mips = 1;          // if = 0 --> create max mipmap auto
        uint32_t num_samples = 1;
        uint32_t num_array = 1;
        PixelFormat format = PixelFormat::Unknown;
        ResourceFlags flags = RESOURCE_FLAG_NONE;
    };

    Desc const&         Descriptor()    const { return m_desc; };
    TextureType         Type()          const { return m_desc.type; }
    uint32_t            Width(uint32_t mip_level = 0)   const { SEEK_ASSERT(mip_level < m_desc.num_mips); return std::max(1U, m_desc.width  >> mip_level); }
    uint32_t            Height(uint32_t mip_level = 0)  const { SEEK_ASSERT(mip_level < m_desc.num_mips); return std::max(1U, m_desc.height >> mip_level); }
    uint32_t            Depth(uint32_t mip_level = 0)   const { SEEK_ASSERT(mip_level < m_desc.num_mips); return std::max(1U, m_desc.depth  >> mip_level); }
    uint32_t            NumMips()       const { return m_desc.num_mips; }
    uint32_t            NumSamples()    const { return m_desc.num_samples; }
    uint32_t            NumArray()      const { return m_desc.num_array; }
    PixelFormat         Format()        const { return m_desc.format; }
    ResourceFlags       Flags()         const { return m_desc.flags; }

    virtual SResult     Create(std::span<BitmapBufferPtr> const& bitmap_datas) = 0;
    virtual SResult     Update(std::span<BitmapBufferPtr> const& bitmap_datas) = 0;
    virtual SResult     GenerateMipMap() { return S_Success; }

    virtual SResult DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) = 0;
    virtual SResult DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Box<uint32_t>* box = nullptr) = 0;
    virtual SResult DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) = 0;

    SResult DumpToFile(std::string path, BitmapBufferPtr bitmap_data)
    {
        if (!bitmap_data)
            return -1;
        bitmap_data->DumpToFile(path);
        return S_Success;
    }

    SResult DumpToFile(std::string path, CubeFaceType face = CubeFaceType::Num)
    {
        BitmapBufferPtr bitmap_data = MakeSharedPtr<BitmapBuffer>();
        switch (m_desc.type)
        {
        case TextureType::Tex2D:
        {
            SEEK_RETIF_FAIL(this->DumpSubResource2D(bitmap_data));
            break;
        }
        case TextureType::Tex3D:
        {
            SEEK_RETIF_FAIL(this->DumpSubResource3D(bitmap_data));
            break;
        }
        case TextureType::Cube:
        {
            SEEK_RETIF_FAIL(this->DumpSubResourceCube(bitmap_data, face));
            break;
        }
        }
        
        bitmap_data->DumpToFile(path);
        return S_Success;
    }

protected:
    RHITexture(Context* context, const Desc& desc)
        : m_pContext(context)
        , m_desc(desc)
    {}
    virtual ~RHITexture() {}

    Context*                m_pContext = nullptr;
    Desc                    m_desc;
};

SEEK_NAMESPACE_END
