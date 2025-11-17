#pragma once

#include "effect/scene_renderer.h"

SEEK_NAMESPACE_BEGIN

class Sprite2DRenderer : public SceneRenderer
{
public:
    virtual ~Sprite2DRenderer();

    virtual SResult             Init() override;
    virtual SResult             GetEffectTechniqueToRender(RHIMeshPtr mesh, Technique** tech) { return S_Success; }
    virtual SResult             BuildRenderJobList() override;

    RendererReturnValue         RenderSprite2DJob();
    RendererReturnValue         FinishJob();

protected:
    friend class Context;
    Sprite2DRenderer(Context* context);

};

SEEK_NAMESPACE_END
