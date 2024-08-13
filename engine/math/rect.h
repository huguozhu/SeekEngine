#pragma once

#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

template<typename T>
class Rect
{
public:
    Rect() {}
    Rect(T left, T right, T top, T bottom)
    {
        this->left      = left;
        this->right     = right;
        this->top       = top;
        this->bottom    = bottom;
    }

    T left = {};
    T right = {};
    T top = {};
    T bottom = {};
};

SEEK_NAMESPACE_END
