#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "math/matrix.h"
SEEK_NAMESPACE_BEGIN

namespace reflect
{
    enum class DataType
    {
        Unknown,
        Scalar,
        Array,
        Struct,
    };

    struct MemberInfo;
    struct StructInfo;

    struct ArrayInfo
    {
        DataType element_type;
        size_t element_size;
        size_t element_count;

        StructInfo const* struct_info; // valid when element_type is DataType::Struct
    };

    struct MemberInfo
    {
        const char* name;
        size_t offset;
        size_t size;
        DataType type;

        ArrayInfo array_info; // valid when type is DataType::Array
        StructInfo const* struct_info; // valid when type is DataType::Struct
    };

    struct StructInfo
    {
        const char* name;
        MemberInfo const* members;
    };

    template <typename T>
    struct is_scalar
    {
        static constexpr const bool value = std::is_scalar<T>::value;
    };

    template <typename T>
    struct is_struct
    {
        static constexpr const bool value = std::is_class<T>::value;
    };

    template <typename T>
    struct is_array
    {
        static constexpr const bool value = std::is_array<T>::value;
    };

    template <typename T>
    struct StructInfoTraits
    {
        static constexpr StructInfo const* value = nullptr;
    };

    template <typename T, typename N = void>
    struct DataTypeHelper
    {
        static constexpr const DataType value = DataType::Unknown;
    };

    template <typename T>
    struct DataTypeHelper<T, typename std::enable_if<is_scalar<T>::value, bool>::type>
    {
        static constexpr const DataType value = DataType::Scalar;
    };

    template <typename T>
    struct DataTypeHelper<T, typename std::enable_if<is_struct<T>::value, bool>::type>
    {
        static constexpr const DataType value = DataType::Struct;
    };

    template <typename T>
    struct DataTypeHelper<T, typename std::enable_if<is_array<T>::value, bool>::type>
    {
        static constexpr const DataType value = DataType::Array;
    };

    template <bool, typename T>
    struct ArrayInfoHelper
    {
        static constexpr DataType element_type()
        {
            return DataType::Unknown;
        }

        static constexpr size_t element_size()
        {
            return 0;
        }

        static constexpr size_t element_count()
        {
            return 0;
        }

        static constexpr StructInfo const* element_struct_info()
        {
            return nullptr;
        }

        static constexpr ArrayInfo value()
        {
            return { element_type(), element_size(), element_count(), element_struct_info() };
        }
    };

    template <typename T>
    struct ArrayInfoHelper<true, T>
    {
        using ElementType = typename std::remove_reference<decltype(std::declval<T>()[0])>::type;

        static constexpr DataType element_type()
        {
            return DataTypeHelper<ElementType, bool>::value;
        }

        static constexpr size_t element_size()
        {
            return sizeof(ElementType);
        }

        static constexpr size_t element_count()
        {
            return sizeof(T) / element_size();
        }

        static constexpr StructInfo const* element_struct_info()
        {
            return StructInfoTraits<ElementType>::value;
        }

        static constexpr ArrayInfo value()
        {
            return { element_type(), element_size(), element_count(), element_struct_info() };
        }
    };

    template <typename T>
    using ArrayInfoHelper_t = ArrayInfoHelper<std::is_array<T>::value, T>;


    //////////////////////////////////////////////////////////////////////////////////////
    template <>
    struct is_scalar<int2>
    {
        static constexpr const bool value = true;
    };

    template <>
    struct is_scalar<int3>
    {
        static constexpr const bool value = true;
    };

    template <>
    struct is_scalar<int4>
    {
        static constexpr const bool value = true;
    };

    template <>
    struct is_scalar<uint2>
    {
        static constexpr const bool value = true;
    };

    template <>
    struct is_scalar<uint3>
    {
        static constexpr const bool value = true;
    };

    template <>
    struct is_scalar<uint4>
    {
        static constexpr const bool value = true;
    };

    template <>
    struct is_scalar<float2>
    {
        static constexpr const bool value = true;
    };

    template <>
    struct is_scalar<float3>
    {
        static constexpr const bool value = true;
    };

