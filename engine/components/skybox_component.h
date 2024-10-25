#pragma once

#include "components/mesh_component.h"

SEEK_NAMESPACE_BEGIN

class SkyBoxComponent : public MeshComponent
{
public:
    SkyBoxComponent(Context* context);
    virtual ~SkyBoxComponent();

    SResult               SetSkyBoxTex(RHITexturePtr t);
    virtual SResult       OnRenderBegin(Technique* tech, RHIMeshPtr mesh) override;
    virtual SResult       Render() override;

private:
    Technique*          m_pTechSkyBox = nullptr;
    RHITexturePtr       m_pTexSkyBox;
    RHIRenderBufferPtr  m_GlobalParamsCBuffer;
};

SEEK_NAMESPACE_END
