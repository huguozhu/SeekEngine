#pragma once

#include "util.h"
#include "log.h"

SEEK_NAMESPACE_BEGIN

class Compression
{
public:
    static SResult Compress(char** dst, uint32_t &dst_len, void* src, uint32_t src_len);
    static SResult Decompress(char* dst, uint32_t &dst_len, void* src, uint32_t src_len);
};

SEEK_NAMESPACE_END
