#include "components/image_component.h"
#include "effect/sprite2d_renderer.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "kernel/context.h"
#include "rhi/render_buffer.h"
#include "math/quad_mesh_process.h"

#define SEEK_MACRO_FILE_UID 35     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#include "shader/shared/Sprite2D.h"

ImageComponent::ImageComponent(Context* context)
    : Sprite2DComponent(context, "ImageComponent", ComponentType::Image)
{
    m_pMesh             = QuadMesh_GetMesh(m_pContext->RenderContextInstance());
    m_VerticesInit      = QuadMesh_GetVertices();
}

SResult ImageComponent::OnRenderBegin(Technique* tech, MeshPtr mesh)
{
    SceneRenderer&  sc = m_pContext->Sprite2DRendererInstance();
    RenderStage     stage  = sc.GetCurRenderStage();

    switch (stage)
    {
        case RenderStage::Sprite2D:
        {
            Material2DPtr material = mesh->GetMaterial2D();
            if (material)
            {
                if (!m_GlobalParamsCBuffer)
                {
                    m_GlobalParamsCBuffer = m_pContext->RenderContextInstance().CreateConstantBuffer(sizeof(Sprite2DGlobalParams), RESOURCE_FLAG_CPU_WRITE);
                }
                Sprite2DGlobalParams global_params;
                global_params.alpha = material->alpha;
                global_params.color = material->color;
                m_GlobalParamsCBuffer->Update(&global_params, sizeof(global_params));
                tech->SetParam("GlobalParams", m_GlobalParamsCBuffer);

                for (size_t i=0; i<material->textures.size(); i++)
                {
                    std::string name = "Texture" + std::to_string(i);
                    tech->SetParam(name, material->textures[i]);
                }
            }
            break;
        }
        default:
            break;
    }
    return S_Success;
}

SResult ImageComponent::OnRenderEnd()
{
    return S_Success;
}

SResult ImageComponent::Render()
{
    if (!m_bVisible)
        return S_Success;

    SceneRenderer& sc = m_pContext->Sprite2DRendererInstance();

    if (m_bDirty)
    {
        auto vertices = m_VerticesInit;

        if (m_iAngle != 0)
            QuadMesh_VertexRotate(vertices, m_iAngle);

        RenderBufferData vertices_data(uint32_t(vertices.size() * sizeof(float)), vertices.data());
        m_pMesh->GetVertexStreamByIndex(0).render_buffer->Update(&vertices_data);
        m_bDirty = false;
    }

    Technique* tech;
    sc.GetEffectTechniqueToRender(m_pMesh, &tech);

    if (tech)
    {
        DVF_RETIF_FAIL(this->OnRenderBegin(tech, m_pMesh));
        DVF_RETIF_FAIL(tech->Render(m_pMesh));
        DVF_RETIF_FAIL(this->OnRenderEnd());
    }

    return S_Success;
}

void ImageComponent::SetAlpha(float alpha)
{
    m_pMesh->GetMaterial2D()->alpha = alpha;
}

void ImageComponent::SetPos(float x1, float x2, float y1, float y2)
{
    m_VerticesInit = QuadMesh_GetVertices(x1, x2, y1, y2);
    m_bDirty = true;
}

void ImageComponent::SetAngle(int angle)
{
    if (m_iAngle != angle)
    {
        m_iAngle = angle;
        m_bDirty = true;
    }
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
