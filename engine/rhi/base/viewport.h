#pragma once

#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

struct Viewport
{
    Viewport() {}
    Viewport(int32_t _left, int32_t _top, uint32_t _width, uint32_t _height) 
        : left(_left)
        , top(_top)
        , width(_width)
        , height(_height) 
    {}
    
    uint32_t Width()
    {
        return width;
    }
    
    uint32_t Height()
    {
        return height;
    }
    
    int32_t Left()
    {
        return left;
    }
    
    int32_t Right()
    {
        return left + width;
    }
    
    int32_t Top()
    {
        return top;
    }
    
    int32_t Bottom()
    {
        return top + height;
    }
    
    bool IsSameSize(const Viewport& rhs)
    {
        return Valid() && rhs.Valid() && (width == rhs.width) && (height == rhs.height);
    }

    int32_t left = 0;
    int32_t top = 0;
    uint32_t width = 0;
    uint32_t height = 0;

    bool operator==(const Viewport& rhs) 
    {
        if (this == &rhs)
            return true;
        else
            return (left == rhs.left) && (top == rhs.top) && (width == rhs.width) && (height == rhs.height);
    }

    bool operator != (const Viewport& rhs)
    {
        return !(*this == rhs);
    }

    bool Valid() const
    {
        return (width != 0) && (height != 0);
    }
};

SEEK_NAMESPACE_END
