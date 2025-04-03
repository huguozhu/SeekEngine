#pragma once
#include <map>
#include <string>
#include "components/animation_component.h"
#include "components/light_component.h"

SEEK_NAMESPACE_BEGIN

namespace gltf
{

#define GLTF_INVALID_INTEGER        (-1)
#define GLTF_INVALID_ENUM           (-1)
#define GLTF_INVALID_ARRAY_INDEX    (-1)

#define GLTF_MODE_POINTS            0
#define GLTF_MODE_LINES             1
#define GLTF_MODE_LINE_LOOP         2
#define GLTF_MODE_LINE_STRIP        3
#define GLTF_MODE_TRIANGLES         4
#define GLTF_MODE_TRIANGLE_STRIP    5
#define GLTF_MODE_TRIANGLE_FAN      6
#define GLTF_MODE_DEFAULT           4

#define GLTF_COMPONENT_TYPE_BYTE             5120
#define GLTF_COMPONENT_TYPE_UNSIGNED_BYTE    5121
#define GLTF_COMPONENT_TYPE_SHORT            5122
#define GLTF_COMPONENT_TYPE_UNSIGNED_SHORT   5123
#define GLTF_COMPONENT_TYPE_INT              5124
#define GLTF_COMPONENT_TYPE_UNSIGNED_INT     5125
#define GLTF_COMPONENT_TYPE_FLOAT            5126

#define GLTF_ELEMENT_TYPE_UNKNOWN  0
#define GLTF_ELEMENT_TYPE_SCALAR   1
#define GLTF_ELEMENT_TYPE_VEC2     2
#define GLTF_ELEMENT_TYPE_VEC3     3
#define GLTF_ELEMENT_TYPE_VEC4     4
#define GLTF_ELEMENT_TYPE_MAT2     5
#define GLTF_ELEMENT_TYPE_MAT3     6
#define GLTF_ELEMENT_TYPE_MAT4     7

#define GLTF_TEXTURE_FILTER_NEAREST                 9728
#define GLTF_TEXTURE_FILTER_LINEAR                  9729
#define GLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST  9984
#define GLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST   9985
#define GLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR   9986
#define GLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR    9987

#define GLTF_TEXTURE_WRAP_REPEAT            10497
#define GLTF_TEXTURE_WRAP_CLAMP_TO_EDGE     33071
#define GLTF_TEXTURE_WRAP_MIRRORED_REPEAT   33648
#define GLTF_TEXTURE_WRAP_S_DEFAULT         10497
#define GLTF_TEXTURE_WRAP_T_DEFAULT         10497

#define GLTF_BUFFERVIEW_TARGET_ARRAY_BUFFER           34962
#define GLTF_BUFFERVIEW_TARGET_ELEMENT_ARRAY_BUFFER   34963

#define GLTF_DEFAULT_BYTE_OFFSET    (0)
#define GLTF_INVALID_BYTE_STRIDE    (0)
#define GLTF_DEFAULT_NORMALIZED     false
#define GLTF_DEFAULT_TEXCOORD_INDEX (0)

#define GLB_MAGIC       0x46546C67  // "glTF"
#define GLB_CHUNK_JSON  0x4E4F534A  // "JSON"
#define GLB_CHUNK_BIN   0x004E4942  // "BIN "

struct GLBChunkHeader
{
    uint32_t chunkLength = 0;
    uint32_t chunkType = 0;
};

struct GLBChunk
{
    GLBChunkHeader chunkHeader;
    uint8_t* chunkData = nullptr;
};

struct GLBHeader
{
    uint32_t magic = 0;
    uint32_t version = 0;
    uint32_t length = 0;
};

struct GLBInfo
{
    GLBHeader glbHeader;
    GLBChunk jsonChunk;
    GLBChunk binChunk; // when chunkType == 0 means no bin chunk
};

inline MeshTopologyType ConvertToTopologyType(int mode)
{
    switch (mode)
    {
    case GLTF_MODE_POINTS:
        return MeshTopologyType::Points;
    case GLTF_MODE_LINES:
        return MeshTopologyType::Lines;
    case GLTF_MODE_LINE_STRIP:
        return MeshTopologyType::Line_Strip;
    case GLTF_MODE_TRIANGLES:
        return MeshTopologyType::Triangles;
    case GLTF_MODE_TRIANGLE_STRIP:
        return MeshTopologyType::Triangle_Strip;
    default:
        return MeshTopologyType::Unknown;
    }
}

inline IndexBufferType ConvertToIndexBufferType(int componentType)
{
    switch (componentType)
    {
    case GLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        return IndexBufferType::UInt16;
    case GLTF_COMPONENT_TYPE_UNSIGNED_INT:
        return IndexBufferType::UInt32;
    default:
        return IndexBufferType::Unknown;
    }
}

static const std::map<std::string, int> _element_type_map{
    { "SCALAR", GLTF_ELEMENT_TYPE_SCALAR},
    { "VEC2",   GLTF_ELEMENT_TYPE_VEC2},
    { "VEC3",   GLTF_ELEMENT_TYPE_VEC3},
    { "VEC4",   GLTF_ELEMENT_TYPE_VEC4},
    { "MAT2",   GLTF_ELEMENT_TYPE_MAT2},
    { "MAT3",   GLTF_ELEMENT_TYPE_MAT3},
    { "MAT4",   GLTF_ELEMENT_TYPE_MAT4},
};
inline int ConvertToElementType(const std::string& elementType)
{
    auto const& it = _element_type_map.find(elementType);
    if (it == _element_type_map.end())
        return GLTF_ELEMENT_TYPE_UNKNOWN;
    else
        return it->second;
}
inline bool IsElementTypeValid(const std::string& elementType)
{
    return _element_type_map.find(elementType) != _element_type_map.end();
}

static const int _component_num_of_data_type[] = {
    0,  // GLTF_ELEMENT_TYPE_UNKNOWN
    1,  // GLTF_ELEMENT_TYPE_SCALAR
    2,  // GLTF_ELEMENT_TYPE_VEC2
    3,  // GLTF_ELEMENT_TYPE_VEC3
    4,  // GLTF_ELEMENT_TYPE_VEC4
    4,  // GLTF_ELEMENT_TYPE_MAT2
    9,  // GLTF_ELEMENT_TYPE_MAT3
    16, // GLTF_ELEMENT_TYPE_MAT4
};
inline int ComponentNumOfElementType(int elementType)
{
    return _component_num_of_data_type[elementType];
}

static const int _component_byte_size[] = {
    1, // GLTF_COMPONENT_TYPE_BYTE
    1, // GLTF_COMPONENT_TYPE_UNSIGNED_BYTE
    2, // GLTF_COMPONENT_TYPE_SHORT
    2, // GLTF_COMPONENT_TYPE_UNSIGNED_SHORT
    4, // GLTF_COMPONENT_TYPE_INT
    4, // GLTF_COMPONENT_TYPE_UNSIGNED_INT
    4, // GLTF_COMPONENT_TYPE_FLOAT
};
inline int ComponentByteSize(int componentType)
{
    return _component_byte_size[componentType - GLTF_COMPONENT_TYPE_BYTE];
}

inline int ElementSize(int componentType, int elementType)
{
    return  ComponentNumOfElementType(elementType) * ComponentByteSize(componentType);
}

static const std::map<std::string, VertexElementUsage> _vertex_attribute_map{
    { "POSITION",   VertexElementUsage::Position},
    { "NORMAL",     VertexElementUsage::Normal},
    { "TANGENT",    VertexElementUsage::Tangent},
    { "TEXCOORD",   VertexElementUsage::TexCoord},
    { "COLOR",      VertexElementUsage::Color},
    { "JOINTS",     VertexElementUsage::BlendIndex},
    { "WEIGHTS",    VertexElementUsage::BlendWeight},
};
inline std::pair<VertexElementUsage, uint32_t> ConvertVertexAttribute(const std::string& attribute)
{
    if (attribute.size() > 2 && attribute[attribute.size() - 2] == '_')
    {
        std::string semantics = attribute.substr(0, attribute.size() - 2);
        uint32_t index = std::atoi(attribute.c_str() + attribute.size() - 1);
        return std::pair<VertexElementUsage, uint32_t>{_vertex_attribute_map.at(semantics), index};
    }
    else
        return std::pair<VertexElementUsage, uint32_t>{_vertex_attribute_map.at(attribute), 0};
}
inline bool IsVertexAttributeValie(const std::string& attribute)
{
    return _vertex_attribute_map.find(attribute) != _vertex_attribute_map.end();
}

static const std::map<std::string, InterpolationType> _animation_interpolation_type_map{
    { "LINEAR",         InterpolationType::Linear},
    { "STEP",           InterpolationType::Step},
    { "CUBICSPLINE",    InterpolationType::CubicSpline},
};
inline InterpolationType ConvertToInterpolationType(const std::string& interpolation)
{
    return _animation_interpolation_type_map.at(interpolation);
}
inline bool IsValidInterpolationType(const std::string& interpolation)
{
    return _animation_interpolation_type_map.find(interpolation) != _animation_interpolation_type_map.end();
}

static const std::map<std::string, LightType> _light_type_map{
    { "directional",    LightType::Directional},
    { "point",          LightType::Point},
    { "spot",           LightType::Spot},
};
inline LightType ConvertToLightType(const std::string& light)
{
    return _light_type_map.at(light);
}
inline bool IsValidLightType(const std::string& light)
{
    return _light_type_map.find(light) != _light_type_map.end();
}

static const VertexFormat _vertex_format_map[8][7] = {
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Int, VertexFormat::UInt, VertexFormat::Float },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Short2, VertexFormat::UShort2, VertexFormat::Int2, VertexFormat::UInt2, VertexFormat::Float2 },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Short3, VertexFormat::UShort3, VertexFormat::Int3, VertexFormat::UInt3, VertexFormat::Float3 },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Short4, VertexFormat::UShort4, VertexFormat::Int4, VertexFormat::UInt4, VertexFormat::Float4 },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown },
    { VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown, VertexFormat::Unknown },
};
inline VertexFormat ConvertToVertexFormat(int elementType, int componentType)
{
    return _vertex_format_map[elementType][componentType - GLTF_COMPONENT_TYPE_BYTE];
}
inline bool IsValidVertexFormat(int elementType, int componentType)
{
    return ConvertToVertexFormat(elementType, componentType) != VertexFormat::Unknown;
}

} // namespace gltf

SEEK_NAMESPACE_END