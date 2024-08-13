#pragma once

#include "kernel/kernel.h"
#include "rhi/format.h"
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
    bool                    Valid() const { return data != nullptr; }
    bool                    Alloc (size_t size);
    bool                    Expand(size_t size);

    uint8_t*                Data() const { return data; }
    size_t                  Size() const { return size; }

    virtual SEEKPicture      ToSEEKPicture(uint32_t _width, uint32_t _height, uint32_t _rowPitch, SEEK_PIC_FORMAT _format) const;

protected:
    void                    Free();

    uint8_t*                data = nullptr;
    size_t                  size = 0;
    size_t                  bufsize = 0;

    bool                    external_memory = false;
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

    uint32_t                Width()             const { return width; }
    uint32_t                Height()            const { return height; }
    uint32_t                RowPitch()          const { return rowPitch; }
    PixelFormat             Format()            const { return format; }
    ColorSpace              GetColorSpace()     const { return color_space; }
    void                    SetColorSpace(ColorSpace cs) { color_space = cs; }

    virtual SEEKPicture     ToSEEKPicture() const;

    void                    DumpToFile(std::string path);

protected:
    void                    UpdateSize(uint32_t width, uint32_t height, PixelFormat format);

    uint32_t    width       = 0;
    uint32_t    height      = 0;
    uint32_t    rowPitch    = 0;
    PixelFormat format      = PixelFormat::Unknown;
    ColorSpace  color_space = ColorSpace::Unknown;
};

SEEK_NAMESPACE_END
