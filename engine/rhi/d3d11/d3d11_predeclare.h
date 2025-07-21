#pragma once

#include "rhi/d3d_common/d3d_common_predeclare.h"
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d3d11shader.h>

SEEK_NAMESPACE_BEGIN

using ID3D11DevicePtr               = ComPtr<ID3D11Device>;
using ID3D11DeviceContextPtr        = ComPtr<ID3D11DeviceContext>;
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

SEEK_NAMESPACE_END
