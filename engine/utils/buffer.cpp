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

SEEKPicture Buffer::ToSEEKPicture(uint32_t _width, uint32_t _height, uint32_t _rowPitch, SEEK_PIC_FORMAT _format) const
{
    SEEKPicture pic;
    seek_memset_s(&pic, sizeof(pic), 0, sizeof(pic));
    pic.pData[0] = m_pData;
    pic.iRowPitch[0] = _rowPitch;
    pic.iWidth = _width;
    pic.iHeight = _height;
    pic.eFormat = _format;

    switch (_format)
    {
        case SEEK_PIC_FORMAT::SEEKPicFormat_AYUV_I420:
            pic.iRowPitch[1] = _rowPitch;
            pic.iRowPitch[2] = _rowPitch >> 1;
            pic.iRowPitch[3] = _rowPitch >> 1;
            pic.pData[1] = pic.pData[0] + pic.iRowPitch[0] * _height;
            pic.pData[2] = pic.pData[1] + pic.iRowPitch[1] * _height;
            pic.pData[3] = pic.pData[2] + pic.iRowPitch[2] * (_height >> 1);
            break;
        case SEEK_PIC_FORMAT::SEEKPicFormat_YUVA_I420:
            pic.iRowPitch[1] = _rowPitch >> 1;
            pic.iRowPitch[2] = _rowPitch >> 1;
            pic.iRowPitch[3] = _rowPitch;
            pic.pData[1] = pic.pData[0] + pic.iRowPitch[0] * _height;
            pic.pData[2] = pic.pData[1] + pic.iRowPitch[1] * (_height >> 1);
            pic.pData[3] = pic.pData[2] + pic.iRowPitch[2] * (_height >> 1);
            break;
        case SEEK_PIC_FORMAT::SEEKPicFormat_I420:
            pic.iRowPitch[1] = _rowPitch >> 1;
            pic.iRowPitch[2] = _rowPitch >> 1;
            pic.pData[1] = pic.pData[0] + pic.iRowPitch[0] * _height;
            pic.pData[2] = pic.pData[1] + pic.iRowPitch[1] * (_height >> 1);
            break;
        case SEEK_PIC_FORMAT::SEEKPicFormat_I444:
            pic.iRowPitch[1] = _rowPitch;
            pic.iRowPitch[2] = _rowPitch;
            pic.pData[1] = pic.pData[0] + pic.iRowPitch[0] * _height;
            pic.pData[2] = pic.pData[1] + pic.iRowPitch[1] * _height;
            break;
        case SEEK_PIC_FORMAT::SEEKPicFormat_NV12:
            pic.iRowPitch[1] = _rowPitch;
            pic.pData[1] = pic.pData[0] + pic.iRowPitch[0] * _height;
            break;
        case SEEK_PIC_FORMAT::SEEKPicFormat_YV12:
            pic.iRowPitch[1] = _rowPitch >> 1;
            pic.iRowPitch[2] = _rowPitch >> 1;
            pic.pData[1] = pic.pData[0] + pic.iRowPitch[0] * _height;
            pic.pData[2] = pic.pData[1] + pic.iRowPitch[1] * (_height >> 1);
            break;
        default:
        {
            break;
        }
    }
    return pic;
}

BitmapBuffer::BitmapBuffer(uint32_t width, uint32_t height, PixelFormat format, uint8_t* data, uint32_t rowpitch)
{
    Create(width, height, format, data, rowpitch);
}

void BitmapBuffer::Create(uint32_t width, uint32_t height, PixelFormat format, uint8_t* data, uint32_t rowpitch)
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
        this->m_eFormat = format;
        this->m_pData = data;
        this->m_iBufSize = this->m_iSize = this->m_iRowPitch * this->m_iHeight;
        this->m_bExternalMemory = true;
    }
    else
    {
        Alloc(width, height, format);
    }
}

void BitmapBuffer::UpdateSize(uint32_t width, uint32_t height, PixelFormat format)
{
    this->m_iWidth = width;
    this->m_iHeight = height;
    this->m_eFormat = format;
    // we want each row address is align to MEM_DEFAULT_ALIGN
    // TODO: NumComponentBytes is 2 or 4, no need to align width to MEM_DEFAULT_ALIGN
    this->m_iRowPitch = seek_alignup(width, MEM_DEFAULT_ALIGN) * Formatutil::NumComponentBytes(format);
    this->m_iSize = this->m_iRowPitch * this->m_iHeight;
}

bool BitmapBuffer::Alloc(uint32_t width, uint32_t height, PixelFormat format)
{
    UpdateSize(width, height, format);
    return Buffer::Alloc(this->m_iSize);
}

bool BitmapBuffer::Expand(uint32_t width, uint32_t height, PixelFormat format)
{
    UpdateSize(width, height, format);
    return Buffer::Expand(this->m_iSize);
}

SEEKPicture BitmapBuffer::ToSEEKPicture() const
{
    SEEK_PIC_FORMAT _format = SEEKPicFormat_Unknown;
    if (m_eFormat == PixelFormat::R8G8B8A8_UNORM ||
        m_eFormat == PixelFormat::R8G8B8A8_UINT)
    {
        _format = SEEK_PIC_FORMAT::SEEKPicFormat_RGBA;
    }

    return Buffer::ToSEEKPicture(m_iWidth, m_iHeight, m_iRowPitch, _format);
}

void BitmapBuffer::DumpToFile(std::string path)
{
    std::fstream file;
    file.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

    for (uint32_t j = 0; j < m_iHeight; j++)
    {
        const char* ptr = (const char*)m_pData + m_iRowPitch * j;
        long long size = m_iWidth * Formatutil::NumComponentBytes(m_eFormat);
        file.write(ptr, size);
    }
    file.close();
}

SEEK_NAMESPACE_END
