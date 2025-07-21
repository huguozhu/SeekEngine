#pragma once

#include "kernel/kernel.h"
#include "rhi/d3d12/d3d12_predeclare.h"

#include <mutex>

SEEK_NAMESPACE_BEGIN


/******************************************************************************
* D3D12GpuDescriptorPage
*******************************************************************************/
class D3D12GpuDescriptorPage
{
public:
	D3D12GpuDescriptorPage(Context* context, int32_t size, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags);

	D3D12GpuDescriptorPage(D3D12GpuDescriptorPage&& rhs) noexcept;
	D3D12GpuDescriptorPage& operator=(D3D12GpuDescriptorPage&& rhs) noexcept;

	ID3D12DescriptorHeap* GetHeap() const noexcept { return m_pHeap.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const noexcept { return m_hCpuHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const noexcept { return m_hGpuHandle; }

private:
	ID3D12DescriptorHeapPtr m_pHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_hCpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_hGpuHandle;
};


/******************************************************************************
* D3D12GpuDescriptorBlock
*******************************************************************************/
class D3D12GpuDescriptorBlock
{
public:
	D3D12GpuDescriptorBlock() noexcept;
	D3D12GpuDescriptorBlock(D3D12GpuDescriptorBlock&& rhs) noexcept;
	D3D12GpuDescriptorBlock& operator=(D3D12GpuDescriptorBlock&& rhs) noexcept;

	ID3D12DescriptorHeap* GetHeap() const noexcept { return m_pHeap.Get(); }
	int32_t GetOffset() const { return m_iOffset; }

	uint32_t GetSize() const { return m_iSize; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const noexcept { return m_hCpuHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const noexcept { return m_hGpuHandle; }

	void Reset() noexcept;
	void Reset(D3D12GpuDescriptorPage const& page, uint32_t offset, uint32_t size) noexcept;

private:
	ID3D12DescriptorHeapPtr m_pHeap = nullptr;
	uint32_t m_iOffset = 0;
	uint32_t m_iSize = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE m_hCpuHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_hGpuHandle = {};
};

/******************************************************************************
* D3D12DescriptorAllocator
*******************************************************************************/
class D3D12GpuDescriptorAllocator
{
public:
	D3D12GpuDescriptorAllocator(Context* context, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags);

	D3D12GpuDescriptorAllocator(D3D12GpuDescriptorAllocator&& rhs);
	D3D12GpuDescriptorAllocator& operator=(D3D12GpuDescriptorAllocator&& rhs);

	uint32_t DescriptorSize() const;

	D3D12GpuDescriptorBlock Allocate(uint32_t size);
	void Deallocate(D3D12GpuDescriptorBlock&& desc_block, uint64_t fence_value);
	void Renew(D3D12GpuDescriptorBlock& desc_block, uint64_t fence_value, uint32_t size);

	void ClearStallPages(uint64_t fence_value);
	void Clear();

private:
	void Allocate(std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuDescriptorBlock& desc_block, uint32_t size);
	void Deallocate(std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuDescriptorBlock& desc_block, uint64_t fence_value);

private:
	Context* m_pContext = nullptr;
	D3D12_DESCRIPTOR_HEAP_TYPE const m_eType;
	D3D12_DESCRIPTOR_HEAP_FLAGS const m_iFlags;

	std::mutex m_AllocationMutex;

	struct PageInfo
	{
		D3D12GpuDescriptorPage page;

		struct FreeRange
		{
			uint16_t first_offset;
			uint16_t last_offset;
		};
		std::vector<FreeRange> free_list;

		struct StallRange
		{
			FreeRange free_range;
			uint64_t fence_value;
		};
		std::vector<StallRange> stall_list;
	};
	std::vector<PageInfo> m_vPages;
};
using D3D12GpuDescriptorAllocatorPtr = std::shared_ptr<D3D12GpuDescriptorAllocator>;

SEEK_NAMESPACE_END
