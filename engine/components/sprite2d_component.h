#pragma once

#include "components/component.h"

SEEK_NAMESPACE_BEGIN

class Sprite2DComponent : public Component
{
public:
    Sprite2DComponent(Context* context, std::string const& name = "Sprite2DComponent", ComponentType type = ComponentType::Sprite2D)
        : Component(context, name, type)
    {}
    virtual ~Sprite2DComponent() {}
    virtual SResult Render() = 0;

protected:
    virtual SResult OnRenderBegin(Technique* tech, MeshPtr mesh) = 0;
    virtual SResult OnRenderEnd() = 0;
};

SEEK_NAMESPACE_END
