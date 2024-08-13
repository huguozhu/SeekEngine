/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : skybox_component.h
**
**      Brief                  : skybox component
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-12-16  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "components/mesh_component.h"

SEEK_NAMESPACE_BEGIN

class SkyBoxComponent : public MeshComponent
{
public:
    SkyBoxComponent(Context* context);
    virtual ~SkyBoxComponent();

    SResult               SetSkyBoxTex(TexturePtr t);
    virtual SResult       OnRenderBegin(Technique* tech, MeshPtr mesh) override;
    virtual SResult       Render() override;

private:
    //EffectPtr       m_pEffectSkyBox = nullptr;
    Technique*      m_pTechSkyBox = nullptr;
    TexturePtr      m_pTexSkyBox;
    RenderBufferPtr m_GlobalParamsCBuffer;
};

SEEK_NAMESPACE_END
