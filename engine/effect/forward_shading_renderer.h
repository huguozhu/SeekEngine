#pragma once

#include "effect/scene_renderer.h"

SEEK_NAMESPACE_BEGIN

class ForwardShadingRenderer : public SceneRenderer
{
public:
    virtual ~ForwardShadingRenderer() override {}

    virtual SResult     Init() override;
    virtual SResult     BuildRenderJobList() override;
    virtual SResult     GetEffectTechniqueToRender(RHIMeshPtr mesh, Technique** tech) override;

    // Rendering Jobs
    RendererReturnValue RenderSceneJob();

    SResult PrepareFrameBuffer();
    
protected:
    friend class Context;
    ForwardShadingRenderer(Context* context);
    
    RHITexturePtr       m_pRenderSceneColorTex;
    RHITexturePtr       m_pRenderSceneDepthTex;
    RHIFrameBufferPtr   m_pRenderSceneFB;

    std::vector<MeshPair> m_renderableMeshes;
};

SEEK_NAMESPACE_END
