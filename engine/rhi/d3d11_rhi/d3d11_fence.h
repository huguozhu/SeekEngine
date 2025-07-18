#pragma once
#include "rhi/base/rhi_definition.h"
#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "rhi/base/rhi_fence.h"
#include "kernel/context.h"

#include <atomic>

SEEK_NAMESPACE_BEGIN


class D3D11RHIFence : public RHIFence
{
public:
	D3D11RHIFence(Context* context)
		:RHIFence(context)
	{}
	virtual ~D3D11RHIFence() {}

	virtual uint64_t Signal() override;
	virtual void Wait(uint64_t value) override;
	virtual bool IsCompleted(uint64_t value) override;

private:
	std::map<uint64_t, ID3D11QueryPtr> m_mFences;
	std::atomic<uint64_t> m_iFenceVal{ 0 };
};




SEEK_NAMESPACE_END
