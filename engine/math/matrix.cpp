#include "math/matrix.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************
 * Matrix4
 ******************************************************************/
Matrix4::Matrix4(float const* rhs)
{
    for (size_t i = 0; i < row_num; i++)
    {
        m_vMatrix[i] = float4(rhs);
        rhs += col_num;
    }
}
Matrix4::Matrix4(Matrix4 const& rhs)
{
    for (size_t i = 0; i < row_num; i++)
    {
        m_vMatrix[i] = rhs.m_vMatrix[i];
    }
}
Matrix4::Matrix4(float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33)
{
    m_vMatrix[0][0] = m00; m_vMatrix[0][1] = m01; m_vMatrix[0][2] = m02; m_vMatrix[0][3] = m03;
    m_vMatrix[1][0] = m10; m_vMatrix[1][1] = m11; m_vMatrix[1][2] = m12; m_vMatrix[1][3] = m13;
    m_vMatrix[2][0] = m20; m_vMatrix[2][1] = m21; m_vMatrix[2][2] = m22; m_vMatrix[2][3] = m23;
    m_vMatrix[3][0] = m30; m_vMatrix[3][1] = m31; m_vMatrix[3][2] = m32; m_vMatrix[3][3] = m33;
}
Matrix4::Matrix4(Matrix3 const& rhs)
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            m_vMatrix[i][j] = rhs(i, j);
    m_vMatrix[0][3] = 0.0f;
    m_vMatrix[1][3] = 0.0f;
    m_vMatrix[2][3] = 0.0f;
    m_vMatrix[3][0] = 0.0f; m_vMatrix[3][1] = 0.0f; m_vMatrix[3][2] = 0.0f; m_vMatrix[3][3] = 1.0f;
}

Matrix4 const& Matrix4::Zero()
{
    static Matrix4 const out(
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0);
    return out;
}

Matrix4 const& Matrix4::Identity()
{
    static Matrix4 const out(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1);
    return out;
}

void Matrix4::Col(size_t index, float4 const& rhs)
{
    for (size_t i = 0; i < row_num; i++)
    {
        m_vMatrix[i][index] = rhs[i];
    }
}

float4 const Matrix4::Col(size_t index) const
{
    float4 out;
    for (size_t i = 0; i < row_num; i++)
    {
        out[i] = m_vMatrix[i][index];
    }
    return out;
}

Matrix4 Matrix4::Transpose() const
{
    return Matrix4(
        m_vMatrix[0][0], m_vMatrix[1][0], m_vMatrix[2][0], m_vMatrix[3][0],
        m_vMatrix[0][1], m_vMatrix[1][1], m_vMatrix[2][1], m_vMatrix[3][1],
        m_vMatrix[0][2], m_vMatrix[1][2], m_vMatrix[2][2], m_vMatrix[3][2],
        m_vMatrix[0][3], m_vMatrix[1][3], m_vMatrix[2][3], m_vMatrix[3][3]);
}

