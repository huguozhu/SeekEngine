#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"

SEEK_NAMESPACE_BEGIN

std::vector<float>      QuadMesh_GetVertices(float left = 0, float right = 1, float top = 0, float bottom = 1); // 0 ~ 1
RHIMeshPtr              QuadMesh_GetMesh(RHIContext& ctx, std::vector<float>* vertices = nullptr);

void                    QuadMesh_VertexRotate       (std::vector<float>& vertices, int angle, uint32_t* width = nullptr, uint32_t* height = nullptr);
void                    QuadMesh_VertexAspectRadio  (std::vector<float>& vertices, int aspect_radio_mode, float dst_aspect_ratio, float src_aspect_ratio);
void                    QuadMesh_VertexScaleX       (std::vector<float>& vertices, float scale);
void                    QuadMesh_VertexScaleY       (std::vector<float>& vertices, float scale);
void                    QuadMesh_VertexScale        (std::vector<float>& vertices, float scale);
void                    QuadMesh_VertexMirrorX      (std::vector<float>& vertices);
void                    QuadMesh_VertexMirrorY      (std::vector<float>& vertices);

void                    QuadMesh_VertexRectInfo     (std::vector<float>& vertices, float2& center, float2& corner, float2& size);

SEEK_NAMESPACE_END
