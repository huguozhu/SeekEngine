#include "math/math_utility.h"
#include "math/aabbox.h"
#include <cmath>
#include <random>

//#ifndef SEEK_PLATFORM_ANDROID // TODO: not support ndk12
//#include "Eigen/Dense"
//using namespace Eigen;
//#endif

SEEK_NAMESPACE_BEGIN

namespace Math {

float Abs(float x)
{
    union FNI
    {
        float f;
        int32_t i;
    } fni;
    fni.f = x;
    fni.i &= 0x7FFFFFFF;
    return fni.f;
}
// From Quake III. But the magic number is from http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
float RecipSqrt(float number)
{
    float const threehalfs = 1.5f;
    float x2 = number * 0.5f;
    union FNI
    {
        float f;
        int32_t i;
    } fni;
    fni.f = number;
    fni.i = 0x5f375a86 - (fni.i >> 1);
    fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));
    fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));
    return fni.f;
}
float2 Min(float2 const& lhs, float2 const& rhs)
{
    return float2(Min(lhs.x(), rhs.x()), Min(lhs.y(), rhs.y()));
}
float3 Min(float3 const& lhs, float3 const& rhs)
{
    return float3(Min(lhs.x(), rhs.x()), Min(lhs.y(), rhs.y()), Min(lhs.z(), rhs.z()));
}
float2 Max(float2 const& lhs, float2 const& rhs)
{
    return float2(Max(lhs.x(), rhs.x()), Max(lhs.y(), rhs.y()));
}
float3 Max(float3 const& lhs, float3 const& rhs)
{
    return float3(Max(lhs.x(), rhs.x()), Max(lhs.y(), rhs.y()), Max(lhs.z(), rhs.z()));
}
float Dot(float2 const& lhs, float2 const& rhs)
{
    return (lhs[0] * rhs[0] + lhs[1] * rhs[1]);
}
float Dot(float3 const& lhs, float3 const& rhs)
{
    return (lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2]);
}
float Dot(float4 const& lhs, float4 const& rhs)
{
    return (lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + lhs[3] * rhs[3]);
}
float Dot(Quaternion const& lhs, Quaternion const& rhs)
{
    return (lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + lhs[3] * rhs[3]);
}
float LengthSq(float2 const& rhs)
{
    return Dot(rhs, rhs);
}
float LengthSq(float3 const& rhs)
{
    return Dot(rhs, rhs);
}
float LengthSq(float4 const& rhs)
{
    return Dot(rhs, rhs);
}
float LengthSq(Quaternion const& rhs)
{
    return Dot(rhs, rhs);
}
float Length(float2 const& rhs)
{
    float lsq = LengthSq(rhs);
    return sqrt(lsq);
}
float Length(float3 const& rhs)
{
    float lsq = LengthSq(rhs);
    return sqrt(lsq);
}
float Length(float4 const& rhs)
{
    float lsq = LengthSq(rhs);
    return sqrt(lsq);
}
float Length(Quaternion const& rhs)
{
    float4 q = float4(rhs.x(), rhs.y(), rhs.z(), rhs.w());
    return Length(q);
}
float Distance(float const& lhs, float const& rhs)
{
    return rhs - lhs;
}
float Distance(float2 const& lhs, float2 const& rhs)
{
    return sqrt((rhs[0] - lhs[0]) * (rhs[0] - lhs[0]) + (rhs[1] - lhs[1]) * (rhs[1] - lhs[1]));
}
float Distance(float3 const& lhs, float3 const& rhs)
{
    return sqrt((rhs[0] - lhs[0]) * (rhs[0] - lhs[0]) + (rhs[1] - lhs[1]) * (rhs[1] - lhs[1]) + (rhs[2] - lhs[2]) * (rhs[2] - lhs[2]));
}
float2 Normalize(float2 const& rhs)
{
    return rhs * RecipSqrt(LengthSq(rhs));
}
float3 Normalize(float3 const& rhs)
{
    return rhs * RecipSqrt(LengthSq(rhs));
}
float4 Normalize(float4 const& rhs)
{
    return rhs * RecipSqrt(LengthSq(rhs));
}
Quaternion Normalize(Quaternion const& rhs)
{
    return rhs * RecipSqrt(LengthSq(rhs));
}
float Cross(float2 const& lhs, float2 const& rhs)
{
    return lhs[0] * rhs[1] - lhs[1] * rhs[0];
}
float3 Cross(float3 const& lhs, float3 const& rhs)
{
    return float3(
        lhs[1] * rhs[2] - lhs[2] * rhs[1],
        lhs[2] * rhs[0] - lhs[0] * rhs[2],
        lhs[0] * rhs[1] - lhs[1] * rhs[0]);
}


