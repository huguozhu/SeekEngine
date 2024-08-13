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
        this->data = data;
        this->size = this->bufsize = size;
        this->external_memory = true;
    }
    else
    {
        Alloc(size);
    }
}

void Buffer::Free()
{
    if (data)
    {
        if (!external_memory)
        {
#if defined(_WIN32)
            _aligned_free(data);
#else
            free(data);
#endif
        }
        data = nullptr;
    }
    size = 0;
    bufsize = 0;
    external_memory = false;
}

bool Buffer::Alloc(size_t size)
{
    if (external_memory)
        return false;

    Free();

    this->size = size;
    this->bufsize = dvf_alignup(size, MEM_DEFAULT_ALIGN); // sometimes we may access overflow(like simd), so align size to meet these requirements

#if defined(_WIN32)
    data = (uint8_t*)_aligned_malloc(this->bufsize, MEM_DEFAULT_ALIGN); // align mem address, so we can use some optimize algo(like simd)
#else
    if (posix_memalign((void**)&(data), MEM_DEFAULT_ALIGN, this->bufsize))
        data = nullptr;
#endif
    external_memory = false;

    if (!data)
    {
        Free();
        return false;
    }
    return true;
}

bool Buffer::Expand(size_t size)
{
    if (external_memory)
        return false;
    size_t aligned_size = dvf_alignup(size, MEM_DEFAULT_ALIGN);
    if (aligned_size > bufsize)
        return Alloc(size);
    this->size = size;
    return true;
}

SEEKPicture Buffer::ToSEEKPicture(uint32_t _width, uint32_t _height, uint32_t _rowPitch, SEEK_PIC_FORMAT _format) const
{
    SEEKPicture pic;
    zm_memset_s(&pic, sizeof(pic), 0, sizeof(pic));
    pic.pData[0] = data;
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
            this->rowPitch = rowpitch;
        else
            this->rowPitch = width;
        this->width = width;
        this->height = height;
        this->format = format;
        this->data = data;
        this->bufsize = this->size = this->rowPitch * this->height;
        this->external_memory = true;
    }
    else
    {
        Alloc(width, height, format);
    }
}

void BitmapBuffer::UpdateSize(uint32_t width, uint32_t height, PixelFormat format)
{
    this->width = width;
    this->height = height;
    this->format = format;
    // we want each row address is align to MEM_DEFAULT_ALIGN
    // TODO: NumComponentBytes is 2 or 4, no need to align width to MEM_DEFAULT_ALIGN
    this->rowPitch = dvf_alignup(width, MEM_DEFAULT_ALIGN) * Formatutil::NumComponentBytes(format);
    this->size = this->rowPitch * this->height;
}

bool BitmapBuffer::Alloc(uint32_t width, uint32_t height, PixelFormat format)
{
    UpdateSize(width, height, format);
    return Buffer::Alloc(this->size);
}

bool BitmapBuffer::Expand(uint32_t width, uint32_t height, PixelFormat format)
{
    UpdateSize(width, height, format);
    return Buffer::Expand(this->size);
}

SEEKPicture BitmapBuffer::ToSEEKPicture() const
{
    SEEK_PIC_FORMAT _format = SEEKPicFormat_Unknown;
    if (format == PixelFormat::R8G8B8A8_UNORM ||
        format == PixelFormat::R8G8B8A8_UINT)
    {
        _format = SEEK_PIC_FORMAT::SEEKPicFormat_RGBA;
    }

    return Buffer::ToSEEKPicture(width, height, rowPitch, _format);
}

void BitmapBuffer::DumpToFile(std::string path)
{
    std::fstream file;
    file.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

    for (uint32_t j = 0; j < height; j++)
    {
        const char* ptr = (const char*)data + rowPitch * j;
        long long size = width * Formatutil::NumComponentBytes(format);
        file.write(ptr, size);
    }
    file.close();
}

SEEK_NAMESPACE_END
