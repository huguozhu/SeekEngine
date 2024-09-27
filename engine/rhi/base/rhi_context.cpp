#include "rhi/base/rhi_context.h"
#include "rhi/base/framebuffer.h"
#include "rhi/base/mesh.h"
#include "math/quad_mesh_process.h"
#include "math/math_utility.h"
#include <cmath>

#define SEEK_MACRO_FILE_UID 40     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

RHIContext::RHIContext(Context* context)
    :m_pContext(context)
{
}

SResult RHIContext::BindFrameBuffer(FrameBufferPtr const& fb)
{
    SResult res = S_Success;
    if (fb != m_pCurFrameBuffer || fb == m_pScreenFrameBuffer || (fb && fb->IsDirty()) )
    {
        if (m_pCurFrameBuffer)
        {
            SEEK_RETIF_FAIL(m_pCurFrameBuffer->OnUnbind());
        }

        m_pCurFrameBuffer = fb;
    }

    if (m_pCurFrameBuffer)
    {
        SEEK_RETIF_FAIL(m_pCurFrameBuffer->OnBind());
    }
    return res;
}

FrameBufferPtr const& RHIContext::GetScreenFrameBuffer() const
{
    return m_pScreenFrameBuffer;
}

FrameBufferPtr const& RHIContext::GetFinalFrameBuffer() const
{
    return m_pFinalFrameBuffer;
}
FrameBufferPtr const& RHIContext::GetCurFrameBuffer() const
{
    return m_pCurFrameBuffer;
}
void RHIContext::SetFinalFrameBuffer(FrameBufferPtr const& fb)
{
    m_pFinalFrameBuffer = fb;
}
MeshPtr RHIContext::GetCubeMesh()
{
    if (!m_pCubeMesh)
        this->CreateCommonMesh();
    return m_pCubeMesh;
}
MeshPtr RHIContext::GetConeMesh()
{
    if (!m_pConeMesh)
        this->CreateCommonMesh();
    return m_pConeMesh;
}
void RHIContext::CreateCommonMesh()
{
    if (!m_pCubeMesh)
    {
        // Create Mesh
        m_pCubeMesh = this->CreateMesh();

        // Create Vertex Buffer
        static const float vertexs[] = {
        //  pos_x  pos_y  pos_z   normal_x  y   z     u     v
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        RenderBufferData vertex_data(sizeof(vertexs), vertexs);
        RenderBufferPtr vertex_buffer = this->CreateVertexBuffer(sizeof(vertexs), RESOURCE_FLAG_NONE, &vertex_data);
        uint32_t stride = sizeof(float) * 8;
        m_pCubeMesh->AddVertexStream(vertex_buffer, 0,                   stride, VertexFormat::Float3, VertexElementUsage::Position, 0);
        m_pCubeMesh->AddVertexStream(vertex_buffer, sizeof(float) * 3,   stride, VertexFormat::Float3, VertexElementUsage::Normal,    0);
        m_pCubeMesh->AddVertexStream(vertex_buffer, sizeof(float) * 6,   stride, VertexFormat::Float2, VertexElementUsage::TexCoord,  0);
        m_pCubeMesh->SetTopologyType(MeshTopologyType::Triangles);
    }
    if (!m_pConeMesh)
    {
        static float radius = 1.0;
        static float height = 1.0;
        static uint32_t N = 20;
        std::vector<float3> vertices;
        std::vector<uint16_t> indices;
        // Step1: Vertex
        float3 top_pos(0, 0, 0);
        vertices.push_back(top_pos);
        float3 bottom_center(0, 0, height);
        vertices.push_back(bottom_center);
        for (uint32_t i = 0; i < N; i++)
        {
            float radian = i * 2 * Math::PI / N;
            float3 bottom_out;
            bottom_out[0] = radius * cos(radian);
            bottom_out[1] = radius * sin(radian);
            bottom_out[2] = height;
            vertices.push_back(bottom_out);
        }

        // Step2: Cone Side Indices
        for (uint16_t i = 0; i < N - 1; i++)
        {
            indices.push_back(0);            
            indices.push_back(2 + i + 1);
            indices.push_back(2 + i);
        }
        indices.push_back(0);        
        indices.push_back(2);
        indices.push_back(2 + N - 1);

        // Step3: Cone Bottom Indices
        for (uint16_t i = 0; i < N - 1; i++)
        {
            indices.push_back(1);           
            indices.push_back(2 + i + 1);
            indices.push_back(2 + i);
        }
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(2 + N - 1);
        

        m_pConeMesh = this->CreateMesh();
        RenderBufferData vertex_data(sizeof(float3)* vertices.size(), &vertices[0]);
        RenderBufferPtr vertex_buffer = this->CreateVertexBuffer((uint32_t)(sizeof(float3) * vertices.size()), RESOURCE_FLAG_NONE, &vertex_data);
        m_pConeMesh->AddVertexStream(vertex_buffer, 0, sizeof(float3), VertexFormat::Float3, VertexElementUsage::Position, 0);

        RenderBufferData index_data(sizeof(uint16_t)* indices.size(), &indices[0]);
        RenderBufferPtr index_buffer = this->CreateIndexBuffer((uint32_t)(sizeof(uint16_t) * indices.size()), RESOURCE_FLAG_NONE, &index_data);
        m_pConeMesh->SetIndexBuffer(index_buffer, IndexBufferType::UInt16);
        m_pConeMesh->SetTopologyType(MeshTopologyType::Triangles);
    }
}

TexturePtr RHIContext::CreateTexture2D(const BitmapBufferPtr data)
{
    Texture::Desc desc;
    desc.width = data->Width();
    desc.height = data->Height();
    desc.type = TextureType::Tex2D;
    desc.format = data->Format();
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_CPU_WRITE;
    return this->CreateTexture2D(desc, data);
}

SamplerPtr RHIContext::GetSampler(SamplerDesc const& desc)
{
    auto it = m_Samplers.find(desc);
    if (it != m_Samplers.end())
        return it->second;

    LOG_INFO("Context::GetSampler");
    SamplerPtr ptr = this->CreateSampler(desc);
    if (ptr)
        m_Samplers[desc] = ptr;
    return ptr;
}

RenderStatePtr RHIContext::GetRenderState(RenderStateDesc const& desc)
{
    auto it = m_RenderStates.find(desc);
    if (it != m_RenderStates.end())
        return it->second;

    LOG_INFO("Context::GetRenderState");
    RenderStatePtr ptr = this->CreateRenderState(desc);
    if (ptr)
        m_RenderStates[desc] = ptr;
    return ptr;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
