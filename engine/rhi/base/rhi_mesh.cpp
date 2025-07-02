#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_render_buffer.h"
#include "kernel/context.h"
#include "resource/resource_mgr.h"

#define SEEK_MACRO_FILE_UID 42     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#include "shader/shared/common.h"

bool RHIMesh::IsUseIndices() const
{
    return this->GetNumIndices() != 0;
}

uint32_t RHIMesh::GetNumIndices() const
{
    uint32_t n = 0;
    if (m_pIndexBuffer)
    {
        uint32_t m = m_eIndexBufferType == IndexBufferType::UInt16 ? 2 : 4;
        n = m_pIndexBuffer->GetSize() / m;
    }
    else if (m_indicesRes)
    {
        n = m_indicesRes->_indexCount;
    }
    return n;
}

RHIRenderBufferPtr const& RHIMesh::GetIndexBuffer()
{
    if (!m_pIndexBuffer)
    {
        RHIContext& rc = m_pContext->RHIContextInstance();
        RHIRenderBufferData index_data{ m_indicesRes->_size, m_indicesRes->_data };
        m_pIndexBuffer = rc.CreateIndexBuffer((uint32_t)m_indicesRes->_size, 0, &index_data);
    }
    return m_pIndexBuffer;
}

void RHIMesh::SetIndexBuffer(RHIRenderBufferPtr buffer, IndexBufferType type)
{
    m_pIndexBuffer = buffer;
    m_eIndexBufferType = type;
    m_bDataDirty = true;
}

void RHIMesh::SetIndexBufferResource(std::shared_ptr<VertexIndicesResource>& indicesRes)
{
    m_indicesRes = indicesRes;
    m_eIndexBufferType = indicesRes->_indexBufferType;
    m_pIndexBuffer.reset();
}

uint32_t RHIMesh::GetNumVertex() const
{
    uint32_t n = 0;
    if (!m_vVertexStreams.empty())
    {
        n = m_vVertexStreams[0].render_buffer->GetSize() / m_vVertexStreams[0].stride;
    }
    return n;
}

MorphInfo& RHIMesh::GetMorphInfo()
{
    if ((!m_morphTargetRes._morphInfo.render_buffer) && m_morphTargetRes._data)
    {
        RHIContext& rc = m_pContext->RHIContextInstance();
        RHIRenderBufferData MorphTargetData(m_morphTargetRes._size, m_morphTargetRes._data);
        RHIRenderBufferPtr MorphTargetBuffer = rc.CreateByteAddressBuffer((uint32_t)m_morphTargetRes._size, RESOURCE_FLAG_GPU_READ, &MorphTargetData);
        m_morphTargetRes._morphInfo.render_buffer = MorphTargetBuffer;
        m_stMorphInfo = m_morphTargetRes._morphInfo;
    }
    if (m_morphTargetRes._refreshRHIRenderBuffer && m_morphTargetRes._data)
    {
        RHIContext& rc = m_pContext->RHIContextInstance();
        RHIRenderBufferData MorphTargetData(m_morphTargetRes._size, m_morphTargetRes._data);
        RHIRenderBufferPtr MorphTargetBuffer = rc.CreateByteAddressBuffer((uint32_t)m_morphTargetRes._size, RESOURCE_FLAG_GPU_READ, &MorphTargetData);
        m_morphTargetRes._morphInfo.render_buffer = MorphTargetBuffer;
        m_stMorphInfo.render_buffer = m_morphTargetRes._morphInfo.render_buffer;
        m_morphTargetRes._refreshRHIRenderBuffer = false;
    }

    return m_stMorphInfo;
}

uint32_t RHIMesh::NumVertexStream()
{
    return (uint32_t)GetVertexStreams().size();
}

