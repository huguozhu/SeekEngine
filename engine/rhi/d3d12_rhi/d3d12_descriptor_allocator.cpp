#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d12_rhi/d3d12_descriptor_allocator.h"
#include "rhi/d3d12_rhi/d3d12_rhi_context.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN
static uint32_t s_DescriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES]{};

static uint16_t constexpr DescriptorPageSizes[] = { 32 * 1024, 1 * 1024, 8 * 1024, 4 * 1024 };

void UpdateDescriptorSize(Context* context,  D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	static bool has_update = false;
	if (s_DescriptorSize[type] == 0)
	{
		D3D12RHIContext& rc = static_cast<D3D12RHIContext&>(context->RHIContextInstance());
		ID3D12Device* pDevice = rc.GetD3D12Device();
		s_DescriptorSize[type] = pDevice->GetDescriptorHandleIncrementSize(type);
	}
}

/******************************************************************************
* D3D12GpuDescriptorPage
*******************************************************************************/
D3D12GpuDescriptorPage::D3D12GpuDescriptorPage(Context* context, int32_t size, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    D3D12RHIContext& rc = static_cast<D3D12RHIContext&>(context->RHIContextInstance());
    ID3D12Device* pDevice = rc.GetD3D12Device();

	D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_heap_desc;
	cbv_srv_heap_desc.Type = type;
	cbv_srv_heap_desc.NumDescriptors = size;
	cbv_srv_heap_desc.Flags = flags;
	cbv_srv_heap_desc.NodeMask = 0;
	SEEK_THROW_IFFAIL(pDevice->CreateDescriptorHeap(&cbv_srv_heap_desc, IID_PPV_ARGS(m_pHeap.ReleaseAndGetAddressOf())));
	m_hCpuHandle = m_pHeap->GetCPUDescriptorHandleForHeapStart();
	m_hGpuHandle = m_pHeap->GetGPUDescriptorHandleForHeapStart();
}

D3D12GpuDescriptorPage::D3D12GpuDescriptorPage(D3D12GpuDescriptorPage&& other) noexcept = default;
D3D12GpuDescriptorPage& D3D12GpuDescriptorPage::operator=(D3D12GpuDescriptorPage&& other) noexcept = default;


/******************************************************************************
* D3D12GpuDescriptorBlock
*******************************************************************************/
D3D12GpuDescriptorBlock::D3D12GpuDescriptorBlock() noexcept = default;
D3D12GpuDescriptorBlock::D3D12GpuDescriptorBlock(D3D12GpuDescriptorBlock&& other) noexcept = default;
D3D12GpuDescriptorBlock& D3D12GpuDescriptorBlock::operator=(D3D12GpuDescriptorBlock&& other) noexcept = default;
void D3D12GpuDescriptorBlock::Reset() noexcept
{
	m_pHeap = nullptr;
	m_iOffset = 0;
	m_iSize = 0;

	m_hCpuHandle = {};
	m_hGpuHandle = {};
}

void D3D12GpuDescriptorBlock::Reset(D3D12GpuDescriptorPage const& page, uint32_t offset, uint32_t size) noexcept
{
	m_pHeap = page.GetHeap();
	m_iOffset = offset;
	m_iSize = size;

	uint32_t const desc_size = s_DescriptorSize[m_pHeap->GetDesc().Type];

	m_hCpuHandle = { page.GetCpuHandle().ptr + offset * desc_size };
	m_hGpuHandle = { page.GetGpuHandle().ptr + offset * desc_size };
}


/******************************************************************************
* D3D12GpuDescriptorBlock
*******************************************************************************/
D3D12GpuDescriptorAllocator::D3D12GpuDescriptorAllocator(Context* context, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
	: m_pContext(context), m_eType(type), m_iFlags(flags)
{
}
D3D12GpuDescriptorAllocator::D3D12GpuDescriptorAllocator(D3D12GpuDescriptorAllocator&& other)
	: m_pContext(other.m_pContext), m_eType(other.m_eType), m_iFlags(other.m_iFlags), m_vPages(std::move(other.m_vPages))
{
}

D3D12GpuDescriptorAllocator& D3D12GpuDescriptorAllocator::operator=(D3D12GpuDescriptorAllocator&& other)
{
	if (this != &other)
	{
		SEEK_ASSERT(m_eType == other.m_eType);
		SEEK_ASSERT(m_iFlags == other.m_iFlags);
		m_vPages = std::move(other.m_vPages);
	}
	return *this;
}

uint32_t D3D12GpuDescriptorAllocator::DescriptorSize() const
{
	UpdateDescriptorSize(m_pContext, m_eType);
	return s_DescriptorSize[m_eType];
}

D3D12GpuDescriptorBlock D3D12GpuDescriptorAllocator::Allocate(uint32_t size)
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);

	D3D12GpuDescriptorBlock desc_block;
	this->Allocate(lock, desc_block, size);
	return desc_block;
}

void D3D12GpuDescriptorAllocator::Allocate(
	[[maybe_unused]] std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuDescriptorBlock& desc_block, uint32_t size)
{
	UpdateDescriptorSize(m_pContext, m_eType);

	uint16_t const default_page_size = DescriptorPageSizes[m_eType];
	SEEK_ASSERT(size <= default_page_size);

	for (auto& page_info : m_vPages)
	{
		auto const iter = std::lower_bound(page_info.free_list.begin(), page_info.free_list.end(), size,
			[](PageInfo::FreeRange const& free_range, uint32_t s) { return free_range.first_offset + s > free_range.last_offset; });
		if (iter != page_info.free_list.end())
		{
			desc_block.Reset(page_info.page, iter->first_offset, size);
			iter->first_offset += static_cast<uint16_t>(size);
			if (iter->first_offset == iter->last_offset)
			{
				page_info.free_list.erase(iter);
			}

			return;
		}
	}

	D3D12GpuDescriptorPage new_page(m_pContext, default_page_size, m_eType, m_iFlags);
	desc_block.Reset(new_page, 0, size);
	m_vPages.emplace_back(PageInfo{ std::move(new_page), {{static_cast<uint16_t>(size), default_page_size}}, {} });
}

void D3D12GpuDescriptorAllocator::Deallocate(D3D12GpuDescriptorBlock&& desc_block, uint64_t fence_value)
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);
	this->Deallocate(lock, desc_block, fence_value);
}

void D3D12GpuDescriptorAllocator::Deallocate(
	[[maybe_unused]] std::lock_guard<std::mutex>& proof_of_lock, D3D12GpuDescriptorBlock& desc_block, uint64_t fence_value)
{
	uint16_t const default_page_size = DescriptorPageSizes[m_eType];

	if (desc_block.GetSize() <= default_page_size)
	{
		for (auto& page : m_vPages)
		{
			if (page.page.GetHeap() == desc_block.GetHeap())
			{
				page.stall_list.push_back(
					{ {static_cast<uint16_t>(desc_block.GetOffset()), static_cast<uint16_t>(desc_block.GetOffset() + desc_block.GetSize())},
						fence_value });
				return;
			}
		}

		ErrUnreachable("This descriptor block is not allocated by this allocator");
	}
}

void D3D12GpuDescriptorAllocator::Renew(D3D12GpuDescriptorBlock& desc_block, uint64_t fence_value, uint32_t size)
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);
	this->Deallocate(lock, desc_block, fence_value);
	this->Allocate(lock, desc_block, size);
}

void D3D12GpuDescriptorAllocator::ClearStallPages(uint64_t fence_value)
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
}

void D3D12GpuDescriptorAllocator::Clear()
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);
	m_vPages.clear();
}


SEEK_NAMESPACE_END
