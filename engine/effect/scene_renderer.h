#pragma once

#include <functional>
#include "kernel/kernel.h"
#include "rhi/base/viewport.h"
#include "scene_manager/scene_manager.h"

SEEK_NAMESPACE_BEGIN

enum RendererReturnValue : uint32_t
{
    RRV_NextJob     = 0x00000001 << 0,
    RRV_Finish      = 0x00000001 << 1,
};

enum class RenderStage : uint8_t
{
    None,
    RenderScene,
    Sprite2D,
};

using RenderingFunc = std::function<RendererReturnValue()>;

class RenderingJob
{
public:
    RenderingJob(RenderingFunc const& func) : m_pFunc(func) {}
    RenderingJob(RenderingFunc&& func)
        : m_pFunc(std::forward<RenderingFunc>(func)) {}
    RendererReturnValue Run() { return m_pFunc(); }
private:
    RenderingFunc m_pFunc;
};
CLASS_DECLARE(RenderingJob);

struct LightInfo;
struct CameraInfo;
class SceneRenderer
{
public:
    virtual ~SceneRenderer();

    virtual SResult         Init();
    virtual SResult         BuildRenderJobList() = 0;
    virtual SResult         GetEffectTechniqueToRender(RHIMeshPtr mesh, Technique** tech) = 0;
    virtual bool            IsNeedShaderInvariant(RenderStage stage) { return false; }

    // call after BuildRenderJobList
    bool                    HasRenderJob();
    RendererReturnValue     DoRenderJob();

    RendererReturnValue     ToneMappingJob();
    RendererReturnValue     FinishJob();

    RendererType            GetRendererType() { return m_eRendererType; }
    RenderStage             GetCurRenderStage() const { return m_eCurRenderStage; }
    void                    SetCurRenderStage(RenderStage stage) { m_eCurRenderStage = stage; }

    SResult                 FillLightInfoByLightIndex(LightInfo& info, CameraComponent*  pCamera, size_t light_index);

    void SetViewport(const Viewport& viewport)
    {
        if (!m_FinalViewport.IsSameSize(viewport))
        {
            m_bRenderSizeChanged = true;
            m_RenderViewport.width = viewport.width;
            m_RenderViewport.height = viewport.height;
        }
        m_FinalViewport = viewport;
    }
    
protected:
    SceneRenderer(Context* context);
    void                    CreateRenderMeshes();

protected:
    Context*                                        m_pContext = nullptr;
    RendererType                                    m_eRendererType = RendererType::Unknown;

    RenderStage                                     m_eCurRenderStage = RenderStage::None;
    std::vector<RenderingJobPtrUnique>              m_vRenderingJobs;

    // ToneMapping
    PostProcessPtr                                  m_pToneMappingPostProcess = nullptr;
    
    //Viewport m_viewport;
    bool m_bRenderSizeChanged = false;
    
    Viewport m_RenderViewport;  // for intermediate resources during rendering, left/top are always 0
    Viewport m_FinalViewport;   // for user provided render target, draw on the specialed area
};

SEEK_NAMESPACE_END
