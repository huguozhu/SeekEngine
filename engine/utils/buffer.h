#pragma once

#include "kernel/kernel.h"
#include "rhi/base/format.h"
#include "utils/seek_defs.h"

SEEK_NAMESPACE_BEGIN

class Buffer
{
public:
    DISALLOW_COPY_AND_ASSIGN(Buffer)
    Buffer() {}
    Buffer(size_t size, uint8_t* data = nullptr); // If data is not null, than memory will not been delete when destructor
    virtual ~Buffer() { Free(); }

    void                    Create(size_t size, uint8_t* data); // Can be called multiple times
    bool                    Valid() const { return m_pData != nullptr; }
    bool                    Alloc (size_t size);
    bool                    Expand(size_t size);

    uint8_t*                Data() const { return m_pData; }
    size_t                  Size() const { return m_iSize; }

    virtual SEEKPicture      ToSEEKPicture(uint32_t _width, uint32_t _height, uint32_t _rowPitch, SEEK_PIC_FORMAT _format) const;

protected:
    void                    Free();

    uint8_t*                m_pData = nullptr;
    size_t                  m_iSize = 0;
    size_t                  m_iBufSize = 0;

    bool                    m_bExternalMemory = false;
};

class BitmapBuffer : public Buffer
{
public:
    DISALLOW_COPY_AND_ASSIGN(BitmapBuffer)
    BitmapBuffer() {}
    BitmapBuffer(uint32_t width, uint32_t height, PixelFormat format, uint8_t* data = nullptr, uint32_t rowpitch = 0);
    virtual ~BitmapBuffer() {};

    void                    Create(uint32_t width, uint32_t height, PixelFormat format, uint8_t* data = nullptr, uint32_t rowpitch = 0); // Can be called multiple times
    bool                    Alloc (uint32_t width, uint32_t height, PixelFormat format);
    bool                    Expand(uint32_t width, uint32_t height, PixelFormat format);

    uint32_t                Width()             const { return m_iWidth; }
    uint32_t                Height()            const { return m_iHeight; }
    uint32_t                RowPitch()          const { return m_iRowPitch; }
    PixelFormat             Format()            const { return m_eFormat; }
    ColorSpace              GetColorSpace()     const { return m_eColorSpace; }
    void                    SetColorSpace(ColorSpace cs) { m_eColorSpace = cs; }

    virtual SEEKPicture     ToSEEKPicture() const;

    void                    DumpToFile(std::string path);

protected:
    void                    UpdateSize(uint32_t width, uint32_t height, PixelFormat format);

    uint32_t    m_iWidth        = 0;
    uint32_t    m_iHeight       = 0;
    uint32_t    m_iRowPitch     = 0;
    PixelFormat m_eFormat       = PixelFormat::Unknown;
    ColorSpace  m_eColorSpace   = ColorSpace::Unknown;
};

SEEK_NAMESPACE_END
