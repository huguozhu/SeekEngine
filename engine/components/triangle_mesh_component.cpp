#include "components/triangle_mesh_component.h"
#include "effect/command_buffer.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN

uint16_t indices[] = {
        0,1,2
};

float3 vertices[] = {
    float4(+0.0f, +0.5f, +0.0f),
    float4(-0.5f, -0.5f, +0.0f),
    float4(+0.5f, -0.5f, +0.0f),
};



TriangleMeshComponent::TriangleMeshComponent(Context* context)
    :MeshComponent(context)
{
    this->Init();
}

TriangleMeshComponent::~TriangleMeshComponent()
{
    m_pIndexBuffer.reset();
}


void TriangleMeshComponent::Init()
{

}
SEEK_NAMESPACE_END