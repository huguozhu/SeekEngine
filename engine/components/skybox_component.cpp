#include "components/skybox_component.h"
#include "components/camera_component.h"
#include "scene_manager/scene_manager.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "effect/scene_renderer.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "kernel/context.h"


#define SEEK_MACRO_FILE_UID 75     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#include "shader/shared/SkyBox.h"

SkyBoxComponent::SkyBoxComponent(Context* context)
    :MeshComponent(context)
{
    m_eComponentType = ComponentType::SkyBox;
    m_szName = "SkyBoxComponent";

    RHIContext& rc = context->RHIContextInstance();
    RHIMeshPtr mesh = QuadMesh_GetMesh(rc);
    m_vMeshes.push_back(mesh);

    Effect& effect = m_pContext->EffectInstance();
    effect.LoadTechnique("SkyBox", &RenderStateDesc::Skybox(), "SkyBoxVS", "SkyBoxPS", nullptr);
    m_pTechSkyBox  = effect.GetTechnique("SkyBox");
}

SkyBoxComponent::~SkyBoxComponent()
{
}

SResult SkyBoxComponent::SetSkyBoxTex(RHITexturePtr t)
{
    if (t->Type() != TextureType::Cube)
        return ERR_INVALID_ARG;
    m_pTexSkyBox = t;
    return S_Success;
}

SResult SkyBoxComponent::OnRenderBegin(Technique* tech, RHIMeshPtr mesh)
{
    SceneManager& sm = m_pContext->SceneManagerInstance();
    Matrix4 inv_mvp = Matrix4::Identity();
    CameraComponent* cam = sm.GetActiveCamera();
    SkyBoxParams global_params;
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
        m_GlobalParamsCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(SkyBoxParams), RESOURCE_FLAG_CPU_WRITE);
    }
    m_GlobalParamsCBuffer->Update(&global_params, sizeof(global_params));
    tech->SetParam("cb_SkyBoxParams", m_GlobalParamsCBuffer);
    tech->SetParam("skybox_tex", m_pTexSkyBox);

    return S_Success;
}
SResult SkyBoxComponent::Render()
{
    RHIMeshPtr pMesh = m_vMeshes[0];

    SEEK_RETIF_FAIL(this->OnRenderBegin(m_pTechSkyBox, pMesh));
    SEEK_RETIF_FAIL(m_pTechSkyBox->Render(pMesh));
    SEEK_RETIF_FAIL(this->OnRenderEnd());
    return S_Success;
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
