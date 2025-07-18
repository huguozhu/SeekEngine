#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_texture.h"

SEEK_NAMESPACE_BEGIN

class D3D12Texture : public RHITexture
{
public:
    D3D12Texture(Context* context, const RHITexture::Desc& tex_desc);

    void UpdateResourceBarrier(ID3D12GraphicsCommandList* cmd_list, uint32_t sub_res, D3D12_RESOURCE_STATES target_state);

protected:


};

SEEK_NAMESPACE_END
