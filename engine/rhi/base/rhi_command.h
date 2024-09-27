#pragma once
#include "kernel/kernel.h"


SEEK_NAMESPACE_BEGIN

// likes: 
//      UE5's           RHICommand
//      bgfx's          CommandBuffer
class RHICommand
{
public:
    RHICommand();
    ~RHICommand();


    enum class RHICommandType
    {
		RendererInit,
		RendererShutdownBegin,
		CreateVertexLayout,
		CreateIndexBuffer,
		UpdateIndexBuffer,
		CreateVertexBuffer,
		UpdateVertexBuffer,
		CreateShader,
		CreateProgram,
		CreateTexture,
		UpdateTexture,
		CreateFrameBuffer,
		CreateUniform,
		UpdateViewName,
		End,
		RendererShutdownEnd,
		DestroyVertexLayout,
		DestroyIndexBuffer,
		DestroyVertexBuffer,
		DestroyDynamicIndexBuffer,
		DestroyDynamicVertexBuffer,
		DestroyShader,
		DestroyProgram,
		DestroyTexture,
		DestroyFrameBuffer,
		DestroyUniform,
		ReadTexture,
    };

};

SEEK_NAMESPACE_END