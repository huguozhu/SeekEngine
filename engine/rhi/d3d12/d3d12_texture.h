#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/d3d12/d3d12_resource.h"
#include "rhi/d3d12/d3d12_gpu_memory_allocator.h"
#include "rhi/d3d12/d3d12_render_view.h"

SEEK_NAMESPACE_BEGIN
/******************************************************************************
* D3D12Texture
*******************************************************************************/
class D3D12Texture : public RHITexture, public D3D12Resource
{
public:
    D3D12Texture(Context* context, const RHITexture::Desc& tex_desc);
    virtual ~D3D12Texture() override;

    virtual SResult Resolve() { return S_Success; }
    ID3D12Resource* GetD3DTexture() { return m_pD3dResource.Get(); }
    DXGI_FORMAT GetD3DFormat() { return m_eDxgiFormat; }

    D3D12SrvPtr const& GetD3DSrv();
    D3D12SrvPtr const& GetD3DSrv(uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels);
    D3D12SrvPtr const& GetD3DSrv(uint32_t array_index, CubeFaceType face, uint32_t first_level, uint32_t num_levels);

    D3D12RtvPtr const& GetD3DRtv();
    D3D12RtvPtr const& GetD3DRtv(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    D3D12RtvPtr const& GetD3DRtv(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
    D3D12RtvPtr const& GetD3DRtv(uint32_t array_index, CubeFaceType face, uint32_t mip_level);

    D3D12DsvPtr const& GetD3DDsv();
    D3D12DsvPtr const& GetD3DDsv(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    D3D12DsvPtr const& GetD3DDsv(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
    D3D12DsvPtr const& GetD3DDsv(uint32_t array_index, CubeFaceType face, uint32_t mip_level);

    D3D12UavPtr const& GetD3DUav();
    D3D12UavPtr const& GetD3DUav(uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    D3D12UavPtr const& GetD3DUav(uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
    D3D12UavPtr const& GetD3DUav(uint32_t first_array_index, uint32_t array_size, CubeFaceType first_face, uint32_t num_faces, uint32_t mip_level);

    virtual void FillSrvDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) { ErrUnreachable("Can't be called."); }
    virtual void FillSrvDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t first_level, uint32_t num_levels) { ErrUnreachable("Can't be called."); }

    virtual void FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) { ErrUnreachable("Can't be called."); }
    virtual void FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level) { ErrUnreachable("Can't be called."); }
    virtual void FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level) { ErrUnreachable("Can't be called."); }

    virtual void FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) { ErrUnreachable("Can't be called."); }
    virtual void FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level) { ErrUnreachable("Can't be called."); }
    virtual void FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level) { ErrUnreachable("Can't be called."); }

    virtual void FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) { ErrUnreachable("Can't be called."); }
    virtual void FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level) { ErrUnreachable("Can't be called."); }
    virtual void FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, CubeFaceType first_face, uint32_t num_faces, uint32_t mip_level) { ErrUnreachable("Can't be called."); }

    virtual SResult DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) { ErrUnreachable("Can't be called."); return S_Success; }
    virtual SResult DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Box<uint32_t>* box = nullptr) { ErrUnreachable("Can't be called."); return S_Success; }
    virtual SResult DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) { ErrUnreachable("Can't be called."); return S_Success; }

protected:
    SResult DoCreate(D3D12_RESOURCE_DIMENSION dim, uint32_t width, uint32_t height, uint32_t depth, uint32_t array_size, std::span<BitmapBufferPtr> const& bitmap_datas);

protected:
    DXGI_FORMAT             m_eDxgiFormat = DXGI_FORMAT_UNKNOWN;
    D3D12GpuMemoryBlock     m_MappedMemoryBlock;

    uint32_t m_iMappedXOffset;
    uint32_t m_iMappedYOffset;
    uint32_t m_iMappedZOffset;
    uint32_t m_iMappedWidth;
    uint32_t m_iMappedHeight;
    uint32_t m_iMappedDepth;

    std::unordered_map<size_t, D3D12SrvPtr> m_mD3dSrvs;
    std::unordered_map<size_t, D3D12RtvPtr> m_mD3dRtvs;
    std::unordered_map<size_t, D3D12DsvPtr> m_mD3dDsvs;
    std::unordered_map<size_t, D3D12UavPtr> m_mD3dUavs;

};
using D3D12TexturePtr = std::shared_ptr<D3D12Texture>;


/******************************************************************************
* D3D12Texture2D
*******************************************************************************/
class D3D12Texture2D : public D3D12Texture
{
public:
    D3D12Texture2D(Context* context, const RHITexture::Desc& tex_desc);
    D3D12Texture2D(Context* context, ID3D12ResourcePtr const& d3d_tex);

    SResult Create(std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult Update(std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult Resolve() override;
    SResult DumpSubResource2D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr);

protected:
    void FillSrvDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) override;
    void FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) override;
    void FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) override;
    void FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level) override;

private:
    //void FillStageTexture2DDesc(D3D12_TEXTURE2D_DESC& desc);
    //D3D12_TEXTURE2D_DESC m_d3dTexture2DDesc = {};
};
using D3D12Texture2DPtr = std::shared_ptr<D3D12Texture2D>;

/******************************************************************************
* D3D11TextureCube
*******************************************************************************/
class D3D12TextureCube : public D3D12Texture
{
public:
    D3D12TextureCube(Context* context, const RHITexture::Desc& tex_desc);

    SResult Create(std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult Update(std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult DumpSubResourceCube(BitmapBufferPtr bitmap_data, CubeFaceType face, uint32_t array_index = 0, uint32_t mip_level = 0, Rect<uint32_t>* rect = nullptr) override;

private:
    void FillSrvDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels);
    void FillSrvDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t first_level, uint32_t num_levels);
    void FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    void FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level);
    void FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    void FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, CubeFaceType face, uint32_t mip_level);
    void FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    void FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, CubeFaceType first_face, uint32_t num_faces, uint32_t mip_level);
};
using D3D12TextureCubePtr = std::shared_ptr<D3D12TextureCube>;


/******************************************************************************
* D3D12Texture3D
*******************************************************************************/
class D3D12Texture3D : public D3D12Texture
{
public:
    D3D12Texture3D(Context* context, const RHITexture::Desc& tex_desc);
    SResult Create(std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult Update(std::span<BitmapBufferPtr> const& bitmap_datas) override;
    SResult DumpSubResource3D(BitmapBufferPtr bitmap_data, uint32_t array_index = 0, uint32_t mip_level = 0, Box<uint32_t>* box = nullptr);

private:
    void FillSrvDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels);
    void FillRtvDesc(D3D12_RENDER_TARGET_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
    void FillDsvDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
    void FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);
    void FillUavDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);
};
using D3D12Texture3DPtr = std::shared_ptr<D3D12Texture3D>;

SEEK_NAMESPACE_END