void RHIMesh::AddVertexStream(RHIRenderBufferPtr render_buffer, uint32_t buffer_offset,
    uint32_t stride, VertexFormat format, VertexElementUsage usage, uint32_t usage_index)
{
    VertexStream* stream = nullptr;

    for (size_t i=0; i<m_vVertexStreams.size(); i++)
    {
        if (render_buffer == m_vVertexStreams[i].render_buffer)
        {
            stream = &m_vVertexStreams[i];
            break;
        }
    }

    if (!stream)
    {
        m_vVertexStreams.push_back(VertexStream());
        stream = &m_vVertexStreams[m_vVertexStreams.size() - 1];
        stream->render_buffer = render_buffer;
        stream->stride = stride;
    }

    VertexStreamLayout layout;
    layout.buffer_offset = buffer_offset;
    layout.format = format;
    layout.usage = usage;
    layout.usage_index = usage_index;
    stream->layouts.push_back(layout);

    m_bDataDirty = true;
}
void RHIMesh::AddVertexStream(VertexStream& vs)
{
    m_vVertexStreams.push_back(vs);
}
void RHIMesh::AddInstanceVertexStream(RHIRenderBufferPtr render_buffer, uint32_t buffer_offset, uint32_t stride, VertexFormat format, VertexElementUsage usage, uint32_t usage_index, uint32_t instance_count, uint32_t divisor)
{
    LOG_INFO("Mesh::AddInstanceVertexStream begin InstancCount=%d", instance_count);
    VertexStream* stream = nullptr;

    for (size_t i = 0; i < m_vVertexStreams.size(); i++)
    {
        if (render_buffer == m_vVertexStreams[i].render_buffer)
        {
            stream = &m_vVertexStreams[i];
            break;
        }
    }

    if (!stream)
    {
        m_vVertexStreams.push_back(VertexStream());
        stream = &m_vVertexStreams[m_vVertexStreams.size() - 1];
        stream->render_buffer = render_buffer;
        stream->stride = stride;
        stream->is_instance_stream = true;
    }

    if (!stream->is_instance_stream)
    {
        LOG_ERROR("Mesh::AddInstanceVertexStream try add instance layout to no-instance stream!");
        return;
    }

    VertexStreamLayout layout;
    layout.buffer_offset = buffer_offset;
    layout.format = format;
    layout.usage = usage;
    layout.usage_index = usage_index;
    layout.is_instance_attrib = true;
    layout.instance_divisor = divisor;//divisor as 1: update attrib once per draw call
    stream->layouts.push_back(layout);

    m_bDataDirty = true;
    m_bIsInstance = true;
    m_uInstancCount = instance_count;
}
VertexStream* RHIMesh::GetInstanceVertexStream(VertexElementUsage usage, uint32_t usage_index)
{
    VertexStream* pStream = nullptr;
    bool isFound = false;
    for (size_t i = 0; i < m_vVertexStreams.size(); i++)
    {
        if (m_vVertexStreams[i].is_instance_stream)
        {
            VertexStream* p_tempstream = &m_vVertexStreams[i];
            for (int j = 0; j < p_tempstream->layouts.size(); j++)
            {
                if (p_tempstream->layouts[j].usage == usage && p_tempstream->layouts[j].usage_index == usage_index)
                {
                    isFound = true;
                    break;
                }
            }
            if (isFound)
            {
                pStream = &m_vVertexStreams[i];
                break;
            }
        }
    }
    return pStream;
}

std::vector<VertexStream>& RHIMesh::GetVertexStreams()
{
    if (m_vVertexStreams.empty())
    {
        RHIContext& rc = m_pContext->RHIContextInstance();
        for (size_t i = 0; i != m_vertexAttributeRes._vertexStreams.size(); i++)
        {
            VertexStream& _vs = m_vertexAttributeRes._vertexStreams[i];
            std::shared_ptr<BufferResource>& _vsBuf = m_vertexAttributeRes._vertexBuffers[i];
            
            m_vVertexStreams.emplace_back(_vs);
            VertexStream& vs = m_vVertexStreams.back();
            RHIRenderBufferData vertex_data(_vsBuf->_size, _vsBuf->_data);
            vs.render_buffer = rc.CreateVertexBuffer((uint32_t)_vsBuf->_size, 0, &vertex_data);
        }
    }
    
    return m_vVertexStreams;
}

VertexStream& RHIMesh::GetVertexStreamByIndex(uint32_t i)
{
    return GetVertexStreams()[i];
}

