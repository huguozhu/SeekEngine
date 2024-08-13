#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
//#include "Eigen/Dense"
//using namespace Eigen;

SEEK_NAMESPACE_BEGIN

class Matrix3;
// Row-major order
class Matrix4
{
    enum { row_num=4, col_num=4 };
public:
    Matrix4() {};
    Matrix4(float const* rhs);
    Matrix4(Matrix4 const& rhs);
    Matrix4(float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33);
    Matrix4(Matrix3 const& rhs);

    static Matrix4 const& Zero();
    static Matrix4 const& Identity();

    float&          operator()(size_t row, size_t col)          { return m_vMatrix[row][col]; }
    float const&    operator()(size_t row, size_t col) const    { return m_vMatrix[row][col]; }
    float*          begin()                                     { return &m_vMatrix[0][0]; }
    float const*    begin() const                               { return &m_vMatrix[0][0]; }
    float*          end()                                       { return &m_vMatrix[0][0] + row_num * col_num; }
    float const*    end() const                                 { return &m_vMatrix[0][0] + row_num * col_num; }
    float&          operator[](size_t index)                    { return *(this->begin() + index); }
    float const&    operator[](size_t index) const              { return *(this->begin() + index); }

    // set/get Row-Column
    void            Row(size_t index, float4 const& rhs)        { m_vMatrix[index] = rhs; }
    void            Col(size_t index, float4 const& rhs);
    float4 const    Row(size_t index) const                     { return m_vMatrix[index]; }
    float4 const    Col(size_t index) const;

    Matrix4         Transpose()     const;
    Matrix4         Inverse()       const;
    float           Determinant()   const;

    std::string     Str()           const;

    Matrix4& operator+=(Matrix4 const& rhs);
    Matrix4& operator-=(Matrix4 const& rhs);
    Matrix4& operator*=(Matrix4 const& rhs);
    Matrix4& operator*=(float rhs);
    Matrix4& operator/=(float rhs);
    Matrix4& operator= (Matrix4 const& rhs);

    Matrix4 const operator+() const;
    Matrix4 const operator-() const;
    bool operator==(Matrix4 const& rhs) const;
    bool operator!=(Matrix4 const& rhs) const;

    Matrix4 operator+(Matrix4 const& rhs) const;
    Matrix4 operator-(Matrix4 const& rhs) const;
    Matrix4 operator*(Matrix4 const& rhs) const;
    Matrix4 operator*(float const& rhs) const;

private:
    float4 m_vMatrix[4];
};

float4 operator*(float4 lhs, Matrix4 const& rhs);
float4 operator*(Matrix4 const& rhs, float4 lhs);

class Matrix3
{
    enum { row_num = 3, col_num = 3 };
public:
    Matrix3() {};
    Matrix3(float const* rhs);
    Matrix3(Matrix3 const& rhs);
    Matrix3(Matrix4 const& rhs);
    Matrix3(float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m20, float m21, float m22);

    static Matrix3 const& Zero();
    static Matrix3 const& Identity();
    // set/get Row-Column
    void Row(size_t index, float3 const& rhs);
    void Col(size_t index, float3 const& rhs);
    float3 const Row(size_t index) const;
    float3 const Col(size_t index) const;

    float& operator()(size_t row, size_t col)               { return m_vMatrix[row][col]; }
    float const& operator()(size_t row, size_t col) const   { return m_vMatrix[row][col]; }
    float* begin()                                          { return &m_vMatrix[0][0]; }
    float* end()                                            { return &m_vMatrix[0][0] + row_num * col_num; }
    float const* begin() const                              { return &m_vMatrix[0][0]; }
    float const* end() const                                { return &m_vMatrix[0][0] + row_num * col_num; }
    float& operator[](size_t index)                         { return *(this->begin() + index); }
    float const& operator[](size_t index) const             { return *(this->begin() + index); }

    Matrix3 Transpose() const;
    Matrix3 Inverse() const;
    float Determinant() const;

    Matrix3& operator+=(Matrix3 const& rhs);
    Matrix3& operator-=(Matrix3 const& rhs);
    Matrix3& operator*=(Matrix3 const& rhs);
    Matrix3& operator*=(float rhs);
    Matrix3& operator/=(float rhs);
    Matrix3& operator=(Matrix3 const& rhs);

    Matrix3 const operator+() const;
    Matrix3 const operator-() const;
    bool operator==(Matrix3 const& rhs) const;
    bool operator!=(Matrix3 const& rhs) const;

    Matrix3 operator+(Matrix3 const& rhs) const;
    Matrix3 operator-(Matrix3 const& rhs) const;
    Matrix3 operator*(Matrix3 const& rhs) const;
    Matrix3 operator*(float const& rhs) const;

private:
    float3 m_vMatrix[3];
};

float3 operator*(float3 lhs, Matrix3 const& rhs);
float3 operator*(Matrix3 const& rhs, float3 lhs);

using float4x4 = Matrix4;
using float3x3 = Matrix3;

SEEK_NAMESPACE_END
