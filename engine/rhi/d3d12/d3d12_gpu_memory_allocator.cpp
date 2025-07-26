#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_gpu_memory_allocator.h"
#include "rhi/d3d12/d3d12_rhi_context.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN
uint32_t constexpr PageSize = 2 * 1024 * 1024;
uint32_t constexpr SegmentSize = 512;
uint32_t constexpr SegmentMask = SegmentSize - 1;

/******************************************************************************
* D3D12GpuMemoryPage
*******************************************************************************/
D3D12GpuMemoryPage::D3D12GpuMemoryPage(Context* context, bool is_upload, uint32_t size_in_bytes)
    :m_pContext(context), m_bIsUpLoad(is_upload)
{
	D3D12Context& rc = static_cast<D3D12Context&>(context->RHIContextInstance());
	ID3D12Device* pDevice = rc.GetD3D12Device();

	D3D12_RESOURCE_STATES init_state;
	D3D12_HEAP_PROPERTIES heap_prop;
	if (is_upload)
	{
		init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
		heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	}
	else
	{
		init_state = D3D12_RESOURCE_STATE_COPY_DEST;
		heap_prop.Type = D3D12_HEAP_TYPE_READBACK;
	}
	heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_prop.CreationNodeMask = 0;
	heap_prop.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC res_desc;
	res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	res_desc.Alignment = 0;
	res_desc.Width = size_in_bytes;
	res_desc.Height = 1;
	res_desc.DepthOrArraySize = 1;
	res_desc.MipLevels = 1;
	res_desc.Format = DXGI_FORMAT_UNKNOWN;
	res_desc.SampleDesc.Count = 1;
	res_desc.SampleDesc.Quality = 0;
	res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	SEEK_THROW_IFFAIL(pDevice->CreateCommittedResource(
		&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, init_state, nullptr, IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())));

	D3D12_RANGE const read_range{ 0, 0 };
	SEEK_THROW_IFFAIL(m_pResource->Map(0, &read_range, &m_pCpuAddr));

	m_GpuAddr = m_pResource->GetGPUVirtualAddress();
}
D3D12GpuMemoryPage::~D3D12GpuMemoryPage()
{
	if (m_pResource)
	{
		D3D12_RANGE const write_range{ 0, 0 };
		m_pResource->Unmap(0, m_bIsUpLoad ? nullptr : &write_range);
	}
}
D3D12GpuMemoryPage::D3D12GpuMemoryPage(D3D12GpuMemoryPage&& rhs)
	: m_pContext(rhs.m_pContext), m_bIsUpLoad(rhs.m_bIsUpLoad), m_pResource(std::move(rhs.m_pResource)), m_pCpuAddr(std::move(rhs.m_pCpuAddr)),
	m_GpuAddr(std::move(rhs.m_GpuAddr))
{

}



/******************************************************************************
* D3D12GpuMemoryPage
*******************************************************************************/
D3D12GpuMemoryBlock::D3D12GpuMemoryBlock() = default;
D3D12GpuMemoryBlock::D3D12GpuMemoryBlock(D3D12GpuMemoryBlock && rhs) = default;
void D3D12GpuMemoryBlock::Reset()
{
	m_pResource = nullptr;
	m_iOffset = 0;
	m_iSize = 0;
	m_pCpuAddr = nullptr;
	m_GpuAddr = {};
}
void D3D12GpuMemoryBlock::Reset(D3D12GpuMemoryPage const& page, uint32_t offset, uint32_t size)
{
	m_pResource = page.GetResource();
	m_iOffset = offset;
	m_iSize = size;
	m_pCpuAddr = page.GetCpuAddress<uint8_t>() + offset;
	m_GpuAddr  = page.GetGpuAddress() + offset;
}


/******************************************************************************
* D3D12GpuMemoryAllocator
*******************************************************************************/
D3D12GpuMemoryAllocator::D3D12GpuMemoryAllocator(Context* context, bool is_upload)
	: m_pContext(context), m_bIsUpload(is_upload)
{
}
D3D12GpuMemoryAllocator::D3D12GpuMemoryAllocator(D3D12GpuMemoryAllocator&& rhs)
	: m_pContext(rhs.m_pContext), m_bIsUpload(rhs.m_bIsUpload), m_vPages(std::move(rhs.m_vPages)), m_vLargePages(std::move(rhs.m_vLargePages))
{
}
D3D12GpuMemoryBlock D3D12GpuMemoryAllocator::Allocate(uint32_t size_in_bytes, uint32_t alignment)
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);

	D3D12GpuMemoryBlock mem_block;
	this->Allocate(lock, mem_block, size_in_bytes, alignment);
	return mem_block;
}

