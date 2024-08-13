/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : d3d11_predeclare.h
**
**      Brief                  : predeclare in d3d11 rendering
**
**      Additional             : including Microsoft::WRL::ComPtr
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-05-28  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <dxgiformat.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <guiddef.h>
#include <wrl/client.h>

#include "kernel/kernel.h"

DVF_NAMESPACE_BEGIN

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

template <class T> using ComPtr     = Microsoft::WRL::ComPtr<T>;

using IDXGIFactory1Ptr              = ComPtr<IDXGIFactory1>;
using IDXGIFactory2Ptr              = ComPtr<IDXGIFactory2>;
using IDXGIFactory3Ptr              = ComPtr<IDXGIFactory3>;
using IDXGIFactory4Ptr              = ComPtr<IDXGIFactory4>;
using IDXGIFactory5Ptr              = ComPtr<IDXGIFactory5>;
using IDXGIFactory6Ptr              = ComPtr<IDXGIFactory6>;

using ID3D11DevicePtr               = ComPtr<ID3D11Device>;
using ID3D11DeviceContextPtr        = ComPtr<ID3D11DeviceContext>;
using IDXGIAdapter1Ptr              = ComPtr<IDXGIAdapter1>;
using ID3D11InputLayoutPtr          = ComPtr<ID3D11InputLayout>;
using IDXGISwapChainPtr             = ComPtr<IDXGISwapChain>;

using ID3D11Texture2DPtr            = ComPtr<ID3D11Texture2D>;
using ID3D11Texture3DPtr            = ComPtr<ID3D11Texture3D>;
using ID3D11RenderTargetViewPtr     = ComPtr<ID3D11RenderTargetView>;
using ID3D11DepthStencilViewPtr     = ComPtr<ID3D11DepthStencilView>;
using ID3D11ShaderResourceViewPtr   = ComPtr<ID3D11ShaderResourceView>;
using ID3D11ResourcePtr             = ComPtr<ID3D11Resource>;
using ID3D11BufferPtr               = ComPtr<ID3D11Buffer>;
using ID3DBlobPtr                   = ComPtr<ID3DBlob>;
using ID3D11UnorderedAccessViewPtr  = ComPtr<ID3D11UnorderedAccessView>;

using ID3D11VertexShaderPtr         = ComPtr<ID3D11VertexShader>;
using ID3D11PixelShaderPtr          = ComPtr<ID3D11PixelShader>;
using ID3D11GeometryShaderPtr       = ComPtr<ID3D11GeometryShader>;
using ID3D11HullShaderPtr           = ComPtr<ID3D11HullShader>;
using ID3D11DomainShaderPtr         = ComPtr<ID3D11DomainShader>;
using ID3D11ComputeShaderPtr        = ComPtr<ID3D11ComputeShader>;
using ID3D11DeviceChildPtr          = ComPtr<ID3D11DeviceChild>;
using ID3D11ShaderReflectionPtr     = ComPtr<ID3D11ShaderReflection>;

using ID3D11RasterizerStatePtr      = ComPtr<ID3D11RasterizerState>;
using ID3D11DepthStencilStatePtr    = ComPtr<ID3D11DepthStencilState>;
using ID3D11BlendStatePtr           = ComPtr<ID3D11BlendState>;

using ID3D11SamplerStatePtr         = ComPtr<ID3D11SamplerState>;
using ID3D11QueryPtr                = ComPtr<ID3D11Query>;

DVF_NAMESPACE_END
