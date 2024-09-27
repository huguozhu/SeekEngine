#ifndef d3d11_shader_param_assign_helper_h
#define d3d11_shader_param_assign_helper_h

#include "rhi/base/shader_param_assign_helper.h"
#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 70     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

// constant buffer assign helper class
// CPU -> GPU
class CBufferAssignHelper
{
public:
    CBufferAssignHelper(ID3D11ShaderReflectionConstantBuffer* cbuffer)
    {
        if (cbuffer) {
            HRESULT hr = cbuffer->GetDesc(&m_cbufferDesc);
            if (SUCCEEDED(hr)) {
                m_shaderParamBuf.resize(m_cbufferDesc.Size);
                m_cbuffer = cbuffer;
            }
            else {
                LOG_ERROR("ID3D11ShaderReflectionConstantBuffer::GetDesc fail, hr:%x", hr);
            }
        }
    }

    ~CBufferAssignHelper()
    {

    }

    template <typename T>
    SResult AssignVariableByName(const char* varName, const T& var)
    {
        if (!m_cbuffer)
            return ERR_INVALID_INIT;

        ID3D11ShaderReflectionVariable* shaderVar = m_cbuffer->GetVariableByName(varName);
        if (!shaderVar)
            return ERR_INVALID_ARG;

        return AssignVariable(shaderVar, var);
    }

    template <typename T>
    SResult AssignVariableByIndex(uint32_t idx, const T& var)
    {
        if (!m_cbuffer)
            return ERR_INVALID_INIT;

        ID3D11ShaderReflectionVariable* shaderVar = m_cbuffer->GetVariableByIndex(idx);
        if (!shaderVar)
            return ERR_INVALID_ARG;

        return AssignVariable(shaderVar, var);
    }

    template <typename T>
    SResult AssignVariable(const T& var)
    {
        return AssignVariableByIndex(0, var);
    }

    void* Data()
    {
        return m_shaderParamBuf.data();
    }

    size_t Size()
    {
        return m_shaderParamBuf.size();
    }

private:
    template <typename T>
    SResult AssignVariable(ID3D11ShaderReflectionVariable* shaderVar, const T& var)
    {
        D3D11_SHADER_VARIABLE_DESC varDesc;
        HRESULT hr = shaderVar->GetDesc(&varDesc);
        if (FAILED(hr))
            return ERR_UNKNOWN;

        if (!(varDesc.uFlags & D3D_SVF_USED)) {
            LOG_WARNING("variable %s is unused, no need to assign", varDesc.Name);
            return S_Success;
        }

        ID3D11ShaderReflectionType* varType = shaderVar->GetType();
        D3D11_SHADER_TYPE_DESC varTypeDesc;
        hr = varType->GetDesc(&varTypeDesc);
        if (FAILED(hr))
            return ERR_UNKNOWN;

        void* dstBuf = m_shaderParamBuf.data() + varDesc.StartOffset;

        if (varTypeDesc.Elements != 0) {
            reflect::ArrayInfo varArrayInfo = reflect::ArrayInfoHelper_t<T>::value();
            return ShaderParamAssign(dstBuf, &var, &varArrayInfo, varType, varDesc.Size);
        }
        else {
            switch (varTypeDesc.Class)
            {
            case D3D_SVC_VECTOR:
            case D3D_SVC_SCALAR:
            case D3D_SVC_MATRIX_ROWS:
            case D3D_SVC_MATRIX_COLUMNS:
            {
                memcpy_s(dstBuf, m_shaderParamBuf.size() - varDesc.StartOffset, &var, sizeof(var));
                break;
            }
            case D3D_SVC_STRUCT:
            {
                reflect::StructInfo const* structInfo = reflect::StructInfoTraits<T>::value;
                if (!structInfo)
                    return ERR_UNKNOWN;
                ShaderParamAssign(dstBuf, &var, structInfo, varType, varDesc.Size);
                break;
            }
            default:
            {
                LOG_ERROR("unsupport variable class %d", varTypeDesc.Class);
                return ERR_NOT_SUPPORT;
            }
            }
        }
        return S_Success;
    }

    SResult ShaderParamAssign(void* dstBuf, const void* srcBuf, reflect::StructInfo const* structInfo, ID3D11ShaderReflectionType* structType, size_t structSize);
    SResult ShaderParamAssign(void* dstBuf, const void* srcBuf, reflect::ArrayInfo const* arrayInfo, ID3D11ShaderReflectionType* arrayType, size_t arraySize);

    ID3D11ShaderReflectionConstantBuffer* m_cbuffer = nullptr;
    D3D11_SHADER_BUFFER_DESC m_cbufferDesc = {};
    std::vector<uint8_t> m_shaderParamBuf;
};

SEEK_NAMESPACE_END

#endif /* d3d11_shader_param_assign_helper_h */

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
