#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d12_rhi/d3d12_fence.h"
#include "rhi/d3d12_rhi/d3d12_texture.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN
D3D12Texture::D3D12Texture(Context* context, const RHITexture::Desc& tex_desc)
    :RHITexture(context, tex_desc), D3D12Resource(context)
{

}
SEEK_NAMESPACE_END
