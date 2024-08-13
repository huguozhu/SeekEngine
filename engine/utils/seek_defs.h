#ifndef _common_include_dvf_dvf_defs_h
#define _common_include_dvf_dvf_defs_h

SEEK_NAMESPACE_BEGIN

enum SEEK_PIC_FORMAT
{
    SEEKPicFormat_Unknown,
    SEEKPicFormat_RGB24,
    SEEKPicFormat_BGR24,
    SEEKPicFormat_BGR24Flip,

    // With alpha
    SEEKPicFormat_RGBA,       // packed
    SEEKPicFormat_BGRA,       // packed
    SEEKPicFormat_BGRA32_P,   // packed R*(A/256),G*(A/256),B*(A/256), A
    SEEKPicFormat_AYUV_I420,  // planar A + I420
    SEEKPicFormat_YUVA_I420,  // planar I420 + A, Data[3] is A

    // Without alpha
    SEEKPicFormat_NV12,
    SEEKPicFormat_I420,
    SEEKPicFormat_I444,
    SEEKPicFormat_YV12,
    SEEKPicFormat_YUY2,
    SEEKPicFormat_UYVY,
};

enum SEEKYUVColorRange
{
    SEEKYUVColorRange_Limited,
    SEEKYUVColorRange_Full,
};

struct SEEKPicture
{
    uint8_t* pData[4];
    uint32_t iRowPitch[4];
    void*    pGpuData;  // CVPixelBufferRef in Apple

    uint32_t iWidth;
    uint32_t iHeight;
    int32_t  iRotateAngle; // 0~360
    SEEK_PIC_FORMAT eFormat;
    SEEKYUVColorRange eYUVColorRange;
};

struct SEEKFloat2
{
    float x;
    float y;
};

struct SEEKFloat3
{
    float x;
    float y;
    float z;
};

struct SEEKRect
{
    float left;
    float top;
    float width;
    float height;
};

SEEK_NAMESPACE_END

#endif
