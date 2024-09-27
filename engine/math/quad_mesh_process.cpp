#include "quad_mesh_process.h"
#include "vector.h"
#include "math_utility.h"
#include "rhi/base/mesh.h"
#include "rhi/base/rhi_context.h"
#include "utils/buffer.h"

SEEK_NAMESPACE_BEGIN

#define VERTEX_POS_OFFSET   0
#define VERTEX_UV_OFFSET    3
#define VERTEX_STRIDE       5
#define VERTEX_CNT          4

std::vector<float> QuadMesh_GetVertices(float left, float right, float top, float bottom) // 0 ~ 1
{
    float ndc_left = 2 * left - 1;
    float ndc_right = 2 * right - 1;
    float ndc_top = 1 - 2 * top;
    float ndc_bottom = 1 - 2 * bottom;

    return std::vector<float> {
        //  pos_x   pos_y           pos_z   u    v
        ndc_left,   ndc_top,        1.0,    0.0, 0.0,
        ndc_left,   ndc_bottom,     1.0,    0.0, 1.0,
        ndc_right,  ndc_top,        1.0,    1.0, 0.0,
        ndc_right,  ndc_bottom,     1.0,    1.0, 1.0,
    };
}

RHIMeshPtr QuadMesh_GetMesh(RHIContext& ctx, std::vector<float>* vertices)
{
    static std::vector<float>     vertices_ = QuadMesh_GetVertices();
    static uint16_t               indics[4] = {0, 1, 2, 3};
    static RenderBufferData       indics_data(sizeof(indics), indics);
    if (!vertices)
        vertices = &vertices_;

    RHIMeshPtr mesh = ctx.CreateMesh();
    RenderBufferPtr indics_buffer = ctx.CreateIndexBuffer(sizeof(indics), 0, &indics_data);
    mesh->SetIndexBuffer(indics_buffer, IndexBufferType::UInt16);

    RenderBufferData vertex_data((uint32_t)(sizeof(float) * vertices->size()), vertices->data());
    RenderBufferPtr vertex_buffer = ctx.CreateVertexBuffer(vertex_data.m_iDataSize, RESOURCE_FLAG_NONE, &vertex_data);
    mesh->AddVertexStream(vertex_buffer, sizeof(float) * VERTEX_POS_OFFSET, sizeof(float) * VERTEX_STRIDE, VertexFormat::Float3, VertexElementUsage::Position, 0);
    mesh->AddVertexStream(vertex_buffer, sizeof(float) * VERTEX_UV_OFFSET,  sizeof(float) * VERTEX_STRIDE, VertexFormat::Float2, VertexElementUsage::TexCoord, 0);
    mesh->SetTopologyType(MeshTopologyType::Triangle_Strip);
    return mesh;
}

void QuadMesh_VertexRotate(std::vector<float>& vertices, int angle, uint32_t* width, uint32_t* height)
{
    if (vertices.size() != VERTEX_STRIDE * VERTEX_CNT)
        return;

    for (int i=0; i<VERTEX_CNT; i++)
    {
        float2 res = Math::Rotate2D(float2(vertices[i*VERTEX_STRIDE+0], vertices[i*VERTEX_STRIDE+1]), -angle);
        vertices[i*VERTEX_STRIDE+0] = res[0];
        vertices[i*VERTEX_STRIDE+1] = res[1];
    }

    if (width && height)
    {
        float2 size = Math::BBoxSizeAfterRotate2D(float2(*width, *height), -angle);
        *width = size[0];
        *height = size[1];
    }

    float new_len_x = 0, new_len_y = 0;
    for (int i=0; i<VERTEX_CNT; i++)
    {
        for (int j=0; j<VERTEX_CNT; j++)
        {
            new_len_x = Math::Max(new_len_x, Math::Abs(vertices[i*VERTEX_STRIDE+0] - vertices[j*VERTEX_STRIDE+0]));
            new_len_y = Math::Max(new_len_y, Math::Abs(vertices[i*VERTEX_STRIDE+1] - vertices[j*VERTEX_STRIDE+1]));
        }
    }
    if (new_len_x > 2 || new_len_y > 2)
    {
        float ratio = Math::Max(new_len_x, new_len_y) / 2;
        for (int i=0; i<VERTEX_CNT; i++)
        {
            vertices[i*VERTEX_STRIDE+0] /= ratio;
            vertices[i*VERTEX_STRIDE+1] /= ratio;
        }
    }
}