void RHIMesh::SetVertexAttributeResource(VertexAttributeResource& res)
{
    m_vertexAttributeRes = res;
}

VertexAttributeResource& RHIMesh::GetVertexAttributeResource()
{
    return m_vertexAttributeRes;
};

VertexIndicesResource& RHIMesh::GetVertexIndicesResource()
{
    return *m_indicesRes;
};

MorphTargetResource& RHIMesh::GetMorphTargetResource()
{
    return m_morphTargetRes;
};

bool RHIMesh::HasMorphTarget()
{
    if ((m_stMorphInfo.morph_target_type == MorphTargetType::None || m_stMorphInfo.render_buffer == nullptr) && m_morphTargetRes._data == nullptr)
        return false;
    return true;
}
void RHIMesh::SaveToPrevMorphTargetWeights()
{
    m_stMorphInfo.prev_morph_target_weights.resize(m_stMorphInfo.morph_target_weights.size());
    m_stMorphInfo.prev_morph_target_weights = m_stMorphInfo.morph_target_weights;
}
uint32_t CalcNumMips(uint32_t width, uint32_t height)
{
    uint32_t w = width;
    uint32_t h = height;
    uint32_t n = 1;
    while (w > 1 || h > 1)
    {
        w = w >> 1;
        h = h >> 1;
        ++n;
    }
    return n;
}
static bool IsPowerOfTwo(int32_t n)
{
    return (n & n - 1) == 0;
}
static bool GenerateMipBuffers(BitmapBufferPtr src, std::vector<BitmapBufferPtr>& dst)
{
    if (src == nullptr)
        return false;
    uint32_t w = src->Width();
    uint32_t h = src->Height();
    if (!IsPowerOfTwo(w) || !IsPowerOfTwo(w))
        return false;
    PixelFormat pf = src->Format();
    uint32_t pf_size = Formatutil::NumComponentBytes(pf);
    if (pf_size != 4)
        return false;

    dst.push_back(src);
    BitmapBufferPtr pre_buf = src;
    while (w > 1 || h > 1)
    {
        w = w >> 1;
        h = h >> 1;
        
        BitmapBufferPtr cur_buf = MakeSharedPtr<BitmapBuffer>();
        cur_buf->Alloc(w, h, pf);
        uint32_t* dst_data = (uint32_t*)cur_buf->Data();

        uint8_t* src_data = (uint8_t*)pre_buf->Data();
        uint32_t src_row_pitch = pre_buf->RowPitch();

        for (uint32_t j = 0; j < h; ++j)
        {
            for (uint32_t i = 0; i < w; ++i)
            {
                uint32_t src0 = *(uint32_t*)(src_data + (j * 2    ) * src_row_pitch + (i * 2    ) * 4);
                uint32_t src1 = *(uint32_t*)(src_data + (j * 2    ) * src_row_pitch + (i * 2 + 1) * 4);
                uint32_t src2 = *(uint32_t*)(src_data + (j * 2 + 1) * src_row_pitch + (i * 2    ) * 4);
                uint32_t src3 = *(uint32_t*)(src_data + (j * 2 + 1) * src_row_pitch + (i * 2 + 1) * 4);
                uint8_t d0 = (((src0 & 0x000000ff) >> 0 ) + ((src1 & 0x000000ff) >> 0 ) + ((src2 & 0x000000ff) >> 0 ) + ((src3 & 0x000000ff) >> 0) ) / 4;
                uint8_t d1 = (((src0 & 0x0000ff00) >> 8)  + ((src1 & 0x0000ff00) >> 8 ) + ((src2 & 0x0000ff00) >> 8 ) + ((src3 & 0x0000ff00) >> 8) ) / 4;
                uint8_t d2 = (((src0 & 0x00ff0000) >> 16) + ((src1 & 0x00ff0000) >> 16) + ((src2 & 0x00ff0000) >> 16) + ((src3 & 0x00ff0000) >> 16)) / 4;
                uint8_t d3 = (((src0 & 0xff000000) >> 24) + ((src1 & 0xff000000) >> 24) + ((src2 & 0xff000000) >> 24) + ((src3 & 0xff000000) >> 24)) / 4;
                uint32_t data = (uint32_t)d0 + (uint32_t)((uint32_t)d1 << 8) + (uint32_t)((uint32_t)d2 << 16) + (uint32_t)((uint32_t)d3 << 24);
                dst_data[j * w + i] = data;
            }
        }
        
        dst.push_back(cur_buf);
        pre_buf = cur_buf;
    }
    return true;
}
static RHITexturePtr CreateTextureFromBitmap(RHIContext& rc, BitmapBufferPtr& bm, bool bColor, bool generate_mips = false)
{
    RHITexture::Desc tex_desc;
    tex_desc.type = TextureType::Tex2D;
    tex_desc.width = bm->Width();
    tex_desc.height = bm->Height();
    if (!bColor)
    {
        tex_desc.format = bm->Format();
    }
    else
    {
        if (bm->Format() == PixelFormat::R8G8B8A8_UNORM)
        {
            if (bm->GetColorSpace() == ColorSpace::Unknown || bm->GetColorSpace() == ColorSpace::sRGB)
            {
                tex_desc.format = PixelFormat::R8G8B8A8_UNORM_SRGB;
            }
            else
            {
                LOG_ERROR("Unsupported color space %d", (int)ColorSpace::sRGB);
                tex_desc.format = bm->Format();
            }
        }
        else
        {
            tex_desc.format = bm->Format();
        }
    }
    tex_desc.flags = RESOURCE_FLAG_GPU_READ;

    if (generate_mips)
    {        
        std::vector<BitmapBufferPtr> mips;
        bool suc = GenerateMipBuffers(bm, mips);
        if (suc)
        {
            tex_desc.flags = RESOURCE_FLAG_GENERATE_MIPS;
            tex_desc.num_mips = CalcNumMips(tex_desc.width, tex_desc.height);
            return rc.CreateTexture2D(tex_desc, mips);
        }
    }
    return rc.CreateTexture2D(tex_desc, bm);
}
MaterialPtr& RHIMesh::GetMaterial()
{
    if (!m_pMaterial && m_materialRes)
    {
        RHIContext& rc = m_pContext->RHIContextInstance();
        m_pMaterial = MakeSharedPtr<Material>();
        if (m_materialRes->_albedoImage)
        {
            bool generate_mips = false;
#if defined(SEEK_PLATFORM_WINDOWS)
            generate_mips = false;
#endif
            m_pMaterial->albedo_tex = CreateTextureFromBitmap(rc, m_materialRes->_albedoImage, true, generate_mips);
        }
        m_pMaterial->albedo_factor = m_materialRes->_albedoFactor;
        
        if (m_materialRes->_normalImage)
            m_pMaterial->normal_tex = CreateTextureFromBitmap(rc, m_materialRes->_normalImage, false);
        m_pMaterial->normal_scale = m_materialRes->_normalScale;

        if (m_materialRes->_normalMaskImage)
            m_pMaterial->normal_mask_tex = CreateTextureFromBitmap(rc, m_materialRes->_normalMaskImage, false);
        m_pMaterial->normal_mask_weights = m_materialRes->_normalMaskWeights;
        
        if (m_materialRes->_occlusionImage)
            m_pMaterial->occlusion_tex = CreateTextureFromBitmap(rc, m_materialRes->_occlusionImage, false);
        
        if (m_materialRes->_metallicRoughnessImage)
            m_pMaterial->metallic_roughness_tex = CreateTextureFromBitmap(rc, m_materialRes->_metallicRoughnessImage, false);
        m_pMaterial->metallic_factor = m_materialRes->_metallicFactor;
        m_pMaterial->roughness_factor = m_materialRes->_roughnessFactor;
        
        if (m_materialRes->_emmissiveImage)
            m_pMaterial->emissive_tex = CreateTextureFromBitmap(rc, m_materialRes->_emmissiveImage, true);
        m_pMaterial->emissive_factor = m_materialRes->_emissiveFactor;
        
        if (m_materialRes->_clearcoatImage)
            m_pMaterial->clearcoat_tex = CreateTextureFromBitmap(rc, m_materialRes->_clearcoatImage, false);
        m_pMaterial->clearcoat_factor = m_materialRes->_clearcoatFactor;
        
        if (m_materialRes->_clearcoatRoughnessImage)
            m_pMaterial->clearcoat_roughness_tex = CreateTextureFromBitmap(rc, m_materialRes->_clearcoatRoughnessImage, false);
        m_pMaterial->clearcoat_roughness_factor = m_materialRes->_clearcoatRoughnessFactor;
        
        if (m_materialRes->_sheenColorImage)
            m_pMaterial->sheen_color_tex = CreateTextureFromBitmap(rc, m_materialRes->_sheenColorImage, true);
        m_pMaterial->sheen_color_factor = m_materialRes->_sheenColorFactor;
        
        if (m_materialRes->_sheenRoughnessImage)
            m_pMaterial->sheen_roughness_tex = CreateTextureFromBitmap(rc, m_materialRes->_sheenRoughnessImage, false);
        m_pMaterial->sheen_roughness_factor = m_materialRes->_sheenRoughnessFactor;
        
        m_pMaterial->alpha_mode = m_materialRes->_alphaMode;
        m_pMaterial->alpha_cutoff = m_materialRes->_alphaCutoff;
        m_pMaterial->double_sided = m_materialRes->_doubleSided;
        m_pMaterial->IOR_factor = m_materialRes->_IORFactor;
    }
    return m_pMaterial;
}