template float  Lerp(float  const& lhs, float  const& rhs, float t);
template float2 Lerp(float2 const& lhs, float2 const& rhs, float t);
template float3 Lerp(float3 const& lhs, float3 const& rhs, float t);
template float4 Lerp(float4 const& lhs, float4 const& rhs, float t);
template <class T>
T Lerp(T const& lhs, T const& rhs, float t)
{
    return lhs + (rhs - lhs) * t;
}
template float  CubicSpline(float  const& v0, float  const& v1, float  const& v2, float  const& v3, float t);
template float2 CubicSpline(float2 const& v0, float2 const& v1, float2 const& v2, float2 const& v3, float t);
template float3 CubicSpline(float3 const& v0, float3 const& v1, float3 const& v2, float3 const& v3, float t);
template float4 CubicSpline(float4 const& v0, float4 const& v1, float4 const& v2, float4 const& v3, float t);
template <class T>
T CubicSpline(T const& v0, T const& v1, T const& v2, T const& v3, float t)
{
    T t2 = t * t;
    T t3 = t2 * t;
    return ((-t3 + t * 3 - t * 3 + 1) * v0 + (t3* 3 - t2 * 6 + 4) * v1
        + (t3 * -3 + t2 * 3 + t * 3 + 1) * v2 + t3 * v3) / 6.0;
}
Quaternion QuatLerp(Quaternion const& lhs, Quaternion const& rhs, float t)
{
    float scale0, scale1;

    // DOT the quats to get the cosine of the angle between them
    float cosom = Dot(lhs, rhs);

    float dir = 1.0f;
    if (cosom < 0)
    {
        dir = -1.0f;
        cosom = -cosom;
    }

    // make sure they are different enough to avoid a divide by 0
    if (cosom < 1.0f - 0.000001)
    {
        // SLERP away
        float const omega = acos(cosom);
        float const isinom = 1.0f / sin(omega);
        scale0 = sin((1.0f - t) * omega) * isinom;
        scale1 = sin(t * omega) * isinom;
    }
    else
    {
        // LERP is good enough at this distance
        scale0 = 1.0f - t;
        scale1 = t;
    }

    // Compute the result
    return lhs * scale0 + rhs * dir * scale1;
}
//--------------------------------------------------------------------------
// Don Hatch's version of sin(x)/x, which is accurate for very small x.
// Returns 1 for x == 0.
//--------------------------------------------------------------------------
float SinxOverX(float x)
{
    if (x * x < Math::FOLAT_EPSILON)
        return 1.0f;
    else
        return std::sin(x) / x;
}
float QuatAngle(Quaternion const& lhs, Quaternion const& rhs)
{
    // Compute the angle between two quaternions,
    // interpreting the quaternions as 4D vectors.
    Quaternion d = lhs - rhs;
    float length_d = Math::LengthSq(d);
    Quaternion s = lhs + rhs;
    float length_s = Math::LengthSq(s);
    return 2.0 * std::atan2(length_d, length_s);
}

// Spherical Interpolation
Quaternion QuatSlerp(Quaternion const& lhs, Quaternion const& rhs, float t)
{
    float a = QuatAngle(lhs, rhs);
    float s = 1 - t;
    Quaternion q = lhs * (SinxOverX(s * a) / SinxOverX(a) * s) +
        rhs * (SinxOverX(t * a) / SinxOverX(a) * t);
    return Normalize(q);
}