Matrix4 Matrix4::Inverse() const
{
    float const _2132_2231(m_vMatrix[1][0] * m_vMatrix[2][1] - m_vMatrix[1][1] * m_vMatrix[2][0]);
    float const _2133_2331(m_vMatrix[1][0] * m_vMatrix[2][2] - m_vMatrix[1][2] * m_vMatrix[2][0]);
    float const _2134_2431(m_vMatrix[1][0] * m_vMatrix[2][3] - m_vMatrix[1][3] * m_vMatrix[2][0]);
    float const _2142_2241(m_vMatrix[1][0] * m_vMatrix[3][1] - m_vMatrix[1][1] * m_vMatrix[3][0]);
    float const _2143_2341(m_vMatrix[1][0] * m_vMatrix[3][2] - m_vMatrix[1][2] * m_vMatrix[3][0]);
    float const _2144_2441(m_vMatrix[1][0] * m_vMatrix[3][3] - m_vMatrix[1][3] * m_vMatrix[3][0]);
    float const _2233_2332(m_vMatrix[1][1] * m_vMatrix[2][2] - m_vMatrix[1][2] * m_vMatrix[2][1]);
    float const _2234_2432(m_vMatrix[1][1] * m_vMatrix[2][3] - m_vMatrix[1][3] * m_vMatrix[2][1]);
    float const _2243_2342(m_vMatrix[1][1] * m_vMatrix[3][2] - m_vMatrix[1][2] * m_vMatrix[3][1]);
    float const _2244_2442(m_vMatrix[1][1] * m_vMatrix[3][3] - m_vMatrix[1][3] * m_vMatrix[3][1]);
    float const _2334_2433(m_vMatrix[1][2] * m_vMatrix[2][3] - m_vMatrix[1][3] * m_vMatrix[2][2]);
    float const _2344_2443(m_vMatrix[1][2] * m_vMatrix[3][3] - m_vMatrix[1][3] * m_vMatrix[3][2]);
    float const _3142_3241(m_vMatrix[2][0] * m_vMatrix[3][1] - m_vMatrix[2][1] * m_vMatrix[3][0]);
    float const _3143_3341(m_vMatrix[2][0] * m_vMatrix[3][2] - m_vMatrix[2][2] * m_vMatrix[3][0]);
    float const _3144_3441(m_vMatrix[2][0] * m_vMatrix[3][3] - m_vMatrix[2][3] * m_vMatrix[3][0]);
    float const _3243_3342(m_vMatrix[2][1] * m_vMatrix[3][2] - m_vMatrix[2][2] * m_vMatrix[3][1]);
    float const _3244_3442(m_vMatrix[2][1] * m_vMatrix[3][3] - m_vMatrix[2][3] * m_vMatrix[3][1]);
    float const _3344_3443(m_vMatrix[2][2] * m_vMatrix[3][3] - m_vMatrix[2][3] * m_vMatrix[3][2]);
    float det = this->Determinant();
    if (det == 0)
        return *this;
    else
    {
        float invDet = (float(1.0) / det);
        return Matrix4(
            +invDet * (m_vMatrix[1][1] * _3344_3443 - m_vMatrix[1][2] * _3244_3442 + m_vMatrix[1][3] * _3243_3342),
            -invDet * (m_vMatrix[0][1] * _3344_3443 - m_vMatrix[0][2] * _3244_3442 + m_vMatrix[0][3] * _3243_3342),
            +invDet * (m_vMatrix[0][1] * _2344_2443 - m_vMatrix[0][2] * _2244_2442 + m_vMatrix[0][3] * _2243_2342),
            -invDet * (m_vMatrix[0][1] * _2334_2433 - m_vMatrix[0][2] * _2234_2432 + m_vMatrix[0][3] * _2233_2332),

            -invDet * (m_vMatrix[1][0] * _3344_3443 - m_vMatrix[1][2] * _3144_3441 + m_vMatrix[1][3] * _3143_3341),
            +invDet * (m_vMatrix[0][0] * _3344_3443 - m_vMatrix[0][2] * _3144_3441 + m_vMatrix[0][3] * _3143_3341),
            -invDet * (m_vMatrix[0][0] * _2344_2443 - m_vMatrix[0][2] * _2144_2441 + m_vMatrix[0][3] * _2143_2341),
            +invDet * (m_vMatrix[0][0] * _2334_2433 - m_vMatrix[0][2] * _2134_2431 + m_vMatrix[0][3] * _2133_2331),

            +invDet * (m_vMatrix[1][0] * _3244_3442 - m_vMatrix[1][1] * _3144_3441 + m_vMatrix[1][3] * _3142_3241),
            -invDet * (m_vMatrix[0][0] * _3244_3442 - m_vMatrix[0][1] * _3144_3441 + m_vMatrix[0][3] * _3142_3241),
            +invDet * (m_vMatrix[0][0] * _2244_2442 - m_vMatrix[0][1] * _2144_2441 + m_vMatrix[0][3] * _2142_2241),
            -invDet * (m_vMatrix[0][0] * _2234_2432 - m_vMatrix[0][1] * _2134_2431 + m_vMatrix[0][3] * _2132_2231),

            -invDet * (m_vMatrix[1][0] * _3243_3342 - m_vMatrix[1][1] * _3143_3341 + m_vMatrix[1][2] * _3142_3241),
            +invDet * (m_vMatrix[0][0] * _3243_3342 - m_vMatrix[0][1] * _3143_3341 + m_vMatrix[0][2] * _3142_3241),
            -invDet * (m_vMatrix[0][0] * _2243_2342 - m_vMatrix[0][1] * _2143_2341 + m_vMatrix[0][2] * _2142_2241),
            +invDet * (m_vMatrix[0][0] * _2233_2332 - m_vMatrix[0][1] * _2133_2331 + m_vMatrix[0][2] * _2132_2231)
        );
    }
}

