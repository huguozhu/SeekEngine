#include "components/entity.h"
#include "components/scene_component.h"
#include "scene_manager/scene_manager.h"
#include "kernel/context.h"

#define SEEK_MACRO_FILE_UID 21     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

Entity::Entity(Context* context, std::string name)
    :m_pContext(context), m_szName(name)
{
    m_pRootComponent = MakeSharedPtr<SceneComponent>(context);
    m_pRootComponent->SetName(name);
    m_pRootComponent->SetOwner(this);
}

void Entity::AddComponent(ComponentPtr component)
{
    m_vOwnedComponents.push_back(component);
    component->SetOwner(this);
}

void Entity::AddSceneComponent(SceneComponentPtr component)
{
    for (ComponentPtr c : m_vOwnedComponents)
    {
        if (c->GetComponentType() != ComponentType::Scene &&
            c->GetComponentType() == component->GetComponentType())
        {
            return ReplaceSceneComponent(component);
        }
    }

    component->SetParent(m_pRootComponent.get());
    m_pRootComponent->AddChild(component);
    AddComponent(component);
}

void Entity::ReplaceSceneComponent(SceneComponentPtr component)
{
    for (ComponentPtr c : m_vOwnedComponents)
    {
        if (c->GetComponentType() == component->GetComponentType())
        {
            SceneComponentPtr scene = std::dynamic_pointer_cast<SceneComponent>(c);
            // remove
            m_pRootComponent->DelChild(scene);
            m_vOwnedComponents.erase(std::find(m_vOwnedComponents.begin(), m_vOwnedComponents.end(), c));
            // add new
            AddSceneComponent(component);
            return;
        }
    }
    SEEK_ASSERT(false);
}

void Entity::Clear()
{
    for (uint32_t i = 0; i < m_pRootComponent->NumChildren(); i++)
    {
        m_pRootComponent->DelChild(m_pRootComponent->ChildByIndex(i));
    }
    m_vOwnedComponents.clear();
}

SResult Entity::Tick(float delta_time)
{
    for (ComponentPtr component : m_vOwnedComponents)
    {
        SEEK_RETIF_FAIL(component->Tick(delta_time));
    }
    return S_Success;
}

SResult Entity::AddToTopScene()
{
    return AddToParent(m_pContext->SceneManagerInstance().GetRootComponent().get());
}

SResult Entity::DeleteFromTopScene()
{
    m_pContext->SceneManagerInstance().GetRootComponent()->DelChild(m_pRootComponent);
    return S_Success;
}

SResult Entity::AddToParent(Entity* parent_entity)
{
    if (parent_entity == nullptr)
        return ERR_INVALID_ARG;

    parent_entity->AddSceneComponent(m_pRootComponent);
    return S_Success;
}

SResult Entity::AddToParent(SceneComponent* parent_component)
{
    if (parent_component == nullptr)
        return ERR_INVALID_ARG;

    m_pRootComponent->SetParent(parent_component);
    parent_component->AddChild(m_pRootComponent);
    return S_Success;
}


// Transform Functions
void Entity::SetWorldTransform(Matrix4 const& matrix)
{
    return m_pRootComponent->SetWorldTransform(matrix);
}
void Entity::SetWorldTranslation(float3 const t)
{
    return m_pRootComponent->SetWorldTranslation(t);
}
void Entity::SetWorldRotate(Quaternion const& r)
{
    return m_pRootComponent->SetWorldRotate(r);
}
void Entity::SetWorldScale(float3 const& s)
{
    return m_pRootComponent->SetWorldScale(s);
}
void Entity::SetLocalTransform(Matrix4 const& matrix)
{
    return m_pRootComponent->SetLocalTransform(matrix);
}
void Entity::SetLocalTranslation(float3 const& t)
{
    return m_pRootComponent->SetLocalTranslation(t);
}
void Entity::SetLocalRotation(Quaternion const& r)
{
    return m_pRootComponent->SetLocalRotation(r);
}
void Entity::SetLocalScale(float3 const& s)
{
    return m_pRootComponent->SetLocalScale(s);
}

Component* Entity::GetComponent(ComponentType type) const
{
    for (ComponentPtr component : m_vOwnedComponents)
    {
        if (component->GetComponentType() == type)
        {
            return component.get();
        }
    }
    //SEEK_ASSERT(false);
    return nullptr;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
