#include "buffer.h"
#include "util.h"
#include <fstream>

SEEK_NAMESPACE_BEGIN

// TODO: simd instruction require more strict alignment.
// AVX2 need 32-bytes alignment
// AVX512 need 64-bytes alignment
#define MEM_DEFAULT_ALIGN   16

Buffer::Buffer(size_t size, uint8_t* data)
{
    Create(size, data);
}

void Buffer::Create(size_t size, uint8_t* data)
{
    if (data)
    {
        Free();
        this->m_pData = data;
        this->m_iSize = this->m_iBufSize = size;
        this->m_bExternalMemory = true;
    }
    else
    {
        Alloc(size);
    }
}

void Buffer::Free()
{
    if (m_pData)
    {
        if (!m_bExternalMemory)
        {
#if defined(_WIN32)
            _aligned_free(m_pData);
#else
            free(m_pData);
#endif
        }
        m_pData = nullptr;
    }
    m_iSize = 0;
    m_iBufSize = 0;
    m_bExternalMemory = false;
}

bool Buffer::Alloc(size_t size)
{
    if (m_bExternalMemory)
        return false;

    Free();

    this->m_iSize = size;
    this->m_iBufSize = seek_alignup(size, MEM_DEFAULT_ALIGN); // sometimes we may access overflow(like simd), so align size to meet these requirements

#if defined(_WIN32)
    m_pData = (uint8_t*)_aligned_malloc(this->m_iBufSize, MEM_DEFAULT_ALIGN); // align mem address, so we can use some optimize algo(like simd)
#else
    if (posix_memalign((void**)&(m_pData), MEM_DEFAULT_ALIGN, this->m_iBufSize))
        data = nullptr;
#endif
    m_bExternalMemory = false;

    if (!m_pData)
    {
        Free();
        return false;
    }
    return true;
}

bool Buffer::Expand(size_t size)
{
    if (m_bExternalMemory)
        return false;
    size_t aligned_size = seek_alignup(size, MEM_DEFAULT_ALIGN);
    if (aligned_size > m_iBufSize)
        return Alloc(size);
    this->m_iSize = size;
    return true;
}

BitmapBuffer::BitmapBuffer(uint32_t width, uint32_t height, PixelFormat format, uint8_t* data, uint32_t rowpitch, uint32_t depth)
{
    Create(width, height, format, data, rowpitch, depth);
}

void BitmapBuffer::Create(uint32_t width, uint32_t height, PixelFormat format, uint8_t* data, uint32_t rowpitch, uint32_t depth)
{
    if (data)
    {
        Free();
        if (rowpitch > 0)
            this->m_iRowPitch = rowpitch;
        else
            this->m_iRowPitch = width;
        
        this->m_iWidth = width;
        this->m_iHeight = height;
        this->m_iDepth = depth;
        this->m_eFormat = format;
        this->m_pData = data;
        this->m_iSlicePitch = this->m_iRowPitch * this->m_iHeight;
        this->m_iBufSize = this->m_iSize = this->m_iSlicePitch * this->m_iDepth;
        this->m_bExternalMemory = true;
    }
    else
    {
        Alloc(width, height, format);
    }
}

void BitmapBuffer::UpdateSize(uint32_t width, uint32_t height, PixelFormat format, uint32_t depth)
{
    this->m_iWidth = width;
    this->m_iHeight = height;
    this->m_iDepth = depth;
    this->m_eFormat = format;
    // we want each row address is align to MEM_DEFAULT_ALIGN
    // TODO: NumComponentBytes is 2 or 4, no need to align width to MEM_DEFAULT_ALIGN
    this->m_iRowPitch = seek_alignup(width, MEM_DEFAULT_ALIGN) * Formatutil::NumComponentBytes(format);
    this->m_iSlicePitch = this->m_iRowPitch * this->m_iHeight;
    this->m_iSize = this->m_iSlicePitch * this->m_iDepth;
}

bool BitmapBuffer::Alloc(uint32_t width, uint32_t height, PixelFormat format, uint32_t depth)
{
    UpdateSize(width, height, format, depth);
    return Buffer::Alloc(this->m_iSize);
}

bool BitmapBuffer::Expand(uint32_t width, uint32_t height, PixelFormat format, uint32_t depth)
{
    UpdateSize(width, height, format, depth);
    return Buffer::Expand(this->m_iSize);
}

void BitmapBuffer::DumpToFile(std::string path)
{
    std::fstream file;
    file.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

    uint32_t num_bytes = Formatutil::NumComponentBytes(m_eFormat);
    for (uint32_t j = 0; j < m_iDepth; j++)
    {
        for (uint32_t i = 0; i < m_iHeight; i++)
        {
            const char* ptr = (const char*)m_pData + m_iRowPitch * i + m_iSlicePitch * j;
            long long size = m_iWidth * num_bytes;
            file.write(ptr, size);
        }
    }
    file.close();
}

SEEK_NAMESPACE_END
