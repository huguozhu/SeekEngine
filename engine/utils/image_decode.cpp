#include "image_decode.h"
#include "turbojpeg.h"
#include "png.h"
#include "log.h"
#include "file.h"
#include <fstream>

SEEK_NAMESPACE_BEGIN

static BitmapBufferPtr ImageDecodeJPEG(uint8_t* data, size_t size)
{
    tjhandle handle = tjInitDecompress();
    if (!handle)
        return nullptr;

    BitmapBufferPtr bitmap = MakeSharedPtr<BitmapBuffer>();
    if (!bitmap)
    {
        tjDestroy(handle);
        return nullptr;
    }
    do {
        int width, height, subSamp;
        int ret = tjDecompressHeader2(handle, data, size, &width, &height, &subSamp);
        if (ret != 0)
            break;

        if (!bitmap->Alloc(width, height, PixelFormat::R8G8B8A8_UNORM))
            break;

        ret = tjDecompress2(handle, data, size, (uint8_t*)bitmap->Data(), width, bitmap->RowPitch(), height, TJPF_RGBA, 0);
        if (ret != 0)
            break;

        tjDestroy(handle);
        return bitmap;
    } while (0);

    tjDestroy(handle);
    return nullptr;
}

struct PngReadBuf
{
    uint8_t* pData;
    uint32_t nSize;
    uint32_t nOffset;
};

static BitmapBufferPtr ImageDecodePNG(uint8_t* data, size_t size)
{
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;

    if (!png_check_sig(data, 8))
        return nullptr;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        return nullptr;

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return nullptr;
    }

    auto png_read_fn = [](png_structp pPngStruct, png_bytep  pData, png_size_t nLen) {
        PngReadBuf* pngBuf = (PngReadBuf*)png_get_io_ptr(pPngStruct);
        if (!pngBuf)
            return;

        if ((int)(pngBuf->nOffset + nLen) <= pngBuf->nSize)
        {
            memcpy_s(pData, nLen, pngBuf->pData + pngBuf->nOffset, nLen);
            pngBuf->nOffset += uint32_t(nLen);
        }
        else
        {
            if (pngBuf->nOffset < pngBuf->nSize)
            {
                memcpy_s(pData, nLen, pngBuf->pData + pngBuf->nOffset, pngBuf->nSize - pngBuf->nOffset);
                pngBuf->nOffset = pngBuf->nSize;
            }
        }
    };

    PngReadBuf pngBuf;
    pngBuf.pData = data;
    pngBuf.nSize = (uint32_t)size;
    pngBuf.nOffset = 0;
    png_set_read_fn(png_ptr, &pngBuf, png_read_fn);

    setjmp(png_jmpbuf(png_ptr));

    png_read_info(png_ptr, info_ptr);

    uint32_t width, height;
    int color_type, bit_depth, interlace_method, compression_method, filter_method;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_method, &compression_method, &filter_method);

    if (width == 0 || height == 0)
    {
        LOG_ERROR("invalid image dimension %ux%u", width, height);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return nullptr;
    }

    if (bit_depth != 8 && bit_depth != 16)
    {
        LOG_ERROR("unsupported bit depth %d", bit_depth);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return nullptr;
    }
    
    if (color_type != PNG_COLOR_TYPE_RGB_ALPHA && color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_GRAY)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        LOG_ERROR("unsupported pixel format %d", color_type);
        return nullptr;
    }

    if (bit_depth == 16)
    {
        // swap byte order, from big endian to little endian
        png_set_swap(png_ptr);
    }

    png_byte channels = png_get_channels(png_ptr, info_ptr);
    if (channels == 3)
    {
        // for RGB, we output RGBA
        png_set_filler(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
    }

    // after change properties, update info_ptr, then we can get the updated information
    png_read_update_info(png_ptr, info_ptr);
    // get updated channels
    channels = png_get_channels(png_ptr, info_ptr);

    BitmapBufferPtr bitmap = MakeSharedPtr<BitmapBuffer>();
    if (!bitmap)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return nullptr;
    }

    // TODO: fill the color space information (like sRGB).
    PixelFormat _formats_map[4][2] = {
        // bit depth              8                                16
        /*1 channel*/ { PixelFormat::R8_UNORM,       PixelFormat::Unknown},
        /*2 channel*/ { PixelFormat::R8G8_UNORM,     PixelFormat::Unknown},
        /*3 channel*/ { PixelFormat::Unknown,        PixelFormat::Unknown},
        /*4 channel*/ { PixelFormat::R8G8B8A8_UNORM, PixelFormat::R16G16B16A16_UNORM},
    };
    
    PixelFormat format = _formats_map[channels - 1][bit_depth / 8 - 1];
    if (format == PixelFormat::Unknown)
    {
        LOG_ERROR("current combination: bit_depth:%d, channels:%d is not supported", bit_depth, channels);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return nullptr;
    }

    if (!bitmap->Alloc(width, height, format))
    {
        LOG_ERROR("alloc bitmap fail");
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return nullptr;
    }

    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    if (bitmap->RowPitch() < rowbytes)
    {
        LOG_ERROR("small rowpitch than need %d<%d", bitmap->RowPitch(), rowbytes);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return nullptr;
    }
    
    std::vector<png_bytep> row_pointers(height);
    for (int i = 0; i < height; i++)
    {
        row_pointers[i] = bitmap->Data() + i * bitmap->RowPitch();
    }
    
    png_read_image(png_ptr, row_pointers.data());
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return bitmap;
}

//static BitmapBufferPtr ImageDecodeHDR(uint8_t* data, uint32_t size)
//{
//    int width, height, nrComponents;
//    float* hdr_data = stbi_loadf_from_memory((stbi_uc const*)data, size, &width, &height, &nrComponents, 0);
//    if (!data)
//        return nullptr;
//
//    PixelFormat fmt = PixelFormat::R32G32B32A32_FLOAT;
//    if (nrComponents == 3)
//        fmt = PixelFormat::R32G32B32_FLOAT;
//    BitmapBufferPtr bitmap = MakeSharedPtr<BitmapBuffer>(width, height, fmt);
//
//    if (!bitmap->Alloc(width, height, fmt))
//    {
//        stbi_image_free(hdr_data);
//        return nullptr;
//    }
//
//    uint8_t* pData = bitmap->Data();
//    uint32_t copy_size = bitmap->Size();
//    memcpy_s(pData, copy_size, hdr_data, copy_size);
//
//    return bitmap;
//}

BitmapBufferPtr ImageDecode(uint8_t* data, size_t size, ImageType imageType /* = ImageType::UNKNOWN */ )
{
    switch (imageType)
    {
        case ImageType::JPEG:
            return ImageDecodeJPEG(data, size);
        case ImageType::PNG:
            return ImageDecodePNG(data, size);
        //case ImageType::HDR:
        //    return ImageDecodeHDR(data, size);
        default:
        {
            BitmapBufferPtr bitmap = ImageDecodeJPEG(data, size);
            if (!bitmap)
                bitmap = ImageDecodePNG(data, size);
        //    if (!bitmap)
        //        bitmap = ImageDecodeHDR(data, size);
            return bitmap;
        }
    }
}

BitmapBufferPtr ImageDecodeFromFile(std::string const& filepath, ImageType imageType)
{
    std::vector<uint8_t> fileData;
    SEEK_RET_NULL_IF_FAIL(read_file_content(filepath.c_str(), "rb", fileData));
    return ImageDecode((uint8_t*)fileData.data(), fileData.size(), imageType);
}

SEEK_NAMESPACE_END
