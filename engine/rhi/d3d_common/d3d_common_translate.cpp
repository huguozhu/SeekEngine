#include "rhi/d3d_common/d3d_common_translate.h"

#include "utils/log.h"

SEEK_NAMESPACE_BEGIN

PixelFormat D3DCommonTranslate::TranslateFromPlatformFormat(DXGI_FORMAT format)
{
    PixelFormat res = PixelFormat::Unknown;
    switch (format)
    {
        // 8-bit
    case DXGI_FORMAT_R8_UNORM:              res = PixelFormat::R8_UNORM;            break;
    case DXGI_FORMAT_R8_UINT:               res = PixelFormat::R8_UINT;             break;

        // 16-bit
    case DXGI_FORMAT_R16_UINT:              res = PixelFormat::R16_UINT;            break;
    case DXGI_FORMAT_R16_SINT:              res = PixelFormat::R16_SINT;            break;
    case DXGI_FORMAT_R8G8_UINT:             res = PixelFormat::R8G8_UINT;           break;
    case DXGI_FORMAT_R8G8_SINT:             res = PixelFormat::R8G8_SINT;           break;

        // 32-bit
    case DXGI_FORMAT_R32_FLOAT:             res = PixelFormat::R32F;                break;
    case DXGI_FORMAT_R32_UINT:              res = PixelFormat::R32_UINT;            break;
    case DXGI_FORMAT_R32_SINT:              res = PixelFormat::R32_SINT;            break;
    case DXGI_FORMAT_R8G8B8A8_UNORM:        res = PixelFormat::R8G8B8A8_UNORM;      break;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   res = PixelFormat::R8G8B8A8_UNORM_SRGB; break;
    case DXGI_FORMAT_B8G8R8A8_UNORM:        res = PixelFormat::B8G8R8A8_UNORM;      break;
    case DXGI_FORMAT_R8G8B8A8_UINT:         res = PixelFormat::R8G8B8A8_UINT;       break;
    case DXGI_FORMAT_R16G16_SNORM:          res = PixelFormat::R16G16_SNORM;        break;
        // 64-bit
    case DXGI_FORMAT_R32G32_FLOAT:          res = PixelFormat::R32G32F;             break;
    case DXGI_FORMAT_R16G16B16A16_UNORM:    res = PixelFormat::R16G16B16A16_UNORM;  break;
    case DXGI_FORMAT_R16G16B16A16_UINT:     res = PixelFormat::R16G16B16A16_UINT;   break;
    case DXGI_FORMAT_R16G16B16A16_SINT:     res = PixelFormat::R16G16B16A16_SINT;   break;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:    res = PixelFormat::R16G16B16A16_FLOAT;  break;

        // 128-bit
    case DXGI_FORMAT_R32G32B32A32_FLOAT:    res = PixelFormat::R32G32B32A32_FLOAT;  break;

        // depth
    case DXGI_FORMAT_R16_TYPELESS:          res = PixelFormat::D16;                 break;
    case DXGI_FORMAT_R24G8_TYPELESS:        res = PixelFormat::D24S8;               break;
    case DXGI_FORMAT_R32_TYPELESS:          res = PixelFormat::D32F;                break;
    }
    return res;
}
DXGI_FORMAT D3DCommonTranslate::TranslateToPlatformFormat(PixelFormat format)
{
    DXGI_FORMAT res = DXGI_FORMAT_UNKNOWN;
    switch (format)
    {
        // 8-bit
    case PixelFormat::R8_UNORM:             res = DXGI_FORMAT_R8_UNORM;             break;
    case PixelFormat::R8_UINT:              res = DXGI_FORMAT_R8_UINT;              break;

        // 16-bit
    case PixelFormat::R16_UINT:             res = DXGI_FORMAT_R16_UINT;             break;
    case PixelFormat::R16_SINT:             res = DXGI_FORMAT_R16_SINT;             break;
    case PixelFormat::R8G8_UINT:            res = DXGI_FORMAT_R8G8_UINT;            break;
    case PixelFormat::R8G8_SINT:            res = DXGI_FORMAT_R8G8_SINT;            break;
    case PixelFormat::R8G8_UNORM:           res = DXGI_FORMAT_R8G8_UNORM;           break;

        // 32-bit
    case PixelFormat::R32F:                 res = DXGI_FORMAT_R32_FLOAT;            break;
    case PixelFormat::R32_UINT:             res = DXGI_FORMAT_R32_UINT;             break;
    case PixelFormat::R32_SINT:             res = DXGI_FORMAT_R32_SINT;             break;
    case PixelFormat::R8G8B8A8_UNORM:       res = DXGI_FORMAT_R8G8B8A8_UNORM;       break;
    case PixelFormat::R8G8B8A8_UNORM_SRGB:  res = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;  break;
    case PixelFormat::B8G8R8A8_UNORM:       res = DXGI_FORMAT_B8G8R8A8_UNORM;       break;
    case PixelFormat::R8G8B8A8_UINT:        res = DXGI_FORMAT_R8G8B8A8_UINT;        break;
    case PixelFormat::R16G16_SNORM:         res = DXGI_FORMAT_R16G16_SNORM;         break;
    case PixelFormat::R8G8_B8G8_UNORM:      res = DXGI_FORMAT_R8G8_B8G8_UNORM;      break;
    case PixelFormat::G8R8_G8B8_UNORM:      res = DXGI_FORMAT_G8R8_G8B8_UNORM;      break;

        // 64-bit
    case PixelFormat::R32G32F:              res = DXGI_FORMAT_R32G32_FLOAT;         break;
    case PixelFormat::R16G16B16A16_UNORM:   res = DXGI_FORMAT_R16G16B16A16_UNORM;   break;
    case PixelFormat::R16G16B16A16_UINT:    res = DXGI_FORMAT_R16G16B16A16_UINT;    break;
    case PixelFormat::R16G16B16A16_SINT:    res = DXGI_FORMAT_R16G16B16A16_SINT;    break;
    case PixelFormat::R16G16B16A16_FLOAT:   res = DXGI_FORMAT_R16G16B16A16_FLOAT;   break;

        // 128-bit
    case PixelFormat::R32G32B32A32_FLOAT:   res = DXGI_FORMAT_R32G32B32A32_FLOAT;   break;

        // depth
    case PixelFormat::D16:                  res = DXGI_FORMAT_R16_TYPELESS;         break;
    case PixelFormat::D24S8:                res = DXGI_FORMAT_R24G8_TYPELESS;       break;
    case PixelFormat::D32F:                 res = DXGI_FORMAT_R32_TYPELESS;         break;
    }
    return res;
}
DXGI_FORMAT D3DCommonTranslate::TranslateToPlatformFormat(VertexFormat format)
{
    switch (format)
    {
    case VertexFormat::Unknown:            return DXGI_FORMAT_UNKNOWN;
    case VertexFormat::UChar2:             return DXGI_FORMAT_R8G8_UINT;
    //case VertexFormat::UChar3:           return DXGI_FORMAT_R8G8B8A8_UINT;
    case VertexFormat::UChar4:             return DXGI_FORMAT_R8G8B8A8_UINT;
    case VertexFormat::Char2:              return DXGI_FORMAT_R8G8_SINT;
    //case VertexFormat::Char3:            return DXGI_FORMAT_R8G8B8A8_SINT;
    case VertexFormat::Char4:              return DXGI_FORMAT_R8G8B8A8_SINT;
    case VertexFormat::UChar2Normalized:   return DXGI_FORMAT_R8G8_UNORM;
    //case VertexFormat::UChar3Normalized: return DXGI_FORMAT_R8G8B8A8_SNORM;
    case VertexFormat::UChar4Normalized:   return DXGI_FORMAT_R8G8B8A8_SNORM;
    case VertexFormat::Char2Normalized:    return DXGI_FORMAT_R8G8_SNORM;
    //case VertexFormat::Char3Normalized:  return DXGI_FORMAT_R8G8B8A8_SNORM;
    case VertexFormat::Char4Normalized:    return DXGI_FORMAT_R8G8B8A8_SNORM;

    case VertexFormat::UShort2:            return DXGI_FORMAT_R16G16_UINT;
    //case VertexFormat::UShort3:          return DXGI_FORMAT_R16G16B16A16_UINT;
    case VertexFormat::UShort4:            return DXGI_FORMAT_R16G16B16A16_UINT;
    case VertexFormat::Short2:             return DXGI_FORMAT_R16G16_SINT;
    //case VertexFormat::Short3:           return DXGI_FORMAT_R16G16B16A16_SINT;
    case VertexFormat::Short4:             return DXGI_FORMAT_R16G16B16A16_SINT;
    case VertexFormat::UShort2Normalized:  return DXGI_FORMAT_R16G16_UNORM;
    //case VertexFormat::UShort3Normalized:return DXGI_FORMAT_R16G16B16A16_UNORM;
    case VertexFormat::UShort4Normalized:  return DXGI_FORMAT_R16G16B16A16_UNORM;
    case VertexFormat::Short2Normalized:   return DXGI_FORMAT_R16G16_SNORM;
    //case VertexFormat::Short3Normalized: return DXGI_FORMAT_R16G16B16A16_SNORM;
    case VertexFormat::Short4Normalized:   return DXGI_FORMAT_R16G16B16A16_SNORM;

    case VertexFormat::Half2:              return DXGI_FORMAT_R16G16_FLOAT;
    //case VertexFormat::Half3:              return DXGI_FORMAT_R16G16B16_FLOAT;
    case VertexFormat::Half4:              return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case VertexFormat::Float:              return DXGI_FORMAT_R32_FLOAT;
    case VertexFormat::Float2:             return DXGI_FORMAT_R32G32_FLOAT;
    case VertexFormat::Float3:             return DXGI_FORMAT_R32G32B32_FLOAT;
    case VertexFormat::Float4:             return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case VertexFormat::UInt:               return DXGI_FORMAT_R32_UINT;
    case VertexFormat::UInt2:              return DXGI_FORMAT_R32G32_UINT;
    case VertexFormat::UInt3:              return DXGI_FORMAT_R32G32B32_UINT;
    case VertexFormat::UInt4:              return DXGI_FORMAT_R32G32B32A32_UINT;
    case VertexFormat::Int:                return DXGI_FORMAT_R32_SINT;
    case VertexFormat::Int2:               return DXGI_FORMAT_R32G32_SINT;
    case VertexFormat::Int3:               return DXGI_FORMAT_R32G32B32_SINT;
    case VertexFormat::Int4:               return DXGI_FORMAT_R32G32B32A32_SINT;
    default:  return DXGI_FORMAT_UNKNOWN;
    }
}

SEEK_NAMESPACE_END
