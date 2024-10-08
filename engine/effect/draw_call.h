#pragma once
#include "kernel/kernel.h"


SEEK_NAMESPACE_BEGIN


// likes: 
//      Apple Metal's   MTLComputeCommandEncoder 
//      Vulkan's        VkCommandBuffer
//      UE5's           IRHICommandContext
//      bgfx's          RenderDraw
class ComputeDrawCall
{
public:

};

// likes: 
//      Apple Metal's   MTLRenderCommandEncoder 
//      Vulkan's        VkCommandBuffer
//      UE5's           IRHIComputeContext
//      bgfx's          RenderDraw
class RenderDrawCall : public ComputeDrawCall
{
public:



};


union DrawCallItem
{
	RenderDrawCall    draw;
	ComputeDrawCall compute;
};

class Encoder
{
public:


private:

	std::vector<DrawCallItem> m_vDrawCallItems;


};

SEEK_NAMESPACE_END