#pragma once

#include "kernel/kernel.h"
#include "scene_manager/scene_manager.h"

#if defined(DVF_PLATFORM_ANDROID) || defined(DVF_PLATFORM_LINUX)
#else
    #define DVF_IMGUI
#endif

#ifdef DVF_IMGUI

#include "imgui.h"

SEEK_NAMESPACE_BEGIN

void IMGUI_Init();
void IMGUI_Setting_Begin();
void IMGUI_Setting_End();

void IMGUI_ShowList(std::string label, std::vector<std::string> lists, int & current_selected, int height, bool needCollaps = false, bool defaultOpen = true);
void IMGUI_ShowCameraList();
void IMGUI_ShowTransform(Context* ctx, bool supportMouse = true);
void IMGUI_ShowControl(Context* ctx, void* app_framework = nullptr);
void IMGUI_ShowSceneManager(Context* ctx);
void IMGUI_ShowAnimationControl(int32_t* animationMsg);

MeshComponentPtr CreateCoordinateAxis(Context* ctx, float thickness = 0.02);

SEEK_NAMESPACE_END

#endif
