# SeekEngine

# 设计目标
## 支持多线程（第一阶段支持支D3D11）
### 针对D3D11,使用双队列+帧同步方案，存在主线程+渲染线程。
### 针对D3dD12/Vulkan/Metal等支持多线程渲染的API，计划使用Job System方案，存在多个渲染线程。
## 支持多平台    
### Windows
### iOS
## 支持多种图形API
### D3D11
### D3D11
### Vulkan
### Metal

