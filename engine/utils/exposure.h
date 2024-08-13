#pragma once

#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

namespace Exposure
{
    float EV100(float aperture, float shutter_speed, float sensitivity) noexcept;
    float EV100FromLuminance(float luminance) noexcept;
    float EV100FromIlluminance(float illuminance) noexcept;
    float Exposure(float aperture, float shutter_speed, float sensitivity) noexcept;
    float Exposure(float ev100) noexcept;
    float Luminance(float aperture, float shutter_speed, float sensitivity) noexcept;
    float Luminance(float ev100) noexcept;
    float Illuminance(float aperture, float shutter_speed, float sensitivity) noexcept;
    float Illuminance(float ev100) noexcept;
};

SEEK_NAMESPACE_END
