#include "components/skybox_component.h"
#include "components/camera_component.h"
#include "scene_manager/scene_manager.h"
#include "rhi/mesh.h"
#include "rhi/texture.h"
#include "effect/scene_renderer.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "kernel/context.h"
#include "rhi/render_buffer.h"

#define SEEK_MACRO_FILE_UID 75     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#include "shader/shared/SkyBox.h"

SkyBoxComponent::SkyBoxComponent(Context* context)
    :MeshComponent(context)
{
    m_eComponentType = ComponentType::SkyBox;
    m_szName = "SkyBoxComponent";

    RenderContext& rc = context->RenderContextInstance();
    MeshPtr mesh = QuadMesh_GetMesh(rc);
    m_vMeshes.push_back(mesh);

    Effect& effect = m_pContext->EffectInstance();
    m_pTechSkyBox  = effect.GetTechnique("SkyBox");
}

SkyBoxComponent::~SkyBoxComponent()
{
}

SResult SkyBoxComponent::SetSkyBoxTex(TexturePtr t)
{
    if (t->Type() != TextureType::Cube)
        return ERR_INVALID_ARG;
    m_pTexSkyBox = t;
    return S_Success;
}

SResult SkyBoxComponent::OnRenderBegin(Technique* tech, MeshPtr mesh)
{
    SceneManager& sm = m_pContext->SceneManagerInstance();
    Matrix4 inv_mvp = Matrix4::Identity();
    CameraComponent* cam = sm.GetActiveCamera();
    SkyBoxGlobalParams global_params;
    if (cam)
    {
        Matrix4 v = cam->GetViewMatrix();
        // no translate
        v(3, 0) = 0;
        v(3, 1) = 0;
        v(3, 2) = 0;
        Matrix4 p = cam->GetProjMatrix();
        Matrix4 vp = v * p;
        global_params.inv_mvp = vp.Inverse().Transpose();
    }

    if (!m_GlobalParamsCBuffer)
    {
        m_GlobalParamsCBuffer = m_pContext->RenderContextInstance().CreateConstantBuffer(sizeof(SkyBoxGlobalParams), RESOURCE_FLAG_CPU_WRITE);
    }
    m_GlobalParamsCBuffer->Update(&global_params, sizeof(global_params));
    tech->SetParam("GlobalParams", m_GlobalParamsCBuffer);
    tech->SetParam("skybox_tex", m_pTexSkyBox);

    return S_Success;
}
SResult SkyBoxComponent::Render()
{
    MeshPtr pMesh = m_vMeshes[0];

    DVF_RETIF_FAIL(this->OnRenderBegin(m_pTechSkyBox, pMesh));
    DVF_RETIF_FAIL(m_pTechSkyBox->Render(pMesh));
    DVF_RETIF_FAIL(this->OnRenderEnd());
    return S_Success;
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
