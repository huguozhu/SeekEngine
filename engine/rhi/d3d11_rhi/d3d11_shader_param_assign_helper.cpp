#include "rhi/d3d11_rhi/d3d11_shader_param_assign_helper.h"
#include "utils/error.h"
#include "utils/util.h"

#define SEEK_MACRO_FILE_UID 69     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

SResult CBufferAssignHelper::ShaderParamAssign(void* dstBuf, const void* srcBuf, reflect::StructInfo const* structInfo, ID3D11ShaderReflectionType* structType, size_t structSize)
{
    HRESULT hr;

    D3D11_SHADER_TYPE_DESC structTypeDesc;
    hr = structType->GetDesc(&structTypeDesc);
    if (FAILED(hr)) {
        LOG_ERROR("ID3D11ShaderReflectionType::GetDesc fail, hr:%x", hr);
        return ERR_UNKNOWN;
    }

    for (UINT memberIdx = 0; memberIdx < structTypeDesc.Members; memberIdx++)
    {
        reflect::MemberInfo const* memberInfo = &structInfo->members[memberIdx];
        if (!memberInfo->name) {
            LOG_ERROR("No.%u member info is null for struct(%s)", memberIdx, structInfo->name);
            return ERR_INVALID_ARG;
        }

        if (strcmp(memberInfo->name, structType->GetMemberTypeName(memberIdx)) != 0) {
            LOG_ERROR("No.%u member is different between CPU(%s) and GPU(%s)", memberIdx, memberInfo->name, structType->GetMemberTypeName(memberIdx));
            return ERR_INVALID_ARG;
        }

        D3D11_SHADER_TYPE_DESC memberTypeDesc;
        structType->GetMemberTypeByIndex(memberIdx)->GetDesc(&memberTypeDesc);

        // FIXME: this memberSize contains the padding bytes, it's greater or equal than the actual size
        size_t memberSize;
        if (memberIdx == structTypeDesc.Members - 1)
            memberSize = structSize - memberTypeDesc.Offset;
        else {
            D3D11_SHADER_TYPE_DESC nextTypeDesc;
            structType->GetMemberTypeByIndex(memberIdx + 1)->GetDesc(&nextTypeDesc);
            memberSize = nextTypeDesc.Offset - memberTypeDesc.Offset;
        }

        void* dst_buf = (uint8_t*)dstBuf + memberTypeDesc.Offset;
        void* src_buf = (uint8_t*)srcBuf + memberInfo->offset;
        switch (memberInfo->type)
        {
            case reflect::DataType::Scalar:
            {
                memcpy_s(dst_buf, memberInfo->size, src_buf, memberInfo->size);
                break;
            }
            case reflect::DataType::Struct:
            {
                SEEK_RETIF_FAIL(ShaderParamAssign(dst_buf, src_buf, memberInfo->struct_info, structType->GetMemberTypeByIndex(memberIdx), memberSize));
                break;
            }
            case reflect::DataType::Array:
            {
                SEEK_RETIF_FAIL(ShaderParamAssign(dst_buf, src_buf, &memberInfo->array_info, structType->GetMemberTypeByIndex(memberIdx), memberSize));
                break;
            }
            default:
            {
                LOG_ERROR("unknown data type for member %s::%s", structInfo->name, memberInfo->name);
                return ERR_INVALID_ARG;
            }
        }
    }

    return S_Success;
}

SResult CBufferAssignHelper::ShaderParamAssign(void* dstBuf, const void* srcBuf, reflect::ArrayInfo const* arrayInfo, ID3D11ShaderReflectionType* arrayType, size_t arraySize)
{
    D3D11_SHADER_TYPE_DESC arrayTypeDesc;
    arrayType->GetDesc(&arrayTypeDesc);

    if (arrayInfo->element_count != arrayTypeDesc.Elements) {
        LOG_ERROR("array element count is different %u!=%u", arrayInfo->element_count, arrayTypeDesc.Elements);
        return ERR_INVALID_ARG;
    }

    // the last element's padding is exclude from arraySize
    // the constant buffer
    size_t element_stride = seek_alignup(arraySize, 16) / arrayTypeDesc.Elements;
    size_t element_size = arraySize - element_stride * (arrayTypeDesc.Elements - 1);

    for (size_t i = 0; i < arrayInfo->element_count; i++)
    {
        void* member_dst = (uint8_t*)dstBuf + i * element_stride;
        void* member_src = (uint8_t*)srcBuf + i * arrayInfo->element_size;

        switch (arrayInfo->element_type)
        {
            case reflect::DataType::Scalar:
            {
                memcpy_s(member_dst, arrayInfo->element_size, member_src, arrayInfo->element_size);
                break;
            }
            case reflect::DataType::Struct:
            {
                SEEK_RETIF_FAIL(ShaderParamAssign(member_dst, member_src, arrayInfo->struct_info, arrayType, element_size));
                break;
            }
            default:
                break;
        }
    }

    return S_Success;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
