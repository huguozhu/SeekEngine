#include "rhi/format.h"

SEEK_NAMESPACE_BEGIN

uint32_t Formatutil::NumComponentBytes(VertexFormat vf)
{
    uint32_t num = 0;
    switch (vf)
    {
    case VertexFormat::UChar2:
    case VertexFormat::Char2:
    case VertexFormat::UChar2Normalized:
    case VertexFormat::Char2Normalized:
        num = 2;
        break;
    case VertexFormat::UChar3:
    case VertexFormat::Char3:
    case VertexFormat::UChar3Normalized:
    case VertexFormat::Char3Normalized:
        num = 3;
        break;
    case VertexFormat::UChar4:
    case VertexFormat::Char4:
    case VertexFormat::UChar4Normalized:
    case VertexFormat::Char4Normalized:
    case VertexFormat::UShort2:
    case VertexFormat::Short2:
    case VertexFormat::UShort2Normalized:
    case VertexFormat::Short2Normalized:
    case VertexFormat::Half2:
    case VertexFormat::Float:
    case VertexFormat::UInt:
    case VertexFormat::Int:
        num = 4;
        break;
    case VertexFormat::UShort3:
    case VertexFormat::Short3:
    case VertexFormat::UShort3Normalized:
    case VertexFormat::Short3Normalized:
    case VertexFormat::Half3:
        num = 6;
        break;
    case VertexFormat::UShort4:
    case VertexFormat::Short4:
    case VertexFormat::UShort4Normalized:
    case VertexFormat::Short4Normalized:
    case VertexFormat::Half4:
    case VertexFormat::Float2:
    case VertexFormat::UInt2:
    case VertexFormat::Int2:
        num = 8;
        break;
    case VertexFormat::Float3:
    case VertexFormat::UInt3:
    case VertexFormat::Int3:
        num = 12;
        break;
    case VertexFormat::Float4:
    case VertexFormat::UInt4:
    case VertexFormat::Int4:
        num = 16;
        break;
    case VertexFormat::Unknown:
        break;
    }
    return num;
}

uint32_t Formatutil::NumComponentCount(VertexFormat vf)
{
    uint32_t count = 0;
    switch (vf)
    {
    case VertexFormat::Float:
    case VertexFormat::UInt:
    case VertexFormat::Int:
        count = 1;
        break;
    case VertexFormat::UChar2:
    case VertexFormat::Char2:
    case VertexFormat::UChar2Normalized:
    case VertexFormat::Char2Normalized:
    case VertexFormat::UShort2:
    case VertexFormat::Short2:
    case VertexFormat::UShort2Normalized:
    case VertexFormat::Short2Normalized:
    case VertexFormat::Half2:
    case VertexFormat::Float2:
    case VertexFormat::UInt2:
    case VertexFormat::Int2:
        count = 2;
        break;
    case VertexFormat::UChar3:
    case VertexFormat::Char3:
    case VertexFormat::UChar3Normalized:
    case VertexFormat::Char3Normalized:
    case VertexFormat::UShort3:
    case VertexFormat::Short3:
    case VertexFormat::UShort3Normalized:
    case VertexFormat::Short3Normalized:
    case VertexFormat::Half3:
    case VertexFormat::Float3:
    case VertexFormat::UInt3:
    case VertexFormat::Int3:
        count = 3;
        break;
    case VertexFormat::UChar4:
    case VertexFormat::Char4:
    case VertexFormat::UChar4Normalized:
    case VertexFormat::Char4Normalized:
    case VertexFormat::UShort4:
    case VertexFormat::Short4:
    case VertexFormat::UShort4Normalized:
    case VertexFormat::Short4Normalized:
    case VertexFormat::Half4:
    case VertexFormat::Float4:
    case VertexFormat::UInt4:
    case VertexFormat::Int4:
        count = 4;
        break;
    case VertexFormat::Unknown:
        break;
    }
    return count;
}