static Quaternion Intermediate(Quaternion const& q0, Quaternion const& q1, Quaternion const& q2)
{
    Quaternion q1_inv = q1.Inverse();
    Quaternion c1 = q1_inv * q2;
    Quaternion c2 = q1_inv * q0;
    Quaternion c3 = (c1.Log() + c2.Log()) * -0.25f;
    Quaternion qa = q1 * c3.Exp();
    qa = Math::Normalize(qa);
    return qa;
}
Quaternion Squad(Quaternion const& q1, Quaternion const& qa, Quaternion const& qb, Quaternion const& q2, float t)
{
    Quaternion r1 = Math::QuatSlerp(q1, q2, t);
    Quaternion r2 = Math::QuatSlerp(qa, qb, t);
    return Math::QuatSlerp(r1, r2, 2 * t * (1 - t));
}
// Spherical Cubic Spline Interpolation
Quaternion QuatCubicSpline(Quaternion const& q0, Quaternion const& q1,
    Quaternion const& q2, Quaternion const& q3, float t)
{
    Quaternion qa = Intermediate(q0, q1, q2);
    Quaternion qb = Intermediate(q1, q2, q3);
    return Squad(q1, qa, qb, q2, t);
}

/* *******************************************************************************
 * 2D Transform                                                                  *
 * *******************************************************************************/

float2 Rotate2D(float2 xy, int degree)
{
    // | x_res | = | cos   -sin | * | x |
    // | y_res |   | sin    cos |   | y |
    float radian = degree * PI / 180;
    float cos_ = cos(radian);
    float sin_ = sin(radian);
    return float2(xy[0] * cos_ - xy[1] * sin_,
                  xy[0] * sin_ + xy[1] * cos_);
}

float2 BBoxSizeAfterRotate2D(float2 size, int degree)
{
    float2 res;
    degree = Math::Abs(degree);
    degree = degree % 180;
    if (degree > 90)
        degree = 180 - degree;
    float radian1 = degree * PI / 180;
    float radian2 = PI/2 - radian1;
    res[0] = size[0] * cos(radian1) + size[1] * cos(radian2);
    res[1] = size[0] * sin(radian1) + size[1] * sin(radian2);
    return res;
}

/* *******************************************************************************
 * 3D Transform                                                                  *
 * *******************************************************************************/
