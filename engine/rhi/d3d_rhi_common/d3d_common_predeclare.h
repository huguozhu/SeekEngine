#pragma once

#include <windows.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <dxgiformat.h>
#include <dxgidebug.h>
#include <guiddef.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DXProgrammableCapture.h>

#include "kernel/context.h"


SEEK_NAMESPACE_BEGIN

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

template<class T> using ComPtr     = Microsoft::WRL::ComPtr<T>;
using IDXGraphicsAnalysisPtr		= ComPtr<IDXGraphicsAnalysis>;
using IDXGIFactoryPtr               = ComPtr<IDXGIFactory>;
using IDXGIFactory1Ptr              = ComPtr<IDXGIFactory1>;
using IDXGIFactory2Ptr              = ComPtr<IDXGIFactory2>;
using IDXGIFactory3Ptr              = ComPtr<IDXGIFactory3>;
using IDXGIFactory4Ptr              = ComPtr<IDXGIFactory4>;
using IDXGIFactory5Ptr              = ComPtr<IDXGIFactory5>;
using IDXGIFactory6Ptr              = ComPtr<IDXGIFactory6>;
using IDXGIAdapter1Ptr              = ComPtr<IDXGIAdapter1>;

SEEK_NAMESPACE_END
