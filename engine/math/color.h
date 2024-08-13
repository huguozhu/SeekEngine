#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include <array>

SEEK_NAMESPACE_BEGIN

class Color
{
public:
	Color() { m_iColor = 0xffffffff; }
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) :m_r(r), m_g(g), m_b(b), m_a(a) {}
    Color(uint32_t color) :m_iColor(color) {}
    Color(float3 c) :m_r(uint8_t(c.x() * 255)), m_g(uint8_t(c.y() * 255)), m_b(uint8_t(c.z() * 255)), m_a(255) {}
    Color(float4 c) :m_r(uint8_t(c.x() * 255)), m_g(uint8_t(c.y() * 255)), m_b(uint8_t(c.z() * 255)), m_a(uint8_t(c.w() * 255)) {}

    uint32_t    const& DWColor() const { return m_iColor; }
    uint8_t     const& R() const { return m_r; }
    uint8_t     const& G() const { return m_g; }
    uint8_t     const& B() const { return m_b; }
    uint8_t     const& A() const { return m_a; }

    float3 ToFloat3();
    float4 ToFloat4();
    float4 ToFloat4WithoutAlpha();

    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color DefaultAmbientColor;
	static const Color Maroon;
	static const Color DarkRed;
	static const Color Brown;
	static const Color Firebrick;
	static const Color Crimson;
	static const Color Tomato;
	static const Color Coral;
	static const Color IndianRed;
	static const Color LightCoral;
	static const Color DarkSalmon;
	static const Color Salmon;
	static const Color LightSalmon;
	static const Color OrangeRed;
	static const Color DarkOrange;
	static const Color Orange;
	static const Color Gold;
	static const Color DarkGoldenRod;
	static const Color GoldenRod;
	static const Color PaleGoldenRod;
	static const Color DarkKhaki;
	static const Color Khaki;
	static const Color Olive;
	static const Color YellowGreen;
	static const Color DarkOliveGreen;
	static const Color OliveDrab;
	static const Color LawnGreen;
	static const Color ChartReuse;
	static const Color GreenYellow;
	static const Color DarkGreen;
	static const Color ForestGreen;
	static const Color Lime;
	static const Color LimeGreen;
	static const Color LightGreen;
	static const Color PaleGreen;
	static const Color DarkSeaGreen;
	static const Color MediumSpringGreen;
	static const Color SpringGreen;
	static const Color SeaGreen;
	static const Color MediumAquaMarine;
	static const Color MediumSeaGreen;
	static const Color LightSeaGreen;
	static const Color DarkSlateGray;
	static const Color Teal;
	static const Color DarkCyan;
	static const Color Aqua;
	static const Color Cyan;
	static const Color LightCyan;
	static const Color DarkTurquoise;
	static const Color Turquoise;
	static const Color MediumTurquoise;
	static const Color PaleTurquoise;
	static const Color Aquamarine;
	static const Color PowderBlue;
	static const Color CadetBlue;
	static const Color SteelBlue;
	static const Color CornflowerBlue;
	static const Color DeepSkyBlue;
	static const Color DodgerBlue;
	static const Color LightBlue;
	static const Color SkyBlue;
	static const Color LightSkyBlue;
	static const Color MidnightBlue;
	static const Color Navy;
	static const Color DarkBlue;
	static const Color MediumBlue;
	static const Color RoyalBlue;
	static const Color BlueViolet;
	static const Color Indigo;
	static const Color DarkSlateBlue;
	static const Color SlateBlue;
	static const Color MediumSlateBlue;
	static const Color MediumPurple;
	static const Color DarkMagenta;
	static const Color DarkViolet;
	static const Color DarkOrchid;
	static const Color MediumOrchid;
	static const Color Purple;
	static const Color Thistle;
	static const Color Plum;
	static const Color Violet;
	static const Color Magenta;
	static const Color Orchid;
	static const Color MediumVioletRed;
	static const Color PaleVioletRed;
	static const Color DeepPink;
	static const Color HotPink;
	static const Color LightPink;
	static const Color Pink;
	static const Color AntiqueWhite;
	static const Color Beige;
	static const Color Bisque;
	static const Color BlanchedAlmond;
	static const Color Wheat;
	static const Color CornSilk;
	static const Color LemonChiffon;
	static const Color LightGoldenRodYellow;
	static const Color LightYellow;
	static const Color SaddleBrown;
	static const Color Sienna;
	static const Color Chocolate;
	static const Color Peru;
	static const Color SandyBrown;
	static const Color BurlyWood;
	static const Color Tan;
	static const Color RosyBrown;
	static const Color Moccasin;
	static const Color NavajoWhite;
	static const Color PeachPuff;
	static const Color MistyRose;
	static const Color LavenderBlush;
	static const Color Linen;
	static const Color OldLace;
	static const Color PapayaWhip;
	static const Color SeaShell;
	static const Color MintCream;
	static const Color SlateGray;
	static const Color LightSlateGray;
	static const Color LightSteelBlue;
	static const Color Lavender;
	static const Color FloralWhite;
	static const Color AliceBlue;
	static const Color GhostWhite;
	static const Color Honeydew;
	static const Color Ivory;
	static const Color Azure;
	static const Color Snow;
	static const Color DimGrey;
	static const Color Grey;
	static const Color DarkGrey;
	static const Color Silver;
	static const Color LightGrey;
	static const Color Gainsboro;
	static const Color WhiteSmoke;

private:
    union
    {
        struct { uint8_t m_r, m_g, m_b, m_a; };
        uint32_t m_iColor;
    };
};

using LinearColor = Color;
using sRGBColor = Color;

SEEK_NAMESPACE_END