Matrix4 Translate(float const& x, float const& y, float const& z)
{
    return Matrix4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1);
}
Matrix4 Translate(float3 const& t)
{
    return Translate(t[0], t[1], t[2]);
}
Matrix4 RotationX(float const& x)
{
    float sx = sin(x);
    float cx = cos(x);
    return Matrix4(
        1, 0, 0, 0,
        0, cx, sx, 0,
        0, -sx, cx, 0,
        0, 0, 0, 1);
}
Matrix4 RotationY(float const& y)
{
    float sy, cy;
    sy = sin(y);
    cy = cos(y);
    return Matrix4(
        cy, 0, -sy, 0,
        0, 1, 0, 0,
        sy, 0, cy, 0,
        0, 0, 0, 1);
}
Matrix4 RotationZ(float const& z)
{
    float sz, cz;
    sz = sin(z);
    cz = cos(z);
    return Matrix4(
        cz, sz, 0, 0,
        -sz, cz, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1);
}
Matrix3 Mat3RotationX(float const& x)
{
    float sx = sin(x);
    float cx = cos(x);
    return Matrix3(
        1, 0, 0,
        0, cx, sx,
        0, -sx, cx);
}
Matrix3 Mat3RotationY(float const& y)
{
    float sy, cy;
    sy = sin(y);
    cy = cos(y);
    return Matrix3(
        cy, 0, -sy,
        0, 1, 0,
        sy, 0, cy);
}
Matrix3 Mat3RotationZ(float const& z)
{
    float sz, cz;
    sz = sin(z);
    cz = cos(z);
    return Matrix3(
        cz, sz, 0,
        -sz, cz, 0,
        0, 0, 1);
}
Matrix4 Rotation(float const& x, float const& y, float const& z)
{
    Matrix4 rot_x = RotationX(x);
    Matrix4 rot_y = RotationY(y);
    Matrix4 rot_z = RotationZ(z);
    return rot_z * rot_x * rot_y;
}
void ToYawPitchRoll(float& yaw, float& pitch, float& roll, Quaternion const& quat)
{
    float x = quat.x();
    float y = quat.y();
    float z = quat.z();
    float w = quat.w();

    float sqx = x * x;
    float sqy = y * y;
    float sqz = z * z;
    float sqw = w * w;

    float unit = sqx + sqy + sqz + sqw;
    float test = w*x + y*z;

    if (test > 0.499f * unit)
    {
        // singularity at north pole
        yaw = 2 * atan2(quat.z(), quat.w());
        pitch = PI / 2;
        roll = 0;
    }
    else
    {
        if (test < -0.499f * unit)
        {
            // singularity at south pole
            yaw = -2 * atan2(quat.z(), quat.w());
            pitch = -PI / 2;
            roll = 0;
        }
        else
        {
            yaw  = atan2(2 * (w*y - x*z), -sqx - sqy + sqz + sqw);
            pitch = asin(2 * test / unit);
            roll = atan2(2 * (w*z - x*y), -sqx + sqy - sqz + sqw);
        }
    }
}
void ToPitchYawRoll(float& pitch, float& yaw, float& roll, Quaternion const& quat)
{
    float x = quat.x();
    float y = quat.y();
    float z = quat.z();
    float w = quat.w();

    float sqx = x * x;
    float sqy = y * y;
    float sqz = z * z;
    float sqw = w * w;

    //float unit = sqx + sqy + sqz + sqw;
    float test = w * y - x * z;

    if (test > 0.499f)
    {
        // singularity at north pole
        pitch = -2 * atan2(quat.z(), quat.w());
        yaw = PI / 2;
        roll = 0;
    }
    else
    {
        if (test < -0.499f)
        {
            // singularity at south pole
            pitch = 2 * atan2(quat.z(), quat.w());
            yaw = -PI / 2;
            roll = 0;
        }
        else
        {
            pitch = atan2(2 * (w * x + y * z), -sqy - sqx + sqz + sqw);
            yaw = asin(2 * test);
            roll = atan2(2 * (w * z + y * x), -sqy + sqx - sqz + sqw);
        }
    }
}
Quaternion FromYawPitchRoll(float yaw, float pitch, float roll)
{
    float angX(pitch / 2), angY(yaw / 2), angZ(roll / 2);
    float sx, sy, sz;
    float cx, cy, cz;
    sx = sin(angX);
    cx = cos(angX);
    sy = sin(angY);
    cy = cos(angY);
    sz = sin(angZ);
    cz = cos(angZ);

    return Quaternion(
        cy * sx * cz - sy * cx * sz,
        cy * sx * sz + sy * cx * cz,
        sy * sx * cz + cy * cx * sz,
        cy * cx * cz - sy * sx * sz);
}
Quaternion FromPitchYawRoll(float pitch, float yaw, float roll)
{
    float angX(pitch / 2), angY(yaw / 2), angZ(roll / 2);
    float sx, sy, sz;
    float cx, cy, cz;
    sx = sin(angX);
    cx = cos(angX);
    sy = sin(angY);
    cy = cos(angY);
    sz = sin(angZ);
    cz = cos(angZ);

    return Quaternion(
        sx * cy * cz - cx * sy * sz,
        sx * cy * sz + cx * sy * cz,
        -sx * sy * cz + cx * cy * sz,
        cx * cy * cz + sx * sy * sz);
}
Quaternion MatrixToQuaternion(Matrix4 const& mat)
{
    Quaternion quat;
    float s;
    float const tr = mat(0, 0) + mat(1, 1) + mat(2, 2) + 1;

    // check the diagonal
    if (tr > 1)
    {
        s = sqrt(tr);
        quat.w() = s * 0.5f;
        s = 0.5f / s;
        quat.x() = (mat(1, 2) - mat(2, 1)) * s;
        quat.y() = (mat(2, 0) - mat(0, 2)) * s;
        quat.z() = (mat(0, 1) - mat(1, 0)) * s;
    }
    else
    {
        int maxi = 0;
        float maxdiag = mat(0, 0);
        for (int i = 1; i < 3; ++i)
        {
            if (mat(i, i) > maxdiag)
            {
                maxi = i;
                maxdiag = mat(i, i);
            }
        }

        switch (maxi)
        {
        case 0:
            s = sqrt((mat(0, 0) - (mat(1, 1) + mat(2, 2))) + 1);
            quat.x() = s * 0.5f;
            if (s != 0)
            {
                s = (float)(0.5 / s);
            }

            quat.w() = (mat(1, 2) - mat(2, 1)) * s;
            quat.y() = (mat(1, 0) + mat(0, 1)) * s;
            quat.z() = (mat(2, 0) + mat(0, 2)) * s;
            break;

        case 1:
            s = sqrt((mat(1, 1) - (mat(2, 2) + mat(0, 0))) + 1);
            quat.y() = (float)(s * 0.5);

            if (s != 0)
            {
                s = (float)(0.5 / s);
            }

            quat.w() = (mat(2, 0) - mat(0, 2)) * s;
            quat.z() = (mat(2, 1) + mat(1, 2)) * s;
            quat.x() = (mat(0, 1) + mat(1, 0)) * s;
            break;

        case 2:
        default:
            s = sqrt((mat(2, 2) - (mat(0, 0) + mat(1, 1))) + 1);

            quat.z() = (float)(s * 0.5);

            if (s != 0)
            {
                s = (float)(0.5 / s);
            }

            quat.w() = (mat(0, 1) - mat(1, 0)) * s;
            quat.x() = (mat(0, 2) + mat(2, 0)) * s;
            quat.y() = (mat(1, 2) + mat(2, 1)) * s;
            break;
        }
    }
    return Normalize(quat);
}
Quaternion TBNToQuaternion(float3 const& tangent, float3 const& binormal, float3 const& normal, int bits)
{
    float k = 1.0;
    if (Dot(binormal, Cross(normal, tangent)) < 0.0)
        k = -1.0;
    Matrix4 mat4_tangent = Matrix4(
        tangent[0], tangent[1], tangent[2], 0.0,
        k * binormal[0], k * binormal[1], k * binormal[2], 0.0,
        normal[0], normal[1], normal[2], 0.0,
        0.0, 0.0, 0.0, 1.0);
    Quaternion tangent_quat = MatrixToQuaternion(mat4_tangent);
    if (tangent_quat.w() < 0.0)
        tangent_quat = -tangent_quat;
    if (bits > 0.0)
    {
        float bias = 1.0f / ((1UL << (bits - 1)) - 1);
        if (tangent_quat.w() < bias)
        {
            float factor = sqrt(1.0f - bias * bias);
            tangent_quat.x() *= factor;
            tangent_quat.y() *= factor;
            tangent_quat.z() *= factor;
            tangent_quat.w() = bias;
        }
    }
    if (k < 0.0)
        tangent_quat = -tangent_quat;
    return tangent_quat;
}
Matrix4 Scale(float x, float y, float z)
{
    return Matrix4(
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    );
}
Matrix4 Scale(float3 const& s)
{
    return Scale(s[0], s[1], s[2]);
}
Matrix4 Transform(float3 pos, Quaternion rot, float3 scale)
{
    Matrix4 m1 = Scale(scale);
    Matrix4 m2 = rot.ToMatrix();
    Matrix4 m3 = Translate(pos);
    return m1 * m2 * m3;
}
void TransformDecompose(float3& scale, Quaternion& rot, float3& trans, Matrix4 const& rhs)
{
    scale[0] = sqrt(rhs(0, 0) * rhs(0, 0) + rhs(0, 1) * rhs(0, 1) + rhs(0, 2) * rhs(0, 2));
    scale[1] = sqrt(rhs(1, 0) * rhs(1, 0) + rhs(1, 1) * rhs(1, 1) + rhs(1, 2) * rhs(1, 2));
    scale[2] = sqrt(rhs(2, 0) * rhs(2, 0) + rhs(2, 1) * rhs(2, 1) + rhs(2, 2) * rhs(2, 2));

    trans = float3(rhs(3, 0), rhs(3, 1), rhs(3, 2));

    Matrix4 rot_mat;
    rot_mat(0, 0) = rhs(0, 0) / scale.x();
    rot_mat(0, 1) = rhs(0, 1) / scale.x();
    rot_mat(0, 2) = rhs(0, 2) / scale.x();
    rot_mat(0, 3) = 0;
    rot_mat(1, 0) = rhs(1, 0) / scale.y();
    rot_mat(1, 1) = rhs(1, 1) / scale.y();
    rot_mat(1, 2) = rhs(1, 2) / scale.y();
    rot_mat(1, 3) = 0;
    rot_mat(2, 0) = rhs(2, 0) / scale.z();
    rot_mat(2, 1) = rhs(2, 1) / scale.z();
    rot_mat(2, 2) = rhs(2, 2) / scale.z();
    rot_mat(2, 3) = 0;
    rot_mat(3, 0) = 0;
    rot_mat(3, 1) = 0;
    rot_mat(3, 2) = 0;
    rot_mat(3, 3) = 1;

    Matrix3 rot_mat3(rot_mat);
    if (rot_mat3.Determinant() < 0) {
        rot_mat(0, 0) = -rot_mat(0, 0);
        rot_mat(0, 1) = -rot_mat(0, 1);
        rot_mat(0, 2) = -rot_mat(0, 2);
        rot_mat(1, 0) = -rot_mat(1, 0);
        rot_mat(1, 1) = -rot_mat(1, 1);
        rot_mat(1, 2) = -rot_mat(1, 2);
        rot_mat(2, 0) = -rot_mat(2, 0);
        rot_mat(2, 1) = -rot_mat(2, 1);
        rot_mat(2, 2) = -rot_mat(2, 2);
        scale[0] = -scale[0];
        scale[1] = -scale[1];
        scale[2] = -scale[2];
    }
    rot = MatrixToQuaternion(rot_mat);
}
Matrix4 TransformEx(float3 const* scale_center, Quaternion const* scaling_rotation,
    float3 const* scale, float3 const* rotation_center, Quaternion const* rotation, float3 const* translate)
{
    float3 psc, prc, pt;
    if (scale_center)
    {
        psc = *scale_center;
    }
    else
    {
        psc = float3(0.0, 0.0, 0.0);
    }
    if (rotation_center)
    {
        prc = *rotation_center;
    }
    else
    {
        prc = float3(0.0, 0.0, 0.0);
    }
    if (translate)
    {
        pt = *translate;
    }
    else
    {
        pt = float3(0.0, 0.0, 0.0);
    }

    Matrix4 m1, m2, m3, m4, m5, m6, m7;
    m1 = Translate(-psc);
    if (scaling_rotation)
    {
        m4 = scaling_rotation->ToMatrix();
        m2 = m4.Inverse();
    }
    else
    {
        m2 = m4 = Matrix4::Identity();
    }
    if (scale)
    {
        m3 = Scale(*scale);
    }
    else
    {
        m3 = Matrix4::Identity();
    }
    if (rotation)
    {
        m6 = rotation->ToMatrix();
    }
    else
    {
        m6 = Matrix4::Identity();
    }
    m5 = Translate(psc - prc);
    m7 = Translate(prc + pt);

    return m1 * m2 * m3 * m4 * m5 * m6 * m7;
}

