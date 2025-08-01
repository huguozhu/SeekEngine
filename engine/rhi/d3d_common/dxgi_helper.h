﻿#pragma once

#include "kernel/kernel.h"
#include "rhi/d3d_common/d3d_adapter.h"
#include "rhi/d3d_common/d3d_common_predeclare.h"

SEEK_NAMESPACE_BEGIN

class DxgiHelper 
{
public:
    virtual SResult Init(int32_t preferred_adapter, bool debug);
    virtual void Uninit();

    D3DAdapterPtr   ActiveAdapter();
    IDXGIFactory*   GetDXGIFactory()  { return m_pDxgiFactory.Get(); }
    IDXGIFactory1*  GetDXGIFactory1() { return m_pDxgiFactory1.Get(); }
    IDXGIFactory2*  GetDXGIFactory2() { return m_pDxgiFactory2.Get(); }
    IDXGIFactory3*  GetDXGIFactory3() { return m_pDxgiFactory3.Get(); }
    IDXGIFactory4*  GetDXGIFactory4() { return m_pDxgiFactory4.Get(); }
    IDXGIFactory5*  GetDXGIFactory5() { return m_pDxgiFactory5.Get(); }
    IDXGIFactory6*  GetDXGIFactory6() { return m_pDxgiFactory6.Get(); }
    

    uint8_t         GetDxgiSubVerion() { return m_iDxgiSubVer; }

protected:
    IDXGIFactoryPtr     m_pDxgiFactory  = nullptr;
    IDXGIFactory1Ptr    m_pDxgiFactory1 = nullptr;
    IDXGIFactory2Ptr    m_pDxgiFactory2 = nullptr;
    IDXGIFactory3Ptr    m_pDxgiFactory3 = nullptr;
    IDXGIFactory4Ptr    m_pDxgiFactory4 = nullptr;
    IDXGIFactory5Ptr    m_pDxgiFactory5 = nullptr;
    IDXGIFactory6Ptr    m_pDxgiFactory6 = nullptr;

    std::vector<D3DAdapterPtr> m_vAdapterList;
    uint32_t m_iCurAdapterNo = INVALID_ADAPTER_INDEX;
    uint8_t m_iDxgiSubVer = 0;

    IDXGraphicsAnalysisPtr m_pGraphicsAnalysis = nullptr;

};

using DxgiHelperPtr = std::shared_ptr<DxgiHelper>;


SEEK_NAMESPACE_END
