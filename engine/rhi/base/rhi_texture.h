#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_definition.h"
#include "math/rect.h"
#include "utils/buffer.h"

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
        uint32_t num_mips = 1;
        uint32_t num_samples = 1;
        PixelFormat format = PixelFormat::Unknown;
        ResourceFlags flags = RESOURCE_FLAG_NONE;
    };

    Desc const&         Descriptor()    const { return m_desc; };
    TextureType         Type()          const { return m_desc.type; }
    uint32_t            SizeX()         const { return m_desc.width; }
    uint32_t            SizeY()         const { return m_desc.height; }
    uint32_t            SizeZ()         const { return m_desc.depth; }
    uint32_t            Width()         const { return m_desc.width; }
    uint32_t            Height()        const { return m_desc.height; }
    uint32_t            Depth()         const { return m_desc.depth; }
    uint32_t            NumMips()       const { return m_desc.num_mips; }
    uint32_t            NumSamples()    const { return m_desc.num_samples; }
    PixelFormat         Format()        const { return m_desc.format; }
    ResourceFlags       Flags()         const { return m_desc.flags; }

    virtual SResult     Create(std::vector<BitmapBufferPtr> const& bitmap_datas) = 0;
    virtual SResult     Update(std::vector<BitmapBufferPtr> const& bitmap_datas) = 0;
    virtual SResult     CopyBack(BitmapBufferPtr bitmap_data, Rect<uint32_t>* rect, CubeFaceType face = CubeFaceType::Num) { return S_Success; }
    virtual SResult     CopyBackTexture3D(BitmapBufferPtr bitmap_data, uint32_t array_index, uint32_t mip_level) { return S_Success; }
    virtual SResult     GenerateMipMap() { return S_Success; }

    SResult DumpToFile(std::string path, CubeFaceType face = CubeFaceType::Num)
    {
        BitmapBufferPtr bitmap_data = MakeSharedPtr<BitmapBuffer>();
        Rect<uint32_t> rect;
        rect.left = rect.top = 0;
        rect.right = m_desc.width;
        rect.bottom = m_desc.height;
        SEEK_RETIF_FAIL(this->CopyBack(bitmap_data, &rect, face));
        bitmap_data->DumpToFile(path);
        return S_Success;
    }

    SResult DumpToFile(std::string path, uint32_t array_index, uint32_t mip_level)
    {
        BitmapBufferPtr bitmap_data = MakeSharedPtr<BitmapBuffer>();
        SEEK_RETIF_FAIL(this->CopyBackTexture3D(bitmap_data, array_index, mip_level));
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