float3 TransformVector(float3 const& v, Matrix4 const& mat)
{
    float4 v4(v.x(), v.y(), v.z(), 0.0);
    v4 = TransformVector(v4, mat);
    return float3(v4[0], v4[1], v4[2]);
}
float4 TransformVector(float4 const& v, Matrix4 const& mat)
{
    return float4(
        v[0] * mat(0, 0) + v[1] * mat(1, 0) + v[2] * mat(2, 0) + v[3] * mat(3, 0),
        v[0] * mat(0, 1) + v[1] * mat(1, 1) + v[2] * mat(2, 1) + v[3] * mat(3, 1),
        v[0] * mat(0, 2) + v[1] * mat(1, 2) + v[2] * mat(2, 2) + v[3] * mat(3, 2),
        v[0] * mat(0, 3) + v[1] * mat(1, 3) + v[2] * mat(2, 3) + v[3] * mat(3, 3));
}
float3 TransformVectorWithScale(float3 const& v, Matrix4 const& mat)
{
    float4 v4(v.x(), v.y(), v.z(), 1.0);
    v4 = TransformVector(v4, mat);
    float recip = 1.0f / v4[3];
    return float3(v4[0] * recip, v4[1] * recip, v4[2] * recip);
}

float3 TransformQuaternion(float3 const& v, Quaternion const& quat)
{
    return v + Cross(quat.v(), Cross(quat.v(), v) + v * quat.w()) * 2.0f;
}
AABBox TransformAABBox(AABBox const& aabb, Matrix4 const& mat)
{
    float3 min, max;
    float3 tmp = aabb.Corner(0);
    min = max = TransformVectorWithScale(tmp, mat);
    for (uint32_t i = 1; i < 8; i++)
    {
        tmp = aabb.Corner(i);
        float3 vec = TransformVectorWithScale(tmp, mat);
        min = Math::Min(min, vec);
        max = Math::Max(max, vec);
    }
    return AABBox(min, max);
}
AABBox TransformAABBox(AABBox const& aabb, float3 const& scale, Quaternion const& rot, float3 const& trans)
{
    float3 min, max;
    min = max = TransformQuaternion(aabb.Corner(0) * scale, rot) + trans;
    for (uint32_t i = 1; i < 8; i++)
    {
        float3 vec = TransformQuaternion(aabb.Corner(i) * scale, rot) + trans;
        min = Math::Min(min, vec);
        max = Math::Max(max, vec);
    }
    return AABBox(min, max);
}

