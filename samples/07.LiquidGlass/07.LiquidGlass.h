#pragma once
#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"

USING_NAMESPACE_SEEK

class LiquidGlass : public AppFramework
{
public:
    LiquidGlass() :AppFramework("LiquidGlass") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr);

    void SetBgTexture(RHITexturePtr bg) { m_pBgTexture = bg; }
    

private:
    EntityPtr       m_pCameraEntity = nullptr;
    RHITexturePtr   m_pBgTexture;

};
