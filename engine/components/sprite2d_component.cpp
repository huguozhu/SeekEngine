#include "components/sprite2d_component.h"
#include "components/camera_component.h"
#include "scene_manager/scene_manager.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "kernel/context.h"


#define SEEK_MACRO_FILE_UID 75     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

Sprite2DComponent::Sprite2DComponent(Context* context, uint32_t width, uint32_t height, uint32_t draw_index)
	: SceneComponent(context, "Sprite2D", ComponentType::Sprite2D), m_iWidth(width), m_iHeight(height), m_iDrawIndex(draw_index)
{
	this->SetCenterPos(float2(0, 0));
}
Sprite2DComponent::~Sprite2DComponent()
{
}
void Sprite2DComponent::SetCenterPos(float2 pos)
{
	this->SetLocalTranslation(float3(pos.x(), pos.y(), 0.0f));
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
