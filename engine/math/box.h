#pragma once

#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

template<typename T>
class Box
{
public:
    Box() {}
    Box(T _x, T _y, T _z, T _width, T _height, T _depth)
    {
        this->x         = _x;
        this->y         = _y;
        this->z         = _z;
        this->width     = _width;
        this->height    = _height;
        this->depth     = _depth;
    }

    T x = {};
    T y = {};
    T z = {};
    T width = {};
    T height = {};
    T depth = {};
};

SEEK_NAMESPACE_END
