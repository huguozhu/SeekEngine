#pragma once

#include "kernel/kernel.h"
#include "math/quaternion.h"
#include "math/aabbox.h"
#include "math/ray.h"

SEEK_NAMESPACE_BEGIN

namespace Math {

constexpr float PI = 3.1415926f;
constexpr float PI2 = 6.2831852f;
constexpr float DEG2RAD = 0.01745329f;
constexpr float RAD2DEG = 57.29577f;

constexpr float FOLAT_EPSILON = 1.192092896e-07f;
constexpr float FLOAT_MAX = 3.402823466e+38f;
constexpr float FLOAT_MIN = 1.175494351e-38f;
constexpr double MIN_EXP_ARG = -700.0;


/* *******************************************************************************
 * float related                                                                 *
 * *******************************************************************************/
float                   Abs(float x);
float                   RecipSqrt(float number);

template<class T> T     Min(T const& lhs, T const& rhs) { return (lhs < rhs ? lhs : rhs); }
float2                  Min(float2 const& lhs, float2 const& rhs);
float3                  Min(float3 const& lhs, float3 const& rhs);

template<class T> T     Max(T const& lhs, T const& rhs) { return (lhs > rhs ? lhs : rhs); }
float2                  Max(float2 const& lhs, float2 const& rhs);
float3                  Max(float3 const& lhs, float3 const& rhs);

template<class T> T     Clamp(T const& val, T const& low, T const& high) { return Max<T>(low, Min<T>(high, val)); }

float                   Dot(float2 const& lhs, float2 const& rhs);
float                   Dot(float3 const& lhs, float3 const& rhs);
float                   Dot(float4 const& lhs, float4 const& rhs);
float                   Dot(Quaternion const& lhs, Quaternion const& rhs);

float                   LengthSq(float2 const& rhs);
float                   LengthSq(float3 const& rhs);
float                   LengthSq(float4 const& rhs);
float                   LengthSq(Quaternion const& rhs);

float                   Length(float2 const& rhs);
float                   Length(float3 const& rhs);
float                   Length(float4 const& rhs);
float                   Length(Quaternion const& rhs);

float                   Distance(float const& lhs, float const& rhs);
float                   Distance(float2 const& lhs, float2 const& rhs);
float                   Distance(float3 const& lhs, float3 const& rhs);

float2                  Normalize(float2 const& rhs);
float3                  Normalize(float3 const& rhs);
float4                  Normalize(float4 const& rhs);
Quaternion              Normalize(Quaternion const& rhs);

float                   Cross(float2 const& lhs, float2 const& rhs);
float3                  Cross(float3 const& lhs, float3 const& rhs);

template <class T> T    Lerp(T const& lhs, T const& rhs, float t);
template <class T> T    CubicSpline(T const& v0, T const& v1, T const& v2, T const& v3, float t);

Quaternion              QuatLerp(Quaternion const& lhs, Quaternion const& rhs, float t);
Quaternion              QuatSlerp(Quaternion const& lhs, Quaternion const& rhs, float t);
Quaternion              QuatCubicSpline(Quaternion const& q0, Quaternion const& q1, Quaternion const& q2, Quaternion const& q3, float t);

/* *******************************************************************************
 * 2D Transform                                                                  *
 * *******************************************************************************/
float2                  Rotate2D(float2 xy, int degree);
float2                  BBoxSizeAfterRotate2D(float2 size, int degree);
/* *******************************************************************************
 * 3D Transform                                                                  *
 * *******************************************************************************/
// Translate
Matrix4                 Translate(float const& x, float const& y, float const& z);
Matrix4                 Translate(float3 const& t);

// Rotate
Matrix4                 RotationX(float const& x);
Matrix4                 RotationY(float const& y);
Matrix4                 RotationZ(float const& z);
Matrix3                 Mat3RotationX(float const& x);
Matrix3                 Mat3RotationY(float const& y);
Matrix3                 Mat3RotationZ(float const& z);
Matrix4                 Rotation(float const& x, float const& y, float const& z);
void                    ToPitchYawRoll(float& pitch, float& yaw, float& roll, Quaternion const& quat);
Quaternion              FromPitchYawRoll(float pitch, float yaw, float roll);
Quaternion              MatrixToQuaternion(Matrix4 const& mat);
Quaternion              TBNToQuaternion(float3 const& tangent, float3 const& binormal, float3 const& normal, int bits);

// Scale
Matrix4                 Scale(float x, float y, float z);
Matrix4                 Scale(float3 const& s);

// Transform
Matrix4                 Transform(float3 pos, Quaternion rot, float3 scale);
void                    TransformDecompose(float3& scale, Quaternion& rot, float3& trans, Matrix4 const& rhs);
Matrix4                 TransformEx(float3 const* scale_center, Quaternion const* scaling_rotation, float3 const* scale,
                                    float3 const* rotation_center, Quaternion const* rotation, float3 const* translate);
float3                  TransformVector(float3 const& v, Matrix4 const& mat);
float4                  TransformVector(float4 const& v, Matrix4 const& mat);
float3                  TransformVectorWithScale(float3 const& v, Matrix4 const& mat);
float3                  TransformQuaternion(float3 const& v, Quaternion const& quat);
AABBox                  TransformAABBox(AABBox const& aabb, Matrix4 const& mat);
AABBox                  TransformAABBox(AABBox const& aabb, float3 const& scale, Quaternion const& rot, float3 const& trans);

/* *******************************************************************************
 * camera related                                                                 *
 * *******************************************************************************/
Matrix4                 LookAtLH(float3 const& eye_pos, float3 const& look_at, float3 const& up);
void                    ToLookAtLH(Matrix4 const& mat, float3 & eye_pos, float3 & look_at, float3 & up);
void                    LookAtRotateX(float3 & eye_pos, float3 const & look_at, float3 const & up, float degree);
void                    LookAtRotateY(float3 & eye_pos, float3 const & look_at, float3       & up, float degree);
Matrix4                 PerspectiveLH(float yfov, float aspect, float near, float far);
Matrix4                 OrthographicLH(float width, float height, float near, float far);
Matrix4                 OrthographicLH(float left, float right, float bottom, float top, float near, float far);

/* *******************************************************************************
 * Intersect                                                                     *
 * *******************************************************************************/
bool                    Intersect_Ray_AABBox(Ray const& ray, AABBox const& aabb, float* hit_time);
bool                    Intersect_AABBox_AABBox(AABBox const& lhs, AABBox const& aabb);

/* *******************************************************************************
 * Random                                                                    *
 * *******************************************************************************/
float                   GenerateRandom(float min, float max);
float2                  GenerateRandom(const float2 &min, const float2 &max);
float3                  GenerateRandom(const float3 &min, const float3 &max);
float4                  GenerateRandom(const float4 &min, const float4 &max);
};

SEEK_NAMESPACE_END
