#include "rhi/d3d11/d3d11_predeclare.h"
#include "rhi/d3d11/d3d11_mesh.h"
#include "rhi/d3d11/d3d11_rhi_context.h"
#include "rhi/d3d11/d3d11_gpu_buffer.h"
#include "rhi/d3d11/d3d11_translate.h"
#include "rhi/d3d11/d3d11_shader.h"
#include "rhi/d3d_common/d3d_common_translate.h"

#include "rhi/base/rhi_program.h"

#include "kernel/context.h"

#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 11     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#define MORPH_TARGET_USAGE_INDEX 1

D3D11Mesh::D3D11Mesh(Context* context)
    : RHIMesh(context)
{

}
D3D11Mesh::~D3D11Mesh()
{
    for (auto layout : m_vD3DInputLayouts)
        layout.second.Reset();
    m_vD3DInputElementDescs.clear();
    m_vD3DInputLayouts.clear();
}

SResult D3D11Mesh::Active(RHIProgram* program)
{
    // Step1: Update Dirty Data
    uint32_t vertex_stream_count = this->NumVertexStream();
    std::vector<ID3D11Buffer*>& vbs = std::get<0>(m_vD3DInputElementInfos);
    std::vector<UINT>& strides = std::get<1>(m_vD3DInputElementInfos);
    std::vector<UINT>& offsets = std::get<2>(m_vD3DInputElementInfos);

    if (vertex_stream_count == 0)
    {
        LOG_ERROR("no vertex data");
        return ERR_INVALID_ARG;
    }

    if (m_bDataDirty)
    {
        D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());

        m_vD3DInputElementDescs.clear();
        m_vD3DInputElementDescs.reserve(vertex_stream_count);
        vbs.clear();
        strides.clear();
        offsets.clear();

        for (uint32_t vertexStreamIndex = 0; vertexStreamIndex < vertex_stream_count; vertexStreamIndex++)
        {
            VertexStream const& vs = this->GetVertexStreamByIndex(vertexStreamIndex);
            UINT elementOffset = 0;
            for (int j = 0; j < vs.layouts.size(); j++)
            {
                VertexStreamLayout layout = vs.layouts[j];

                D3D11_INPUT_ELEMENT_DESC desc;
                desc.SemanticName = D3D11Translate::TranslateVertexElementUsageSemantic(layout.usage);
                desc.SemanticIndex = layout.usage_index;
                desc.Format = D3DCommonTranslate::TranslateToPlatformFormat(layout.format);

                desc.InputSlot = vertexStreamIndex;
                desc.AlignedByteOffset = layout.buffer_offset;
                if (layout.usage == VertexElementUsage::Instance)
                {
                    desc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
                    desc.InstanceDataStepRate = layout.instance_divisor;
                }
                else
                {
                    desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                    desc.InstanceDataStepRate = 0;
                }
                m_vD3DInputElementDescs.push_back(desc);
            }

            D3D11VertexBuffer* vb = static_cast<D3D11VertexBuffer*>(vs.render_buffer.get());
            vbs.push_back(vb->GetD3DBuffer());
            strides.push_back((UINT)vs.stride);
            offsets.push_back((UINT)vs.offset);
        }
        m_bDataDirty = false;
    }

    D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();

    // Step2: Set Mesh Topology
    pDeviceContext->IASetPrimitiveTopology(D3D11Translate::TranslatePrimitiveTopology(m_eTopoType));

    // Step3: Set Input Layout
    ID3D11InputLayout* input_layout = this->GetInputLayout(program);
    if (!input_layout)
        return ERR_INVALID_ARG;
    pDeviceContext->IASetInputLayout(input_layout);

    // Step4: Set Indices
    if (this->IsUseIndices())
    {
        D3D11IndexBuffer& ib = static_cast<D3D11IndexBuffer&>(*this->GetIndexBuffer());
        DXGI_FORMAT index_format = (this->GetIndexBufferType() == IndexBufferType::UInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
        pDeviceContext->IASetIndexBuffer(ib.GetD3DBuffer(), index_format, 0);
    }
    else
    {
        pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
    }

    // Step5: Set Vertex Buffer
    pDeviceContext->IASetVertexBuffers(0, (UINT)vbs.size(), &vbs[0], &strides[0], &offsets[0]);

    return S_Success;
}
SResult D3D11Mesh::Deactive() const
{
    D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
    ID3D11DeviceContext* pDeviceContext = rc.GetD3D11DeviceContext();
    pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
    std::vector<ID3D11Buffer*> null_buffers(D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT);
    std::vector<UINT> null_datas(D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT);
    pDeviceContext->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, null_buffers.data(), null_datas.data(), null_datas.data());

    return S_Success;
}
ID3D11InputLayout* D3D11Mesh::GetInputLayout(RHIProgram* program) const
{
    if (m_vVertexStreams.empty())
        return nullptr;

    for (auto layout : m_vD3DInputLayouts)
    {
        if (layout.first == program)
            return layout.second.Get();
    }

    D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
    ID3D11Device* pDevice = rc.GetD3D11Device();
    D3D11Shader* vs_shader = (D3D11Shader*)(program->GetShader(ShaderType::Vertex));
    if (!vs_shader)
        return nullptr;
    //ID3DBlobPtr vs_code = vs_shader->GetD3DVSCode();
    ID3D11InputLayoutPtr d3d_input_layout = nullptr;
    HRESULT hr = pDevice->CreateInputLayout(&m_vD3DInputElementDescs[0], (UINT)m_vD3DInputElementDescs.size(),
        vs_shader->GetBufferPointer(), vs_shader->GetBufferSize(), d3d_input_layout.GetAddressOf());
    if (FAILED(hr))
    {
        LOG_ERROR("CreateInputLayout Error.");
        return nullptr;
    }

    ID3D11InputLayout* out = d3d_input_layout.Get();
    m_vD3DInputLayouts.push_back(std::make_pair(program, d3d_input_layout));
    return out;
}

SEEK_NAMESPACE_END


#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
