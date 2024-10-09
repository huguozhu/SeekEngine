#pragma once
#include "components/mesh_component.h"


SEEK_NAMESPACE_BEGIN


class TriangleMeshComponent : public MeshComponent
{
public:
    TriangleMeshComponent(Context* context);
    ~TriangleMeshComponent();

    void Init();

};

SEEK_NAMESPACE_END