template<typename T>
inline size_t vector_byte_size(const std::vector<T>& vec)
{
    return vec.size() * sizeof(T);
}

template<typename T>
inline size_t std_container_byte_size(const T& vec)
{
    return vec.size() * sizeof(T::value_type);
}

const RHIRenderBufferPtr& RHIMesh::GetMorphWeightsCBuffer()
{
    MorphInfo& mi = GetMorphInfo();
    if (!m_morphWeightsCBuffer)
    {
        m_morphWeightsCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(MAX_MORPH_SIZE * sizeof(float4), RESOURCE_FLAG_CPU_WRITE);
    }

    std::vector<float4> float4_weights;
    float4_weights.resize(mi.morph_target_weights.size());
    for (size_t i = 0; i < float4_weights.size(); i++)
        float4_weights[i] = float4(mi.morph_target_weights[i], 0.0, 0.0, 0.0);
    m_morphWeightsCBuffer->Update(float4_weights.data(), (uint32_t)float4_weights.size() * sizeof(float4));
    return m_morphWeightsCBuffer;
}
const RHIRenderBufferPtr& RHIMesh::GetPreMorphWeightsCBuffer()
{
    MorphInfo& mi = GetMorphInfo();
    if (!m_prevMorphWeightsCBuffer)
    {
        m_prevMorphWeightsCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(MAX_MORPH_SIZE * sizeof(float4), RESOURCE_FLAG_CPU_WRITE);
    }

    std::vector<float4> float4_prev_weights;
    float4_prev_weights.resize(mi.prev_morph_target_weights.size());
    for (size_t i = 0; i < float4_prev_weights.size(); i++)
        float4_prev_weights[i] = float4(mi.morph_target_weights[i], 0.0, 0.0, 0.0);
    m_prevMorphWeightsCBuffer->Update(float4_prev_weights.data(), (uint32_t)float4_prev_weights.size() * sizeof(float4));
    return m_prevMorphWeightsCBuffer;
}
const RHIRenderBufferPtr& RHIMesh::GetMorphSizeCBuffer()
{
    if (!m_morphSizeCBuffer)
    {
        m_morphSizeCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(uint32_t), RESOURCE_FLAG_CPU_WRITE);
        uint32_t morph_count = (uint32_t)m_stMorphInfo.morph_target_weights.size();
        m_morphSizeCBuffer->Update(&morph_count, sizeof(morph_count));
    }
    
    return m_morphSizeCBuffer;
}

RHIRenderBufferPtr& RHIMesh::GetMaterialInfoCBuffer()
{
    if (!m_MaterialInfoCBuffer)
    {
        m_MaterialInfoCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(MaterialInfo), RESOURCE_FLAG_CPU_WRITE);
    }
    return m_MaterialInfoCBuffer;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
