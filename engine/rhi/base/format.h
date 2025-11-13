#pragma once
#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

enum class VertexFormat : uint8_t
{
    Unknown = 0,
    UChar2,
    UChar3,
    UChar4,
    Char2,
    Char3,
    Char4,
    UChar2Normalized,
    UChar3Normalized,
    UChar4Normalized,
    Char2Normalized,
    Char3Normalized,
    Char4Normalized,

    UShort2,
    UShort3,
    UShort4,
    Short2,
    Short3,
    Short4,
    UShort2Normalized,
    UShort3Normalized,
    UShort4Normalized,
    Short2Normalized,
    Short3Normalized,
    Short4Normalized,

    Half2,
    Half3,
    Half4,

    Float,
    Float2,
    Float3,
    Float4,

    UInt,
    UInt2,
    UInt3,
    UInt4,
    Int,
    Int2,
    Int3,
    Int4,
};

enum class PixelFormat : uint8_t
{
    Unknown = 0,

    // 8-bit
    R8_UNORM,
    R8_UINT,

    // 16-bit
    R16_UINT,
    R16_SINT,
    R8G8_UINT,
    R8G8_SINT,
    R8G8_UNORM,

    // 32-bit
    R32F,
    R32_UINT,
    R32_SINT,
    R8G8B8A8_UNORM,
    R8G8B8A8_UNORM_SRGB,
    B8G8R8A8_UNORM,
    B8G8R8A8_UNORM_SRGB,
    R8G8B8A8_UINT,
    R16G16_SNORM,
    R8G8_B8G8_UNORM,
    G8R8_G8B8_UNORM,

    // 64-bit
    R32G32F,
    R16G16B16A16_UNORM,
    R16G16B16A16_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_FLOAT,

    // 128-bit
    R32G32B32A32_FLOAT,

    // Depth
    D16,    // ios not support
    D24S8,  // ios not support
    D32F,

    Num,
};

enum class ColorSpace : uint32_t
{
    Unknown = 0,
    sRGB,

    Num,
};

class Formatutil
{
public:
    static uint32_t NumComponentBytes(VertexFormat vf);
    static uint32_t NumComponentCount(VertexFormat vf);

    static uint32_t NumComponentBytes(PixelFormat pf);
    static uint32_t NumComponentCount(PixelFormat pf);
    static bool     IsDepthFormat(PixelFormat pf);
};

SEEK_NAMESPACE_END