float Matrix4::Determinant() const
{
    float const _3142_3241(m_vMatrix[2][0] * m_vMatrix[3][1] - m_vMatrix[2][1] * m_vMatrix[3][0]);
    float const _3143_3341(m_vMatrix[2][0] * m_vMatrix[3][2] - m_vMatrix[2][2] * m_vMatrix[3][0]);
    float const _3144_3441(m_vMatrix[2][0] * m_vMatrix[3][3] - m_vMatrix[2][3] * m_vMatrix[3][0]);
    float const _3243_3342(m_vMatrix[2][1] * m_vMatrix[3][2] - m_vMatrix[2][2] * m_vMatrix[3][1]);
    float const _3244_3442(m_vMatrix[2][1] * m_vMatrix[3][3] - m_vMatrix[2][3] * m_vMatrix[3][1]);
    float const _3344_3443(m_vMatrix[2][2] * m_vMatrix[3][3] - m_vMatrix[2][3] * m_vMatrix[3][2]);

    return m_vMatrix[0][0] * (m_vMatrix[1][1] * _3344_3443 - m_vMatrix[1][2] * _3244_3442 + m_vMatrix[1][3] * _3243_3342)
        - m_vMatrix[0][1] * (m_vMatrix[1][0] * _3344_3443 - m_vMatrix[1][2] * _3144_3441 + m_vMatrix[1][3] * _3143_3341)
        + m_vMatrix[0][2] * (m_vMatrix[1][0] * _3244_3442 - m_vMatrix[1][1] * _3144_3441 + m_vMatrix[1][3] * _3142_3241)
        - m_vMatrix[0][3] * (m_vMatrix[1][0] * _3243_3342 - m_vMatrix[1][1] * _3143_3341 + m_vMatrix[1][2] * _3142_3241);
}

std::string Matrix4::Str() const
{
    float const* b = this->begin();

    std::string s = "Matrix4\n";
    s += "[" + std::to_string(b[0])  + "\t," + std::to_string(b[1])  + "\t," + std::to_string(b[2])  + "\t," + std::to_string(b[3])  + "\t]\n";
    s += "[" + std::to_string(b[4])  + "\t," + std::to_string(b[5])  + "\t," + std::to_string(b[6])  + "\t," + std::to_string(b[7])  + "\t]\n";
    s += "[" + std::to_string(b[8])  + "\t," + std::to_string(b[9])  + "\t," + std::to_string(b[10]) + "\t," + std::to_string(b[11]) + "\t]\n";
    s += "[" + std::to_string(b[12]) + "\t," + std::to_string(b[13]) + "\t," + std::to_string(b[14]) + "\t," + std::to_string(b[15]) + "\t]";
    return s;
}

Matrix4& Matrix4::operator+=(Matrix4 const& rhs)
{
    for (int i=0; i<row_num; i++)
        m_vMatrix[i] += rhs.m_vMatrix[i];
    return *this;
}
Matrix4& Matrix4::operator-=(Matrix4 const& rhs)
{
    for (int i = 0; i < row_num; i++)
        m_vMatrix[i] -= rhs.m_vMatrix[i];
    return *this;
}
Matrix4& Matrix4::operator*=(Matrix4 const& rhs)
{
    *this = *this * rhs;
    return *this;
}
Matrix4& Matrix4::operator*=(float rhs)
{
    for (int i = 0; i < row_num; i++)
    {
        m_vMatrix[i] *= rhs;
    }
    return *this;
}
Matrix4& Matrix4::operator/=(float rhs)
{
    for (int i = 0; i < row_num; i++)
    {
        m_vMatrix[i] /= rhs;
    }
    return *this;
}
Matrix4& Matrix4::operator=(Matrix4 const& rhs)
{
    for (int i = 0; i < row_num; i++)
    {
        m_vMatrix[i] = rhs.Row(i);
    }
    return *this;
}

