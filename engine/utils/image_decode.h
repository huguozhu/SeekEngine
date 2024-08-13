#pragma once

#include "util.h"
#include "buffer.h"

SEEK_NAMESPACE_BEGIN

enum class ImageType
{
    UNKNOWN,
    JPEG,
    PNG,
    //HDR,
    NUM,
};

BitmapBufferPtr ImageDecode(uint8_t* data, size_t size, ImageType imageType = ImageType::UNKNOWN);
BitmapBufferPtr ImageDecodeFromFile(std::string const& filepath, ImageType imageType = ImageType::UNKNOWN);

SEEK_NAMESPACE_END