    template <>
    struct is_scalar<float4>
    {
        static constexpr const bool value = true;
    };

    template <>
    struct is_scalar<Matrix3>
    {
        static constexpr const bool value = true;
    };

    template <>
    struct is_scalar<Matrix4>
    {
        static constexpr const bool value = true;
    };

    //////////////////////////////////////////////////////////////////////////////////////

    template <>
    struct is_struct<int2>
    {
        static constexpr const bool value = false;
    };

    template <>
    struct is_struct<int3>
    {
        static constexpr const bool value = false;
    };

    template <>
    struct is_struct<int4>
    {
        static constexpr const bool value = false;
    };

    template <>
    struct is_struct<uint2>
    {
        static constexpr const bool value = false;
    };

    template <>
    struct is_struct<uint3>
    {
        static constexpr const bool value = false;
    };

    template <>
    struct is_struct<uint4>
    {
        static constexpr const bool value = false;
    };

    template <>
    struct is_struct<float2>
    {
        static constexpr const bool value = false;
    };

    template <>
    struct is_struct<float3>
    {
        static constexpr const bool value = false;
    };

    template <>
    struct is_struct<float4>
    {
        static constexpr const bool value = false;
    };

    template <>
    struct is_struct<Matrix3>
    {
        static constexpr const bool value = false;
    };

    template <>
    struct is_struct<Matrix4>
    {
        static constexpr const bool value = false;
    };

    //////////////////////////////////////////////////////////////////////////////////////


}

#define STRUCT_INFO_DEF(TYPE) __reflect_##TYPE##_struct_info
#define STRUCT_MEMBERS_INFO_DEF(TYPE) __reflect_##TYPE##_struct_members_info

#define REFLECT_STRUCT_INFO_BEGIN(TYPE) \
    static const reflect::MemberInfo STRUCT_MEMBERS_INFO_DEF(TYPE)[] = {

#define REFLECT_STRUCT_MEMBER(TYPE, MEMBER) { \
        #MEMBER, offsetof(TYPE, MEMBER), sizeof(TYPE::MEMBER), reflect::DataTypeHelper<decltype(TYPE::MEMBER), bool>::value, \
        reflect::ArrayInfoHelper_t<decltype(TYPE::MEMBER)>::value(), \
        reflect::StructInfoTraits<decltype(TYPE::MEMBER)>::value \
    },

#define REFLECT_STRUCT_INFO_END(TYPE) \
    { nullptr } }; \
    static const reflect::StructInfo STRUCT_INFO_DEF(TYPE) = { \
        #TYPE, \
        STRUCT_MEMBERS_INFO_DEF(TYPE) \
    }; \
    namespace reflect { \
        template <> struct StructInfoTraits<TYPE> \
        { \
            static constexpr StructInfo const* value = &STRUCT_INFO_DEF(TYPE); \
        }; \
    }


struct SampleChildStruct
{
    float3 light_dir[3];
    float3 light_pos;
    uint32_t light_type;
};

struct SampleStruct
{
    float4 albedo_factor;
    int has_albedo_tex;
    Matrix4 mvp;
    SampleChildStruct light;
};

REFLECT_STRUCT_INFO_BEGIN(SampleChildStruct)
    REFLECT_STRUCT_MEMBER(SampleChildStruct, light_dir)
    REFLECT_STRUCT_MEMBER(SampleChildStruct, light_pos)
    REFLECT_STRUCT_MEMBER(SampleChildStruct, light_type)
REFLECT_STRUCT_INFO_END(SampleChildStruct)

REFLECT_STRUCT_INFO_BEGIN(SampleStruct)
    REFLECT_STRUCT_MEMBER(SampleStruct, albedo_factor)
    REFLECT_STRUCT_MEMBER(SampleStruct, has_albedo_tex)
    REFLECT_STRUCT_MEMBER(SampleStruct, mvp)
    REFLECT_STRUCT_MEMBER(SampleStruct, light)
REFLECT_STRUCT_INFO_END(SampleStruct)

SEEK_NAMESPACE_END
