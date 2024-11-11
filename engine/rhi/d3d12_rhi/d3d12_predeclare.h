#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <guiddef.h>
#include <wrl/client.h>
#include <DXProgrammableCapture.h>
#include "kernel/kernel.h"


SEEK_NAMESPACE_BEGIN

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

template <class T> using ComPtr     = Microsoft::WRL::ComPtr<T>;
using IDXGraphicsAnalysisPtr		= ComPtr<IDXGraphicsAnalysis>;
using IDXGIAdapter1Ptr				= ComPtr<IDXGIAdapter1>;
using IDXGIFactory1Ptr              = ComPtr<IDXGIFactory1>;
using IDXGIFactory2Ptr              = ComPtr<IDXGIFactory2>;
using IDXGIFactory3Ptr              = ComPtr<IDXGIFactory3>;
using IDXGIFactory4Ptr              = ComPtr<IDXGIFactory4>;
using IDXGIFactory5Ptr              = ComPtr<IDXGIFactory5>;
using IDXGIFactory6Ptr              = ComPtr<IDXGIFactory6>;

using ID3D12DebugPtr                = ComPtr<ID3D12Debug>;

using ID3D11DevicePtr               = ComPtr<ID3D12Device>;
//using ID3D11DeviceContextPtr        = ComPtr<ID3D12DeviceContext>;
//using IDXGIAdapter1Ptr              = ComPtr<IDXGIAdapter1>;
//using ID3D11InputLayoutPtr          = ComPtr<ID3D112nputLayout>;
//using IDXGISwapChainPtr             = ComPtr<IDXGISwapChain>;
//
//using ID3D11Texture2DPtr            = ComPtr<ID3D12Texture2D>;
//using ID3D11Texture3DPtr            = ComPtr<ID3D12Texture3D>;
//using ID3D11RenderTargetViewPtr     = ComPtr<ID3D12RenderTargetView>;
//using ID3D11DepthStencilViewPtr     = ComPtr<ID3D12DepthStencilView>;
//using ID3D11ShaderResourceViewPtr   = ComPtr<ID3D12ShaderResourceView>;
//using ID3D11ResourcePtr             = ComPtr<ID3D12Resource>;
//using ID3D11BufferPtr               = ComPtr<ID3D12Buffer>;
//using ID3DBlobPtr                   = ComPtr<ID3DBlob>;
//using ID3D11UnorderedAccessViewPtr  = ComPtr<ID3D12UnorderedAccessView>;
//
//using ID3D11VertexShaderPtr         = ComPtr<ID3D12VertexShader>;
//using ID3D11PixelShaderPtr          = ComPtr<ID3D12PixelShader>;
//using ID3D11GeometryShaderPtr       = ComPtr<ID3D12GeometryShader>;
//using ID3D11HullShaderPtr           = ComPtr<ID3D12HullShader>;
//using ID3D11DomainShaderPtr         = ComPtr<ID3D12DomainShader>;
//using ID3D11ComputeShaderPtr        = ComPtr<ID3D12ComputeShader>;
//using ID3D11DeviceChildPtr          = ComPtr<ID3D12DeviceChild>;
//using ID3D11ShaderReflectionPtr     = ComPtr<ID3D11ShaderReflection>;
//
//using ID3D11RasterizerStatePtr      = ComPtr<ID3D12RasterizerState>;
//using ID3D11DepthStencilStatePtr    = ComPtr<ID3D12DepthStencilState>;
//using ID3D11BlendStatePtr           = ComPtr<ID3D12BlendState>;
//
//using ID3D11SamplerStatePtr         = ComPtr<ID3D12SamplerState>;
//using ID3D11QueryPtr                = ComPtr<ID3D12Query>;

SEEK_NAMESPACE_END