Matrix4 const Matrix4::operator+() const
{
    return *this;
}
Matrix4 const Matrix4::operator-() const
{
    Matrix4 tmp;
    for (int i = 0; i < row_num; i++)
        tmp.m_vMatrix[i] = -m_vMatrix[i];
    return tmp;
}
bool Matrix4::operator==(Matrix4 const& rhs) const
{
    bool b = true;
    for (int i = 0; i < row_num; i++)
    {
        b = b && (m_vMatrix[i] == rhs.m_vMatrix[i]);
    }
    return b;
}
bool Matrix4::operator!=(Matrix4 const& rhs) const
{
    return !(this->operator==(rhs));
}

Matrix4 Matrix4::operator+(Matrix4 const& rhs) const
{
    Matrix4 out;
    for (int i = 0; i < row_num; i++)
    {
        out.Row(i, this->Row(i) + rhs.Row(i));
    }
    return out;
}
Matrix4 Matrix4::operator-(Matrix4 const& rhs) const
{
    Matrix4 out;
    for (int i = 0; i < row_num; i++)
    {
        out.Row(i, this->Row(i) - rhs.Row(i));
    }
    return out;
}
Matrix4 Matrix4::operator*(Matrix4 const& rhs) const
{
    float const* lhs_data = begin();
    float const* rhs_data = rhs.begin();

    return Matrix4(
        lhs_data[0] * rhs_data[0] + lhs_data[1] * rhs_data[4] + lhs_data[2] * rhs_data[8] + lhs_data[3] * rhs_data[12],
        lhs_data[0] * rhs_data[1] + lhs_data[1] * rhs_data[5] + lhs_data[2] * rhs_data[9] + lhs_data[3] * rhs_data[13],
        lhs_data[0] * rhs_data[2] + lhs_data[1] * rhs_data[6] + lhs_data[2] * rhs_data[10] + lhs_data[3] * rhs_data[14],
        lhs_data[0] * rhs_data[3] + lhs_data[1] * rhs_data[7] + lhs_data[2] * rhs_data[11] + lhs_data[3] * rhs_data[15],

        lhs_data[4] * rhs_data[0] + lhs_data[5] * rhs_data[4] + lhs_data[6] * rhs_data[8] + lhs_data[7] * rhs_data[12],
        lhs_data[4] * rhs_data[1] + lhs_data[5] * rhs_data[5] + lhs_data[6] * rhs_data[9] + lhs_data[7] * rhs_data[13],
        lhs_data[4] * rhs_data[2] + lhs_data[5] * rhs_data[6] + lhs_data[6] * rhs_data[10] + lhs_data[7] * rhs_data[14],
        lhs_data[4] * rhs_data[3] + lhs_data[5] * rhs_data[7] + lhs_data[6] * rhs_data[11] + lhs_data[7] * rhs_data[15],

        lhs_data[8] * rhs_data[0] + lhs_data[9] * rhs_data[4] + lhs_data[10] * rhs_data[8] + lhs_data[11] * rhs_data[12],
        lhs_data[8] * rhs_data[1] + lhs_data[9] * rhs_data[5] + lhs_data[10] * rhs_data[9] + lhs_data[11] * rhs_data[13],
        lhs_data[8] * rhs_data[2] + lhs_data[9] * rhs_data[6] + lhs_data[10] * rhs_data[10] + lhs_data[11] * rhs_data[14],
        lhs_data[8] * rhs_data[3] + lhs_data[9] * rhs_data[7] + lhs_data[10] * rhs_data[11] + lhs_data[11] * rhs_data[15],

        lhs_data[12] * rhs_data[0] + lhs_data[13] * rhs_data[4] + lhs_data[14] * rhs_data[8] + lhs_data[15] * rhs_data[12],
        lhs_data[12] * rhs_data[1] + lhs_data[13] * rhs_data[5] + lhs_data[14] * rhs_data[9] + lhs_data[15] * rhs_data[13],
        lhs_data[12] * rhs_data[2] + lhs_data[13] * rhs_data[6] + lhs_data[14] * rhs_data[10] + lhs_data[15] * rhs_data[14],
        lhs_data[12] * rhs_data[3] + lhs_data[13] * rhs_data[7] + lhs_data[14] * rhs_data[11] + lhs_data[15] * rhs_data[15]
    );
    return *this;
}

