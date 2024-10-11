#pragma once
#include "kernel/kernel.h"
#include "rhi/base/format.h"
#include "utils/util.h"

SEEK_NAMESPACE_BEGIN

// flags for all gpu resources: buffer, texture
using ResourceFlags = uint64_t;

#define RESOURCE_FLAG_NONE                  UINT64_C(0)

//#define RESOURCE_FLAG_CPU_READ              UINT64_C(0x0000000000000001)
#define RESOURCE_FLAG_CPU_WRITE             UINT64_C(0x0000000000000002)
#define RESOURCE_FLAG_CPU_MASK              UINT64_C(0x0000000000000003)
//#define RESOURCE_FLAG_GPU_READ              UINT64_C(0x0000000000000004)
#define RESOURCE_FLAG_GPU_WRITE             UINT64_C(0x0000000000000008)
#define RESOURCE_FLAG_GPU_MASK              UINT64_C(0x000000000000000C)
#define RESOURCE_FLAG_CPU_GPU_MASK          UINT64_C(0x000000000000000F)
#define RESOURCE_FLAG_COPY_BACK             UINT64_C(0x0000000000000010)

#define RESOURCE_FLAG_RENDER_TARGET         UINT64_C(0x0000000000000100)

#define RESOURCE_FLAG_SHADER_RESOURCE       UINT64_C(0x0000000000010000)
#define RESOURCE_FLAG_SHADER_WRITE          UINT64_C(0x0000000000020000)
#define RESOURCE_FLAG_GENERATE_MIPS         UINT64_C(0x0000000000040000)
#define RESOURCE_FLAG_DRAW_INDIRECT_ARGS    UINT64_C(0x0000000000080000)

struct RHIRenderBufferData
{
    RHIRenderBufferData(uint32_t size, const void* data) :m_iDataSize(size), m_pData(data) { }
    RHIRenderBufferData(size_t size, const void* data) :m_iDataSize((uint32_t)size), m_pData(data) { }
    const void*                 m_pData = nullptr;
    uint32_t                    m_iDataSize = 0;
};

enum class VertexElementUsage : uint8_t
{
    Unknown,
    Position,
    TexCoord,
    Normal,
    Color,
    BlendWeight,
    BlendIndex,
    Tangent,
    Binormal,
    Instance,
};

enum class IndexBufferType : uint8_t
{
    Unknown,
    UInt16,
    UInt32,
};

enum class MeshTopologyType : uint8_t
{
    Points,
    Lines,
    Line_Strip,
    Triangles,
    Triangle_Strip,
    
    Unknown = 0xFF,
};

struct VertexStreamLayout
{
    uint32_t                buffer_offset = 0;
    uint32_t                usage_index = 0;
    uint32_t                instance_divisor = 1;
    VertexFormat            format = VertexFormat::Unknown;
    VertexElementUsage      usage = VertexElementUsage::Position;    
    uint8_t                 is_instance_attrib = false;
    uint8_t                 pad;
};

struct VertexStream
{
    RHIRenderBufferPtr              render_buffer = nullptr;
    uint32_t                        offset = 0;
    uint32_t                        stride = 0;
    std::vector<VertexStreamLayout> layouts;
    uint8_t                         is_instance_stream = false;
};

//#define MAX_VertexStreamLayout 8
//struct VertexStreamInfo
//{
//    uint32_t                        offset = 0;
//    uint32_t                        stride = 0;
//    uint32_t                        num_vertex_stream_layout;
//    VertexStreamLayout              layouts[MAX_VertexStreamLayout];
//    bool                            is_instance_stream = false;
//};

enum class TextureFormatSupportType : uint8_t
{
    Filtering,
    Write,
    RenderTarget,
    MSAA,
    Num,
};

#define CAP_MAX_TEXTURE_SAMPLE_COUNT 16
struct CapabilitySet
{
    bool        TextureSampleCountSupport[CAP_MAX_TEXTURE_SAMPLE_COUNT+1] = { false };
    uint8_t     maxRenderTargetCount = 8;
    bool        TextureSupport[to_underlying(PixelFormat::Num)][to_underlying(TextureFormatSupportType::Num)] = { {false} };

    bool        IsTextureSupport(PixelFormat pixel_format, TextureFormatSupportType type) const { return TextureSupport[(uint32_t)pixel_format][(uint32_t)type]; }
};

#undef None
enum class MorphTargetType
{
    None,
    Position,
    PositionNormal,
};

struct MorphInfo
{
    MorphTargetType             morph_target_type = MorphTargetType::None;
    RHIRenderBufferPtr             render_buffer = nullptr;
    std::vector<float>          morph_target_weights;
    std::vector<std::string>    morph_target_names;
};

enum class AlphaMode : uint8_t
{
    Opaque = 0,
    Mask,
    Blend,
};



SEEK_NAMESPACE_END
