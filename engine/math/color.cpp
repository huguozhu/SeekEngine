#include "math/color.h"

SEEK_NAMESPACE_BEGIN

// https://www.rapidtables.com/web/color/RGB_Color.html
const Color Color::DefaultAmbientColor = float3(0.1f, 0.1f, 0.1f);
const Color Color::Maroon = float3(0.501961f, 0.0f, 0.0f);
const Color Color::DarkRed = float3(0.545098f, 0.0f, 0.0f);
const Color Color::Brown = float3(0.647059f, 0.164706f, 0.164706f);
const Color Color::Firebrick = float3(0.698039f, 0.133333f, 0.133333f);
const Color Color::Crimson = float3(0.862745f, 0.0784314f, 0.235294f);
const Color Color::Red = float3(1.0f, 0.0f, 0.0f);
const Color Color::Tomato = float3(1.0f, 0.388235f, 0.278431f);
const Color Color::Coral = float3(1.0f, 0.498039f, 0.313726f);
const Color Color::IndianRed = float3(0.803922f, 0.360784f, 0.360784f);
const Color Color::LightCoral = float3(0.941176f, 0.501961f, 0.501961f);
const Color Color::DarkSalmon = float3(0.913725f, 0.588235f, 0.478431f);
const Color Color::Salmon = float3(0.980392f, 0.501961f, 0.447059f);
const Color Color::LightSalmon = float3(1.0f, 0.627451f, 0.478431f);
const Color Color::OrangeRed = float3(1.0f, 0.270588f, 0.0f);
const Color Color::DarkOrange = float3(1.0f, 0.54902f, 0.0f);
const Color Color::Orange = float3(1.0f, 0.647059f, 0.0f);
const Color Color::Gold = float3(1.0f, 0.843137f, 0.0f);
const Color Color::DarkGoldenRod = float3(0.721569f, 0.52549f, 0.0431373f);
const Color Color::GoldenRod = float3(0.854902f, 0.647059f, 0.12549f);
const Color Color::PaleGoldenRod = float3(0.933333f, 0.909804f, 0.666667f);
const Color Color::DarkKhaki = float3(0.741176f, 0.717647f, 0.419608f);
const Color Color::Khaki = float3(0.941176f, 0.901961f, 0.54902f);
const Color Color::Olive = float3(0.501961f, 0.501961f, 0.0f);
const Color Color::Yellow = float3(1.0f, 1.0f, 0.0f);
const Color Color::YellowGreen = float3(0.603922f, 0.803922f, 0.196078f);
const Color Color::DarkOliveGreen = float3(0.333333f, 0.419608f, 0.184314f);
const Color Color::OliveDrab = float3(0.419608f, 0.556863f, 0.137255f);
const Color Color::LawnGreen = float3(0.486275f, 0.988235f, 0.0f);
const Color Color::ChartReuse = float3(0.498039f, 1.0f, 0.0f);
const Color Color::GreenYellow = float3(0.678431f, 1.0f, 0.184314f);
const Color Color::DarkGreen = float3(0.0f, 0.392157f, 0.0f);
const Color Color::Green = float3(0.0f, 0.501961f, 0.0f);
const Color Color::ForestGreen = float3(0.133333f, 0.545098f, 0.133333f);
const Color Color::Lime = float3(0.0f, 1.0f, 0.0f);
const Color Color::LimeGreen = float3(0.196078f, 0.803922f, 0.196078f);
const Color Color::LightGreen = float3(0.564706f, 0.933333f, 0.564706f);
const Color Color::PaleGreen = float3(0.596078f, 0.984314f, 0.596078f);
const Color Color::DarkSeaGreen = float3(0.560784f, 0.737255f, 0.560784f);
const Color Color::MediumSpringGreen = float3(0.0f, 0.980392f, 0.603922f);
const Color Color::SpringGreen = float3(0.0f, 1.0f, 0.498039f);
const Color Color::SeaGreen = float3(0.180392f, 0.545098f, 0.341176f);
const Color Color::MediumAquaMarine = float3(0.4f, 0.803922f, 0.666667f);
const Color Color::MediumSeaGreen = float3(0.235294f, 0.701961f, 0.443137f);
const Color Color::LightSeaGreen = float3(0.12549f, 0.698039f, 0.666667f);
const Color Color::DarkSlateGray = float3(0.184314f, 0.309804f, 0.309804f);
const Color Color::Teal = float3(0.0f, 0.501961f, 0.501961f);
const Color Color::DarkCyan = float3(0.0f, 0.545098f, 0.545098f);
const Color Color::Aqua = float3(0.0f, 1.0f, 1.0f);
const Color Color::Cyan = float3(0.0f, 1.0f, 1.0f);
const Color Color::LightCyan = float3(0.878431f, 1.0f, 1.0f);
const Color Color::DarkTurquoise = float3(0.0f, 0.807843f, 0.819608f);
const Color Color::Turquoise = float3(0.25098f, 0.878431f, 0.815686f);
const Color Color::MediumTurquoise = float3(0.282353f, 0.819608f, 0.8f);
const Color Color::PaleTurquoise = float3(0.686275f, 0.933333f, 0.933333f);
const Color Color::Aquamarine = float3(0.498039f, 1.0f, 0.831373f);
const Color Color::PowderBlue = float3(0.690196f, 0.878431f, 0.901961f);
const Color Color::CadetBlue = float3(0.372549f, 0.619608f, 0.627451f);
const Color Color::SteelBlue = float3(0.27451f, 0.509804f, 0.705882f);
const Color Color::CornflowerBlue = float3(0.392157f, 0.584314f, 0.929412f);
const Color Color::DeepSkyBlue = float3(0.0f, 0.74902f, 1.0f);
const Color Color::DodgerBlue = float3(0.117647f, 0.564706f, 1.0f);
const Color Color::LightBlue = float3(0.678431f, 0.847059f, 0.901961f);
const Color Color::SkyBlue = float3(0.529412f, 0.807843f, 0.921569f);
const Color Color::LightSkyBlue = float3(0.529412f, 0.807843f, 0.980392f);
const Color Color::MidnightBlue = float3(0.0980392f, 0.0980392f, 0.439216f);
const Color Color::Navy = float3(0.0f, 0.0f, 0.501961f);
const Color Color::DarkBlue = float3(0.0f, 0.0f, 0.545098f);
const Color Color::MediumBlue = float3(0.0f, 0.0f, 0.803922f);
const Color Color::Blue = float3(0.0f, 0.0f, 1.0f);
const Color Color::RoyalBlue = float3(0.254902f, 0.411765f, 0.882353f);
const Color Color::BlueViolet = float3(0.541176f, 0.168627f, 0.886275f);
const Color Color::Indigo = float3(0.294118f, 0.0f, 0.509804f);
const Color Color::DarkSlateBlue = float3(0.282353f, 0.239216f, 0.545098f);
const Color Color::SlateBlue = float3(0.415686f, 0.352941f, 0.803922f);
const Color Color::MediumSlateBlue = float3(0.482353f, 0.407843f, 0.933333f);
const Color Color::MediumPurple = float3(0.576471f, 0.439216f, 0.858824f);
const Color Color::DarkMagenta = float3(0.545098f, 0.0f, 0.545098f);
const Color Color::DarkViolet = float3(0.580392f, 0.0f, 0.827451f);
const Color Color::DarkOrchid = float3(0.6f, 0.196078f, 0.8f);
const Color Color::MediumOrchid = float3(0.729412f, 0.333333f, 0.827451f);
const Color Color::Purple = float3(0.501961f, 0.0f, 0.501961f);
const Color Color::Thistle = float3(0.847059f, 0.74902f, 0.847059f);
const Color Color::Plum = float3(0.866667f, 0.627451f, 0.866667f);
const Color Color::Violet = float3(0.933333f, 0.509804f, 0.933333f);
const Color Color::Magenta = float3(1.0f, 0.0f, 1.0f);
const Color Color::Orchid = float3(0.854902f, 0.439216f, 0.839216f);
const Color Color::MediumVioletRed = float3(0.780392f, 0.0823529f, 0.521569f);
const Color Color::PaleVioletRed = float3(0.858824f, 0.439216f, 0.576471f);
const Color Color::DeepPink = float3(1.0f, 0.0784314f, 0.576471f);
const Color Color::HotPink = float3(1.0f, 0.411765f, 0.705882f);
const Color Color::LightPink = float3(1.0f, 0.713726f, 0.756863f);
const Color Color::Pink = float3(1.0f, 0.752941f, 0.796078f);
const Color Color::AntiqueWhite = float3(0.980392f, 0.921569f, 0.843137f);
const Color Color::Beige = float3(0.960784f, 0.960784f, 0.862745f);
const Color Color::Bisque = float3(1.0f, 0.894118f, 0.768627f);
const Color Color::BlanchedAlmond = float3(1.0f, 0.921569f, 0.803922f);
const Color Color::Wheat = float3(0.960784f, 0.870588f, 0.701961f);
const Color Color::CornSilk = float3(1.0f, 0.972549f, 0.862745f);
const Color Color::LemonChiffon = float3(1.0f, 0.980392f, 0.803922f);
const Color Color::LightGoldenRodYellow = float3(0.980392f, 0.980392f, 0.823529f);
const Color Color::LightYellow = float3(1.0f, 1.0f, 0.878431f);
const Color Color::SaddleBrown = float3(0.545098f, 0.270588f, 0.0745098f);
const Color Color::Sienna = float3(0.627451f, 0.321569f, 0.176471f);
const Color Color::Chocolate = float3(0.823529f, 0.411765f, 0.117647f);
const Color Color::Peru = float3(0.803922f, 0.521569f, 0.247059f);
const Color Color::SandyBrown = float3(0.956863f, 0.643137f, 0.376471f);
const Color Color::BurlyWood = float3(0.870588f, 0.721569f, 0.529412f);
const Color Color::Tan = float3(0.823529f, 0.705882f, 0.54902f);
const Color Color::RosyBrown = float3(0.737255f, 0.560784f, 0.560784f);
const Color Color::Moccasin = float3(1.0f, 0.894118f, 0.709804f);
const Color Color::NavajoWhite = float3(1.0f, 0.870588f, 0.678431f);
const Color Color::PeachPuff = float3(1.0f, 0.854902f, 0.72549f);
const Color Color::MistyRose = float3(1.0f, 0.894118f, 0.882353f);
const Color Color::LavenderBlush = float3(1.0f, 0.941176f, 0.960784f);
const Color Color::Linen = float3(0.980392f, 0.941176f, 0.901961f);
const Color Color::OldLace = float3(0.992157f, 0.960784f, 0.901961f);
const Color Color::PapayaWhip = float3(1.0f, 0.937255f, 0.835294f);
const Color Color::SeaShell = float3(1.0f, 0.960784f, 0.933333f);
const Color Color::MintCream = float3(0.960784f, 1.0f, 0.980392f);
const Color Color::SlateGray = float3(0.439216f, 0.501961f, 0.564706f);
const Color Color::LightSlateGray = float3(0.466667f, 0.533333f, 0.6f);
const Color Color::LightSteelBlue = float3(0.690196f, 0.768627f, 0.870588f);
const Color Color::Lavender = float3(0.901961f, 0.901961f, 0.980392f);
const Color Color::FloralWhite = float3(1.0f, 0.980392f, 0.941176f);
const Color Color::AliceBlue = float3(0.941176f, 0.972549f, 1.0f);
const Color Color::GhostWhite = float3(0.972549f, 0.972549f, 1.0f);
const Color Color::Honeydew = float3(0.941176f, 1.0f, 0.941176f);
const Color Color::Ivory = float3(1.0f, 1.0f, 0.941176f);
const Color Color::Azure = float3(0.941176f, 1.0f, 1.0f);
const Color Color::Snow = float3(1.0f, 0.980392f, 0.980392f);
const Color Color::Black = float3(0.0f, 0.0f, 0.0f);
const Color Color::DimGrey = float3(0.411765f, 0.411765f, 0.411765f);
const Color Color::Grey = float3(0.501961f, 0.501961f, 0.501961f);
const Color Color::DarkGrey = float3(0.662745f, 0.662745f, 0.662745f);
const Color Color::Silver = float3(0.752941f, 0.752941f, 0.752941f);
const Color Color::LightGrey = float3(0.827451f, 0.827451f, 0.827451f);
const Color Color::Gainsboro = float3(0.862745f, 0.862745f, 0.862745f);
const Color Color::WhiteSmoke = float3(0.960784f, 0.960784f, 0.960784f);
const Color Color::White = float3(1.0f, 1.0f, 1.0f);

float3 Color::ToFloat3()
{
    return float3(float(m_r) / 255.0f, float(m_g) / 255.0f, float(m_b) / 255.0f);
}

float4 Color::ToFloat4()
{
    return float4(float(m_r) / 255.0f, float(m_g) / 255.0f, float(m_b) / 255.0f, float(m_a) / 255.0f);
}

float4 Color::ToFloat4WithoutAlpha()
{
    return float4(float(m_r) / 255.0f, float(m_g) / 255.0f, float(m_b) / 255.0f, 1.0f);
}

SEEK_NAMESPACE_END