uint32_t Formatutil::NumComponentBytes(PixelFormat pf)
{
    switch (pf)
    {
        // 8-bit
        case PixelFormat::R8_UNORM:
        case PixelFormat::R8_UINT:
            return 1;
        // 16-bit
        case PixelFormat::R16_UINT:
        case PixelFormat::R16_SINT:
        case PixelFormat::R8G8_UINT:
        case PixelFormat::R8G8_SINT:
        case PixelFormat::R8G8_UNORM:
            return 2;
        // 32-bit
        case PixelFormat::R32F:
        case PixelFormat::R32_UINT:
        case PixelFormat::R32_SINT:
        case PixelFormat::R8G8B8A8_UNORM:
        case PixelFormat::R8G8B8A8_UNORM_SRGB:
        case PixelFormat::B8G8R8A8_UNORM:
        case PixelFormat::B8G8R8A8_UNORM_SRGB:
        case PixelFormat::R8G8B8A8_UINT:
        case PixelFormat::R16G16_SNORM:
        case PixelFormat::R8G8_B8G8_UNORM:
        case PixelFormat::G8R8_G8B8_UNORM:
            return 4;
        // 64-bit
        case PixelFormat::R32G32F:
        case PixelFormat::R16G16B16A16_UNORM:
        case PixelFormat::R16G16B16A16_UINT:
        case PixelFormat::R16G16B16A16_SINT:
        case PixelFormat::R16G16B16A16_FLOAT:
            return 8;
        // 128-bit
        case PixelFormat::R32G32B32A32_FLOAT:
            return 16;
        // Depth
        case PixelFormat::D16:      return 2;
        case PixelFormat::D24S8:    return 4;
        case PixelFormat::D32F:     return 4;
        case PixelFormat::Unknown:
        case PixelFormat::Num:
            return 0;
    }
    return 0;
}

uint32_t Formatutil::NumComponentCount(PixelFormat pf)
{
    uint32_t count = 0;
    switch (pf)
    {
        // 8-bit
        case PixelFormat::R8_UNORM:             count = 1; break;
        case PixelFormat::R8_UINT:              count = 1; break;
        // 16-bit
        case PixelFormat::R16_UINT:             count = 1; break;
        case PixelFormat::R16_SINT:             count = 1; break;
        case PixelFormat::R8G8_UINT:            count = 2; break;
        case PixelFormat::R8G8_SINT:            count = 2; break;
        case PixelFormat::R8G8_UNORM:           count = 2; break;
        // 32-bit
        case PixelFormat::R32F:                 count = 1; break;
        case PixelFormat::R32_UINT:             count = 1; break;
        case PixelFormat::R32_SINT:             count = 1; break;
        case PixelFormat::R8G8B8A8_UNORM:       count = 4; break;
        case PixelFormat::R8G8B8A8_UNORM_SRGB:  count = 4; break;
        case PixelFormat::B8G8R8A8_UNORM:       count = 4; break;
        case PixelFormat::B8G8R8A8_UNORM_SRGB:  count = 4; break;
        case PixelFormat::R8G8B8A8_UINT:        count = 4; break;
        case PixelFormat::R16G16_SNORM:         count = 2; break;
        case PixelFormat::R8G8_B8G8_UNORM:      count = 4; break;
        case PixelFormat::G8R8_G8B8_UNORM:      count = 4; break;
        // 64-bit
        case PixelFormat::R32G32F:              count = 2; break;
        case PixelFormat::R16G16B16A16_UNORM:   count = 4; break;
        case PixelFormat::R16G16B16A16_UINT:    count = 4; break;
        case PixelFormat::R16G16B16A16_SINT:    count = 4; break;
        case PixelFormat::R16G16B16A16_FLOAT:   count = 4; break;
        // 128-bit
        case PixelFormat::R32G32B32A32_FLOAT:   count = 4; break;
        // Depth
        case PixelFormat::D16:                  count = 1; break;
        case PixelFormat::D24S8:                count = 2; break;
        case PixelFormat::D32F:                 count = 1; break;
        case PixelFormat::Unknown:
        case PixelFormat::Num:                               count = 0; break;
    }
    return count;
}

bool Formatutil::IsDepthFormat(PixelFormat pf)
{
    bool res = false;
    switch (pf)
    {
    case PixelFormat::D16:
    case PixelFormat::D24S8:
    case PixelFormat::D32F:
        res = true;
    default: break;
    }
    return res;
}

SEEK_NAMESPACE_END