void QuadMesh_VertexAspectRadio(std::vector<float>& vertices, int aspect_radio_mode, float dst_aspect_ratio, float src_aspect_ratio)
{
    if (aspect_radio_mode == 0) // fit screen
    {
        if (dst_aspect_ratio > src_aspect_ratio)
            QuadMesh_VertexScaleX(vertices, src_aspect_ratio / dst_aspect_ratio);
        else
            QuadMesh_VertexScaleY(vertices, dst_aspect_ratio / src_aspect_ratio);
    }
    else if (aspect_radio_mode == 1) // fill screen
    {
        if (dst_aspect_ratio > src_aspect_ratio)
            QuadMesh_VertexScaleY(vertices, dst_aspect_ratio / src_aspect_ratio);
        else
            QuadMesh_VertexScaleX(vertices, src_aspect_ratio / dst_aspect_ratio);
    }
}

void QuadMesh_VertexScale_Offset(std::vector<float>& vertices, float scale, float offset)
{
    float center = 0.0f;
    for (int i=0; i<VERTEX_CNT; i++)
        center += vertices[i*VERTEX_STRIDE+offset];
    center /= VERTEX_CNT;

    for (int i=0; i<VERTEX_CNT; i++)
        vertices[i*VERTEX_STRIDE+offset] = (vertices[i*VERTEX_STRIDE+offset] - center) * scale + center;
}

void QuadMesh_VertexScaleX(std::vector<float>& vertices, float scale)
{
    int offset = 0;
    if (scale > 1)
    {
        offset = 3;
        scale = 1 / scale;
    }

    QuadMesh_VertexScale_Offset(vertices, scale, offset);
}

void QuadMesh_VertexScaleY(std::vector<float>& vertices, float scale)
{
    int offset = 1;
    if (scale > 1)
    {
        offset = 4;
        scale = 1 / scale;
    }

    QuadMesh_VertexScale_Offset(vertices, scale, offset);
}

void QuadMesh_VertexScale(std::vector<float>& vertices, float scale)
{
    QuadMesh_VertexScaleX(vertices, scale);
    QuadMesh_VertexScaleY(vertices, scale);
}

void QuadMesh_VertexMirrorX(std::vector<float>& vertices)
{
    std::swap(vertices[0*VERTEX_STRIDE+0], vertices[2*VERTEX_STRIDE+0]);
    std::swap(vertices[1*VERTEX_STRIDE+0], vertices[3*VERTEX_STRIDE+0]);
}

void QuadMesh_VertexMirrorY(std::vector<float>& vertices)
{
    std::swap(vertices[0*VERTEX_STRIDE+1], vertices[1*VERTEX_STRIDE+1]);
    std::swap(vertices[2*VERTEX_STRIDE+1], vertices[3*VERTEX_STRIDE+1]);
}

void QuadMesh_VertexRectInfo(std::vector<float>& vertices, float2& center, float2& corner, float2& size)
{
    float y_min = 100.0f, y_max = -100.0f;
    float x_min = 100.0f, x_max = -100.0f;

    for (int i=0; i<VERTEX_CNT; i++)
    {
        float x = vertices[i*VERTEX_STRIDE+0];
        float y = vertices[i*VERTEX_STRIDE+1];
        x_min = Math::Min<float>(x_min, x);
        x_max = Math::Max<float>(x_max, x);
        y_min = Math::Min<float>(y_min, y);
        y_max = Math::Max<float>(y_max, y);
    }
    center = float2{ (x_max + x_min) / 2, (y_max + y_min) / 2 };
    corner = float2{ x_max, y_min };
    size = float2{ x_max - x_min, y_max - y_min };
}

SEEK_NAMESPACE_END