/* *******************************************************************************
 * camera related                                                                *
 * *******************************************************************************/
Matrix4 LookAtLH(float3 const& eye_pos, float3 const& look_at, float3 const& up)
{
    float3 zAxis = Normalize(look_at - eye_pos);
    float3 xAxis = Normalize(Cross(up, zAxis));
    float3 yAxis = Cross(zAxis, xAxis);
    return Matrix4(
        xAxis[0], yAxis[0], zAxis[0], 0,
        xAxis[1], yAxis[1], zAxis[1], 0,
        xAxis[2], yAxis[2], zAxis[2], 0,
        -Dot(xAxis, eye_pos), -Dot(yAxis, eye_pos), -Dot(zAxis, eye_pos), 1);
}
void ToLookAtLH(Matrix4 const& mat, float3 & eye_pos, float3 & look_at, float3 & up)
{
//#ifndef SEEK_PLATFORM_ANDROID // TODO: not support ndk12
//    float3 xAxis = float3(mat[0], mat[4], mat[8]);
//    float3 yAxis = float3(mat[1], mat[5], mat[9]);
//    float3 zAxis = float3(mat[2], mat[6], mat[10]);
//
//    Matrix3f A;
//    A << xAxis[0], xAxis[1], xAxis[2], yAxis[0], yAxis[1], yAxis[2], zAxis[0], zAxis[1], zAxis[2];
//    Vector3f B;
//    B << -mat[12], -mat[13], -mat[14];
//    Vector3f x = A.colPivHouseholderQr().solve(B);
//
//    up = Cross(zAxis, xAxis);
//    eye_pos = float3(x[0], x[1], x[2]);
//
//    float3 ratio3 = eye_pos / Normalize(eye_pos);
//    float ratio = Max(ratio3[0], ratio3[1]);
//    ratio = Max(ratio, ratio3[2]);
//    if (std::isnan(ratio))
//        ratio = 1.0f;
//    look_at = zAxis * ratio + eye_pos;
//#endif
}
void LookAtRotateX(float3 & eye_pos, float3 const & look_at, float3 const & up, float degree)
{
    float angle = degree * PI / 180.0;
    Matrix4 rot = Rotation(up[0] * angle, up[1] * angle, up[2] * angle);
    eye_pos = TransformVector(eye_pos, rot);
}
void LookAtRotateY(float3 & eye_pos, float3 const & look_at, float3       & up, float degree)
{
    float3 normal = Cross(eye_pos - look_at, up);
    float angle = degree * PI / 180.0;
    Matrix4 rot = Rotation(normal[0] * angle, normal[1] * angle, normal[2] * angle);
    eye_pos = TransformVector(eye_pos, rot);
    up = Normalize(Cross(normal, eye_pos - look_at));
}
Matrix4 PerspectiveLH(float yfov, float aspect, float near, float far)
{
    float h = 1.0f / tan(yfov * 0.5f);
    float w = h / aspect;
    float q = far / (far - near);
    float p = -near * q;
    return Matrix4(
        w, 0, 0, 0,
        0, h, 0, 0,
        0, 0, q, 1,
        0, 0, p, 0);
}
Matrix4 OrthographicLH(float width, float height, float near, float far)
{
    float w_2 = width / 2;
    float h_2 = height / 2;
    return OrthographicLH(-w_2, w_2, -h_2, h_2, near, far);
}
Matrix4 OrthographicLH(float left, float right, float bottom, float top, float near, float far)
{
    float const q = (float)(1.0 / (far - near));
    float const invWidth = (float)(1.0 / (right - left));
    float const invHeight = (float)(1.0 / (top - bottom));

    return Matrix4(
        invWidth + invWidth, 0, 0, 0,
        0, invHeight + invHeight, 0, 0,
        0, 0, q, 0,
        -(left + right) * invWidth, -(top + bottom) * invHeight, -near * q, 1);
}

