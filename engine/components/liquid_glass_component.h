#pragma once

#include "components/sprite2d_component.h"
#include "math/vector.h"
#include "physics/spring_mass_damper.h"

SEEK_NAMESPACE_BEGIN
#include "shader/shared/LiquidGlass.h"

enum class SpringMassDamperState
{
    Stopped,
    Playing,
};

class LiquidGlassComponent : public Sprite2DComponent
{
public:
    LiquidGlassComponent(Context* context, uint32_t width, uint32_t height, uint32_t draw_index = 0);
    virtual ~LiquidGlassComponent();

    virtual SResult     OnRenderBegin() override;
    virtual SResult     Render() override;
    virtual SResult     Tick(float delta_time) override;

    bool HitShape(float2 hit_pos, int& hited_shape_index);
    void SetSpringState(uint32_t spring_index, SpringMassDamperState state);
    void SetInitShapeType(uint32_t shape_index, ShapeType type);
    void SetInitShapeRadius(uint32_t shape_index, float2 pos);
    void SetInitShapeCenter(uint32_t shape_index, float2 pos);
    void SetCurShapeCenter(uint32_t shape_index, float2 pos);
    void Reset(uint32_t shape_index);
    void ResetAll();

private:
    SResult InitShaders();

private:
    Technique*          m_pLiquildTech = nullptr;
	RHIMeshPtr	        m_pLiquidMesh = nullptr;

    LiquidGlassParam    m_Param;
    RHIGpuBufferPtr     m_pMvpCbBuffer = nullptr;
    RHIGpuBufferPtr     m_pParamCbBuffer = nullptr;

    SpringMassDamperPtr     m_pSpringMassDamper[Num_Shapes] = { nullptr, nullptr };
    SpringMassDamperState   m_States[Num_Shapes] = { SpringMassDamperState::Playing, SpringMassDamperState::Playing };
    ShapeType               m_InitShapeType[Num_Shapes];
    float2                  m_InitShapeRadius[Num_Shapes];
    float2                  m_InitShapeCenter[Num_Shapes];
    

	float               m_fDuration = 0.0f;
};

SEEK_NAMESPACE_END
