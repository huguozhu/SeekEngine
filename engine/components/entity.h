#pragma once

#include "kernel/kernel.h"
#include "components/component.h"
#include "math/quaternion.h"

SEEK_NAMESPACE_BEGIN

class Entity
{
public:
    Entity(Context* context, std::string name = "Entity");
    virtual ~Entity() {};

    std::string const&      GetName() const { return m_szName; }
    void                    SetName(std::string const& name) { m_szName = name; }

    SceneComponentPtr       GetRootComponent() const { return m_pRootComponent; }

    // Add a Component to m_vOwnedComponents
    void                    AddComponent(ComponentPtr component);
    // Add a new child to m_pRootComponent, and add it to m_vOwnedComponents
    void                    AddSceneComponent(SceneComponentPtr component);
    // Clear
    void                    Clear();

    // Replace buildt-in SceneComponent
    void                    ReplaceSceneComponent(SceneComponentPtr component);

    virtual SResult       Tick(float delta_time);

    // Add m_pRootComponent to the root node of the SceneManager
    SResult               AddToTopScene();
    // Delete m_pRootComponent from the root node of the SceneManager
    SResult               DeleteFromTopScene();

    // Set the current root SC as the child of the parent entity root SC, and add to parent's m_vOwnedComponents
    SResult               AddToParent(Entity* parent_entity);
    // Set the current root SC as the child of the parent_component
    SResult               AddToParent(SceneComponent* parent_component);

    // Transform Functions
    void                    SetWorldTransform(Matrix4 const& matrix);
    void                    SetWorldTranslation(float3 const t);
    void                    SetWorldRotate(Quaternion const& r);
    void                    SetWorldScale(float3 const& s);

    void                    SetLocalTransform(Matrix4 const& matrix);
    void                    SetLocalTranslation(float3 const& t);
    void                    SetLocalRotation(Quaternion const& r);
    void                    SetLocalScale(float3 const& s);
    
    Component*              GetComponent(ComponentType type) const;
    std::vector<ComponentPtr>   GetOwnedComponents() { return m_vOwnedComponents; };

protected:
    Context*                    m_pContext = nullptr;
    std::string                 m_szName;
    SceneComponentPtr           m_pRootComponent = nullptr; // never be nullptr
    std::vector<ComponentPtr>   m_vOwnedComponents;
};

SEEK_NAMESPACE_END