Matrix4 Matrix4::operator*(float const& rhs) const
{
    Matrix4 out;
    for (int i = 0; i < row_num; i++)
    {
        out.Row(i, m_vMatrix[i]*rhs);
    }
    return out;
}
float4 operator*(float4 lhs, Matrix4 const& rhs)
{
    float const* rhs_data = rhs.begin();
    return float4(
        lhs[0] * rhs_data[0] + lhs[1] * rhs_data[4] + lhs[2] * rhs_data[8]  + lhs[3] * rhs_data[12],
        lhs[0] * rhs_data[1] + lhs[1] * rhs_data[5] + lhs[2] * rhs_data[9]  + lhs[3] * rhs_data[13],
        lhs[0] * rhs_data[2] + lhs[1] * rhs_data[6] + lhs[2] * rhs_data[10] + lhs[3] * rhs_data[14],
        lhs[0] * rhs_data[3] + lhs[1] * rhs_data[7] + lhs[2] * rhs_data[11] + lhs[3] * rhs_data[15]
    );
}
float4 operator*(Matrix4 const& lhs, float4 rhs)
{
    float const* lhs_data = lhs.begin();
    return float4(
        lhs_data[0]  * rhs[0] + lhs_data[1]  * rhs[1] + lhs_data[2]  * rhs[2] + lhs_data[3]  * rhs[3],
        lhs_data[4]  * rhs[0] + lhs_data[5]  * rhs[1] + lhs_data[6]  * rhs[2] + lhs_data[7]  * rhs[3],
        lhs_data[8]  * rhs[0] + lhs_data[9]  * rhs[1] + lhs_data[10] * rhs[2] + lhs_data[11] * rhs[3],
        lhs_data[12] * rhs[0] + lhs_data[13] * rhs[1] + lhs_data[14] * rhs[2] + lhs_data[15] * rhs[3]
    );
}
float3 operator*(float3 lhs, Matrix3 const& rhs)
{
    float const* rhs_data = rhs.begin();
    return float3(
        lhs[0] * rhs_data[0] + lhs[1] * rhs_data[3] + lhs[2] * rhs_data[6],
        lhs[0] * rhs_data[1] + lhs[1] * rhs_data[4] + lhs[2] * rhs_data[7],
        lhs[0] * rhs_data[2] + lhs[1] * rhs_data[5] + lhs[2] * rhs_data[8]
    );
}
float3 operator*(Matrix3 const& lhs, float3 rhs)
{
    float const* lhs_data = lhs.begin();
    return float3(
        lhs_data[0] * rhs[0] + lhs_data[1] * rhs[1] + lhs_data[2] * rhs[2],
        lhs_data[3] * rhs[0] + lhs_data[4] * rhs[1] + lhs_data[5] * rhs[2],
        lhs_data[6] * rhs[0] + lhs_data[7] * rhs[1] + lhs_data[8] * rhs[2]
    );
}
/******************************************************************
 * Matrix3
 ******************************************************************/
Matrix3::Matrix3(float const* rhs)
{
    for (size_t i = 0; i < 3; i++)
    {
        m_vMatrix[i] = float3(rhs);
        rhs += 3;
    }
}
Matrix3::Matrix3(Matrix3 const& rhs)
{
    for (size_t i = 0; i < row_num; i++)
    {
        m_vMatrix[i] = rhs.m_vMatrix[i];
    }
}
Matrix3::Matrix3(Matrix4 const& rhs)
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            m_vMatrix[i][j] = rhs(i, j);
}
Matrix3::Matrix3(float m00, float m01, float m02,
    float m10, float m11, float m12,
    float m20, float m21, float m22)
{
    m_vMatrix[0][0] = m00;  m_vMatrix[0][1] = m01;  m_vMatrix[0][2] = m02;
    m_vMatrix[1][0] = m10;  m_vMatrix[1][1] = m11;  m_vMatrix[1][2] = m12;
    m_vMatrix[2][0] = m20;  m_vMatrix[2][1] = m21;  m_vMatrix[2][2] = m22;
}
Matrix3 const& Matrix3::Zero()
{
    static Matrix3 const out(
        0, 0, 0,
        0, 0, 0,
        0, 0, 0);
    return out;
}