void D3D12GpuMemoryAllocator::Allocate([[maybe_unused]] std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuMemoryBlock& mem_block,
	uint32_t size_in_bytes, uint32_t alignment)
{
	SEEK_ASSERT(alignment <= SegmentSize);
	uint32_t const aligned_size = ((size_in_bytes + alignment - 1) / alignment * alignment + SegmentMask) & ~SegmentMask;

	if (aligned_size > PageSize)
	{
		auto& large_page = m_vLargePages.emplace_back(D3D12GpuMemoryPage(m_pContext, m_bIsUpload, aligned_size));
		mem_block.Reset(large_page, 0, size_in_bytes);
		return;
	}

	for (auto& page_info : m_vPages)
	{
		auto const iter = std::lower_bound(page_info.free_list.begin(), page_info.free_list.end(), aligned_size,
			[](PageInfo::FreeRange const& free_range, uint32_t s) { return free_range.first_offset + s > free_range.last_offset; });
		if (iter != page_info.free_list.end())
		{
			uint32_t const aligned_offset = (iter->first_offset + alignment - 1) / alignment * alignment;
			mem_block.Reset(page_info.page, aligned_offset, size_in_bytes);
			iter->first_offset += aligned_size;
			if (iter->first_offset == iter->last_offset)
			{
				page_info.free_list.erase(iter);
			}

			return;
		}
	}

	D3D12GpuMemoryPage new_page(m_pContext, m_bIsUpload, PageSize);
	mem_block.Reset(new_page, 0, size_in_bytes);
	m_vPages.emplace_back(PageInfo{ std::move(new_page), {{aligned_size, PageSize}}, {} });
}

void D3D12GpuMemoryAllocator::Deallocate(D3D12GpuMemoryBlock&& mem_block, uint64_t fence_value)
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);
	this->Deallocate(lock, mem_block, fence_value);
}

void D3D12GpuMemoryAllocator::Deallocate(
	[[maybe_unused]] std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuMemoryBlock& mem_block, uint64_t fence_value)
{
	if (mem_block.GetSize() <= PageSize)
	{
		for (auto& page : m_vPages)
		{
			if (page.page.GetResource() == mem_block.GetResource())
			{
				uint32_t const offset = mem_block.GetOffset() & ~SegmentMask;
				uint32_t const size = (mem_block.GetOffset() + mem_block.GetSize() - offset + SegmentMask) & ~SegmentMask;
				page.stall_list.push_back({ {offset, offset + size}, fence_value });
				return;
			}
		}

		ErrUnreachable("This memory block is not allocated by this allocator");
	}
}

void D3D12GpuMemoryAllocator::Renew(D3D12GpuMemoryBlock& mem_block, uint64_t fence_value, uint32_t size_in_bytes, uint32_t alignment)
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);

	this->Deallocate(lock, mem_block, fence_value);
	this->Allocate(lock, mem_block, size_in_bytes, alignment);
}

void D3D12GpuMemoryAllocator::ClearStallPages(uint64_t fence_value)
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);

	for (auto& page : m_vPages)
	{
		for (auto stall_iter = page.stall_list.begin(); stall_iter != page.stall_list.end();)
		{
			if (stall_iter->fence_value <= fence_value)
			{
				auto const free_iter = std::lower_bound(page.free_list.begin(), page.free_list.end(),
					stall_iter->free_range.first_offset, [](PageInfo::FreeRange const& free_range, uint32_t first_offset) {
						return free_range.first_offset < first_offset;
					});
				if (free_iter == page.free_list.end())
				{
					if (page.free_list.empty() || (page.free_list.back().last_offset != stall_iter->free_range.first_offset))
					{
						page.free_list.emplace_back(std::move(stall_iter->free_range));
					}
					else
					{
						page.free_list.back().last_offset = stall_iter->free_range.last_offset;
					}
				}
				else if (free_iter->first_offset != stall_iter->free_range.last_offset)
				{
					bool merge_with_prev = false;
					if (free_iter != page.free_list.begin())
					{
						auto const prev_free_iter = std::prev(free_iter);
						if (prev_free_iter->last_offset == stall_iter->free_range.first_offset)
						{
							prev_free_iter->last_offset = stall_iter->free_range.last_offset;
							merge_with_prev = true;
						}
					}

					if (!merge_with_prev)
					{
						page.free_list.emplace(free_iter, std::move(stall_iter->free_range));
					}
				}
				else
				{
					free_iter->first_offset = stall_iter->free_range.first_offset;
					if (free_iter != page.free_list.begin())
					{
						auto const prev_free_iter = std::prev(free_iter);
						if (prev_free_iter->last_offset == free_iter->first_offset)
						{
							prev_free_iter->last_offset = free_iter->last_offset;
							page.free_list.erase(free_iter);
						}
					}
				}

				stall_iter = page.stall_list.erase(stall_iter);
			}
			else
			{
				++stall_iter;
			}
		}
	}

	m_vLargePages.clear();
}

void D3D12GpuMemoryAllocator::Clear()
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);

	m_vPages.clear();
	m_vLargePages.clear();
}

SEEK_NAMESPACE_END
