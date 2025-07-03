#pragma once

#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

template<typename T>
class Rect
{
public:
    Rect() {}
    Rect(T _x, T _y, T _width, T _height)
    {
        this->x = _x;
        this->y = _y;
        this->width = _width;
        this->height = _height;
    }

    T x = {};
    T y = {};
    T width = {};
    T height = {};
};

SEEK_NAMESPACE_END