Matrix3 const& Matrix3::Identity()
{
    static Matrix3 const out(
        1, 0, 0,
        0, 1, 0,
        0, 0, 1);
    return out;
}
void Matrix3::Row(size_t index, float3 const& rhs)
{
    m_vMatrix[index] = rhs;
}

float3 const Matrix3::Row(size_t index) const
{
    return m_vMatrix[index];
}

void Matrix3::Col(size_t index, float3 const& rhs)
{
    for (size_t i = 0; i < row_num; i++)
    {
        m_vMatrix[i][index] = rhs[i];
    }
}

float3 const Matrix3::Col(size_t index) const
{
    float3 out;
    for (size_t i = 0; i < row_num; i++)
    {
        out[i] = m_vMatrix[i][index];
    }
    return out;
}

Matrix3 Matrix3::Transpose() const
{
    return Matrix3(
        m_vMatrix[0][0], m_vMatrix[1][0], m_vMatrix[2][0],
        m_vMatrix[0][1], m_vMatrix[1][1], m_vMatrix[2][1],
        m_vMatrix[0][2], m_vMatrix[1][2], m_vMatrix[2][2]);
}

Matrix3 Matrix3::Inverse() const
{
    Matrix3 inv = Matrix3::Identity();
    float det = Determinant();
    if (det != 0.0)
    {
        float mDet = (float)1.0 / det;
        inv.m_vMatrix[0][0] = (m_vMatrix[1][1] * m_vMatrix[2][2] - m_vMatrix[1][2] * m_vMatrix[2][1]) * mDet;
        inv.m_vMatrix[0][1] = (m_vMatrix[0][2] * m_vMatrix[2][1] - m_vMatrix[0][1] * m_vMatrix[2][2]) * mDet;
        inv.m_vMatrix[0][2] = (m_vMatrix[0][1] * m_vMatrix[1][2] - m_vMatrix[0][2] * m_vMatrix[1][1]) * mDet;

        inv.m_vMatrix[1][0] = (m_vMatrix[1][2] * m_vMatrix[2][0] - m_vMatrix[1][0] * m_vMatrix[2][2]) * mDet;
        inv.m_vMatrix[1][1] = (m_vMatrix[0][0] * m_vMatrix[2][2] - m_vMatrix[0][2] * m_vMatrix[2][0]) * mDet;
        inv.m_vMatrix[1][2] = (m_vMatrix[0][2] * m_vMatrix[1][0] - m_vMatrix[0][0] * m_vMatrix[1][2]) * mDet;

        inv.m_vMatrix[2][0] = (m_vMatrix[1][0] * m_vMatrix[2][1] - m_vMatrix[1][1] * m_vMatrix[2][0]) * mDet;
        inv.m_vMatrix[2][1] = (m_vMatrix[0][1] * m_vMatrix[2][0] - m_vMatrix[0][0] * m_vMatrix[2][1]) * mDet;
        inv.m_vMatrix[2][2] = (m_vMatrix[0][0] * m_vMatrix[1][1] - m_vMatrix[0][1] * m_vMatrix[1][0]) * mDet;
    }
    return inv;
}

float Matrix3::Determinant() const
{
    float det =
        m_vMatrix[0][0] * (m_vMatrix[1][1] * m_vMatrix[2][2] -m_vMatrix[1][2] * m_vMatrix[2][1]) -
        m_vMatrix[0][1] * (m_vMatrix[1][0] * m_vMatrix[2][2] -m_vMatrix[1][2] * m_vMatrix[2][0]) +
        m_vMatrix[0][2] * (m_vMatrix[1][0] * m_vMatrix[2][1] -m_vMatrix[1][1] * m_vMatrix[2][0]);
    return det;
}

