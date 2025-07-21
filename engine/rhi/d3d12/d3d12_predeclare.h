#pragma once

#include <d3d12.h>
#include <d3d12shader.h>
#include "rhi/d3d_common/d3d_common_predeclare.h"



SEEK_NAMESPACE_BEGIN

using IDXGIAdapter1Ptr              = ComPtr<IDXGIAdapter1>;
using IDXGISwapChain3Ptr            = ComPtr<IDXGISwapChain3>;

using ID3D12DebugPtr                = ComPtr<ID3D12Debug>;
using ID3D12DevicePtr               = ComPtr<ID3D12Device>;
using ID3D12CommandQueuePtr         = ComPtr<ID3D12CommandQueue>;
using ID3D12CommandSignaturePtr     = ComPtr<ID3D12CommandSignature>;
using ID3D12CommandAllocatorPtr     = ComPtr<ID3D12CommandAllocator>;
using ID3D12GraphicsCommandListPtr  = ComPtr<ID3D12GraphicsCommandList>;

using ID3D12FencePtr                = ComPtr<ID3D12Fence>;
using ID3D12DescriptorHeapPtr       = ComPtr<ID3D12DescriptorHeap>;
using ID3D12ResourcePtr             = ComPtr<ID3D12Resource>;



SEEK_NAMESPACE_END
