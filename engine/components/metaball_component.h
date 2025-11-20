#pragma once
#include "components/sprite2d_component.h"
#include "physics/metaball_water_simulator.h"

SEEK_NAMESPACE_BEGIN

#include "shader/shared/MetaBall.h"

/******************************************************************************
 * Metaball2DComponent
 ******************************************************************************/
class Metaball2DComponent : public Sprite2DComponent
{
public:
    Metaball2DComponent(Context* context, uint32_t width, uint32_t height, uint32_t draw_index = 0);
    ~Metaball2DComponent();

    void InitRandomBall(uint32_t init_count);
    void AddMetaball(Metaball ball);
    void RemoveMetaball(uint32_t index);

	virtual SResult     OnRenderBegin() override;
    virtual SResult     Render() override;
    virtual SResult     Tick(float delta_time) override;

private:
    void Reset();
    SResult InitShaders();

private:
    std::vector<Metaball> m_vMetaballs;
    MetaballWaterSimulatorPtr m_pSimulator;

    Technique*          m_pMetaballTech = nullptr;
    RHIMeshPtr	        m_pMetaballMesh = nullptr;

    MetaballParam       m_Param;
    RHIGpuBufferPtr     m_pMvpCbBuffer = nullptr;
    RHIGpuBufferPtr     m_pParamCbBuffer = nullptr;
};

/******************************************************************************
 * Metaball3DComponent
 ******************************************************************************/
class Metaball3DComponent : public SceneComponent
{
public:
    Metaball3DComponent(Context* context)
        :SceneComponent(context)
    {
    }
    ~Metaball3DComponent() {}
	SResult             Render() { return S_Success; }
	virtual SResult     Tick(float delta_time) override { return S_Success; }
};

SEEK_NAMESPACE_END