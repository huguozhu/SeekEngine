#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_shader.h"

SEEK_NAMESPACE_BEGIN

class RHIFence
{
public:
	RHIFence(Context* context)
		:m_pContext(context)
	{}
	virtual ~RHIFence() {}

	virtual uint64_t Signal() = 0;
	virtual void Wait(uint64_t id) = 0;
	virtual bool IsCompleted(uint64_t id) = 0;

protected:
	Context* m_pContext = nullptr;
};



SEEK_NAMESPACE_END