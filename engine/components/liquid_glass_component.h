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
    void SetInitShapeSize(uint32_t shape_index, float2 size);
    void SetInitShapeCenter(uint32_t shape_index, float2 pos);
    void SetCurShapeCenter(uint32_t shape_index, float2 pos);
    void Reset(uint32_t shape_index);
    void ResetAll();

    virtual void SetImage(RHITexturePtr image) override;

private:
    SResult InitShaders();

private:
    Technique*          m_pLiquildTech = nullptr;
	RHIMeshPtr	        m_pLiquidMesh = nullptr;
    RHITexturePtr       m_pBlurImage = nullptr;

    LiquidGlassParam    m_Param;
    LiquidGlassLighting m_LightingParam;
    RHIGpuBufferPtr     m_pMvpCbBuffer = nullptr;
    RHIGpuBufferPtr     m_pParamCbBuffer = nullptr;
    RHIGpuBufferPtr     m_pLightingCbBuffer = nullptr;

    // Dampers
    float mass[MAX_Num_Shapes] = { 0.005, 0.005, 0.005, 0.005, 0.005, 0.005 };
    float damping[MAX_Num_Shapes] = { 0.01, 0.01, 0.01, 0.01, 0.01, 0.01 };
    float stiffness[MAX_Num_Shapes] = { 2.0, 2.0, 2.0, 2.0, 2.0, 2.0 };
    double3 damper_centers[MAX_Num_Shapes];
    SpringMassDamperPtr     m_pSpringMassDamper[MAX_Num_Shapes] = { nullptr, nullptr, };

    SpringMassDamperState   m_States[MAX_Num_Shapes];;
    ShapeType               m_InitShapeType[MAX_Num_Shapes];
    float2                  m_InitShapeSize[MAX_Num_Shapes];
    float2                  m_InitShapeCenter[MAX_Num_Shapes];
    float3                  m_InitShapeParams[MAX_Num_Shapes];
    int                     m_iNumShapes = 0;
	float                   m_fDuration = 0.0f;
};

SEEK_NAMESPACE_END