Matrix3& Matrix3::operator+=(Matrix3 const& rhs)
{
    for (int i = 0; i < row_num; i++)
        m_vMatrix[i] += rhs.m_vMatrix[i];
    return *this;
}
Matrix3& Matrix3::operator-=(Matrix3 const& rhs)
{
    for (int i = 0; i < row_num; i++)
        m_vMatrix[i] -= rhs.m_vMatrix[i];
    return *this;
}
Matrix3& Matrix3::operator*=(Matrix3 const& rhs)
{
    *this = *this * rhs;
    return *this;
}
Matrix3& Matrix3::operator*=(float rhs)
{
    for (int i = 0; i < col_num; i++)
    {
        m_vMatrix[i] *= rhs;
    }
    return *this;
}
Matrix3& Matrix3::operator/=(float rhs)
{
    for (int i = 0; i < col_num; i++)
    {
        m_vMatrix[i] /= rhs;
    }
    return *this;
}
Matrix3& Matrix3::operator=(Matrix3 const& rhs)
{
    for (int i = 0; i < col_num; i++)
    {
        m_vMatrix[i] = rhs.Row(i);
    }
    return *this;
}

Matrix3 const Matrix3::operator+() const
{
    return *this;
}
Matrix3 const Matrix3::operator-() const
{
    Matrix3 tmp;
    for (int i = 0; i < row_num; i++)
        tmp.m_vMatrix[i] = -m_vMatrix[i];
    return tmp;
}
bool Matrix3::operator==(Matrix3 const& rhs) const
{
    bool b = true;
    for (int i = 0; i < row_num; i++)
    {
        b = b && m_vMatrix[i] == rhs.m_vMatrix[i];
    }
    return b;
}
bool Matrix3::operator!=(Matrix3 const& rhs) const
{
    return !(this->operator==(rhs));
}

Matrix3 Matrix3::operator+(Matrix3 const& rhs) const
{
    Matrix3 out;
    for (int i = 0; i < col_num; i++)
    {
        out.Row(i, this->Row(i) + rhs.Row(i));
    }
    return out;
}
Matrix3 Matrix3::operator-(Matrix3 const& rhs) const
{
    Matrix3 out;
    for (int i = 0; i < col_num; i++)
    {
        out.Row(i, this->Row(i) - rhs.Row(i));
    }
    return out;
}
Matrix3 Matrix3::operator*(Matrix3 const& rhs) const
{
    Matrix3 const tmp = rhs.Transpose();

    return Matrix3(
        m_vMatrix[0][0] * tmp(0, 0) + m_vMatrix[0][1] * tmp(0, 1) + m_vMatrix[0][2] * tmp(0, 2),
        m_vMatrix[0][0] * tmp(1, 0) + m_vMatrix[0][1] * tmp(1, 1) + m_vMatrix[0][2] * tmp(1, 2),
        m_vMatrix[0][0] * tmp(2, 0) + m_vMatrix[0][1] * tmp(2, 1) + m_vMatrix[0][2] * tmp(2, 2),

        m_vMatrix[1][0] * tmp(0, 0) + m_vMatrix[1][1] * tmp(0, 1) + m_vMatrix[1][2] * tmp(0, 2),
        m_vMatrix[1][0] * tmp(1, 0) + m_vMatrix[1][1] * tmp(1, 1) + m_vMatrix[1][2] * tmp(1, 2),
        m_vMatrix[1][0] * tmp(2, 0) + m_vMatrix[1][1] * tmp(2, 1) + m_vMatrix[1][2] * tmp(2, 2),

        m_vMatrix[2][0] * tmp(0, 0) + m_vMatrix[2][1] * tmp(0, 1) + m_vMatrix[2][2] * tmp(0, 2),
        m_vMatrix[2][0] * tmp(1, 0) + m_vMatrix[2][1] * tmp(1, 1) + m_vMatrix[2][2] * tmp(1, 2),
        m_vMatrix[2][0] * tmp(2, 0) + m_vMatrix[2][1] * tmp(2, 1) + m_vMatrix[2][2] * tmp(2, 2));
    return *this;
}
Matrix3 Matrix3::operator*(float const& rhs) const
{
    Matrix3 out;
    for (int i = 0; i < col_num; i++)
    {
        out.Row(i, this->Row(i) * rhs);
    }
    return out;

}

SEEK_NAMESPACE_END
