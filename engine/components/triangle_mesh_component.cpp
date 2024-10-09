#include "components/triangle_mesh_component.h"
#include "effect/command_buffer.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN

TriangleMeshComponent::TriangleMeshComponent(Context* context)
    :MeshComponent(context)
{

}

TriangleMeshComponent::~TriangleMeshComponent()
{

}


void TriangleMeshComponent::Init()
{
    m_pContext->RendererCommandManagerInstance().CreateMesh();
}
SEEK_NAMESPACE_END