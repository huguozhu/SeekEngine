#pragma once
#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"

USING_NAMESPACE_SEEK

class MetaballWater : public AppFramework
{
public:
    MetaballWater() :AppFramework("MetaballWater") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr);
     

private:
    EntityPtr       m_pEntity = nullptr;
    RHITexturePtr   m_pBgTexture;

};