/* *******************************************************************************
 * Intersect                                                                     *
 * *******************************************************************************/
bool Intersect_Ray_AABBox(Ray const& ray, AABBox const& aabb, float* hit_time)
{
    float t_near = -FLOAT_MAX;
    float t_far = FLOAT_MAX;
    float3 dir = ray.GetDirection();
    float3 orig = ray.GetOrigin();

    for (int i = 0; i < 3; ++i)
    {
        if (dir[i] == 0.0f)
        {
            if ((dir[i] < aabb.Min()[i]) || (dir[i] > aabb.Max()[i]))
            {
                return false;
            }
        }
        else
        {
            float t1 = (aabb.Min()[i] - orig[i]) / dir[i];
            float t2 = (aabb.Max()[i] - orig[i]) / dir[i];
            if (t1 > t2)
            {
                std::swap(t1, t2);
            }
            if (t1 > t_near)
            {
                t_near = t1;
            }
            if (t2 < t_far)
            {
                t_far = t2;
            }

            if (t_near > t_far)
            {
                // box is missed
                return false;
            }
            if (t_far < 0)
            {
                // box is behind ray
                return false;
            }
        }
    }
    if (hit_time)
    {
        if (t_near > 0)
            *hit_time = t_near;
        else
            *hit_time = t_far;
    }
    return true;
}

