#pragma once
#include "app_framework.h"
#include "seek_engine.h"

USING_NAMESPACE_SEEK

class Tutorial : public AppFramework
{
public:
    Tutorial();
    virtual ~Tutorial() = default;
    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;

private:
    EntityPtr m_pEntity = nullptr;

};

