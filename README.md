# SeekEngine

# 设计目标
## 支持多线程（第一阶段支持支D3D11）
### 针对D3D11,使用双队列+帧同步方案，存在主线程+渲染线程。
### 针对D3D12支持多线程渲染的API，计划使用Job System方案，存在多个渲染线程。
## 支持平台    
### Windows
## 支持图形API
### D3D11
### D3D12


> 后续需要实现的功能：
> 1. D3d12/Vulkan
> 2. GPU Driven Pipeline
>   a. GPU 可见性剔除（GPU Culling）
>   b. 间接渲染（Indirect Rendering），包括：使用DrawIndirect()
>   c. GPU管理场景数据（将物体位置、材质、LOD级别保存到GPU缓存中）
>   d. 动态LOD选择  
>   