bool Intersect_AABBox_AABBox(AABBox const& lhs, AABBox const& aabb)
{
    float3 const t = aabb.Center() - lhs.Center();
    float3 const e = aabb.HalfSize() + lhs.HalfSize();
    return (Math::Abs(t.x()) <= e.x()) && (Math::Abs(t.y()) <= e.y()) && (Math::Abs(t.z()) <= e.z());
}

float GenerateRandom(float min, float max)
{
    std::random_device rd; // Non-determinstic seed source
    std::default_random_engine generator{ rd() }; // Create random number generator
    std::uniform_real_distribution<float> distribution(min, max);
    float res = distribution(generator);
    return res;
}

float2 GenerateRandom(const float2 &min, const float2 &max)
{
    std::random_device rd; // Non-determinstic seed source
    std::default_random_engine generator{ rd() }; // Create random number generator

    float2 res;
    for(int i = 0; i < 2; i++)
    {
        std::uniform_real_distribution<float> distribution(min[i], max[i]);
        res[i] = distribution(generator);
    }
    return res;
}

float3 GenerateRandom(const float3 &min, const float3 &max)
{
    std::random_device rd; // Non-determinstic seed source
    std::default_random_engine generator{ rd() }; // Create random number generator

    float3 res;
    for (int i = 0; i < 3; i++)
    {
        std::uniform_real_distribution<float> distribution(min[i], max[i]);
        res[i] = distribution(generator);
    }
    return res;
}

float4 GenerateRandom(const float4 &min, const float4 &max)
{
    std::random_device rd; // Non-determinstic seed source
    std::default_random_engine generator{ rd() }; // Create random number generator

    float4 res;
    for (int i = 0; i < 4; i++)
    {
        std::uniform_real_distribution<float> distribution(min[i], max[i]);
        res[i] = distribution(generator);
    }
    return res;
}

} // namespace Math

SEEK_NAMESPACE_END
