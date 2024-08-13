#pragma once

#include <array>
#include <cmath>

SEEK_NAMESPACE_BEGIN

template <typename T, size_t N>
class Vector_T
{
public:
    using value_type        = typename std::array<T, N>::value_type;
    using size_type         = size_t;
    using reference         = typename std::array<T, N>::reference;
    using const_reference   = typename std::array<T, N>::const_reference;
    using iterator          = typename std::array<T, N>::iterator;
    using const_iterator    = typename std::array<T, N>::const_iterator;

    enum { elem_num = N };

private:
    template <typename U, size_t M>
    friend class Vector_T;

public:
    Vector_T() 
    {
        for (size_type i = 0; i < N; i++)
            m_vVec[i] = T{};
    }
    Vector_T(T const* x)
    {
        for (size_type i = 0; i < N; i++)
            m_vVec[i] = x[i];
    }
    Vector_T(T const& x)
    {
        for (size_type i = 0; i < N; i++)
            m_vVec[i] = x;
    }
    template <typename U, size_type M>
    Vector_T(Vector_T<U, M> const& rhs)
    {
        static_assert(M >= N, "Could not convert to a smaller vector.");
        for (size_type i = 0; i < N; i++)
            m_vVec[i] = rhs.m_vVec[i];
    }
    Vector_T(T const& x, T const& y)
    {
        static_assert(2 == elem_num, "Must be 2D vector.");
        m_vVec[0] = x;
        m_vVec[1] = y;
    }
    Vector_T(T const& x, T const& y, T const& z)
    {
        static_assert(3 == elem_num, "Must be 3D vector.");
        m_vVec[0] = x;
        m_vVec[1] = y;
        m_vVec[2] = z;
    }
    Vector_T(T const& x, T const& y, T const& z, T const& w)
    {
        static_assert(4 == elem_num, "Must be 4D vector.");
        m_vVec[0] = x;
        m_vVec[1] = y;
        m_vVec[2] = z;
        m_vVec[3] = w;
    }
    static Vector_T const& Zero()
    {
        static Vector_T<T, N> const out(T(0));
        return out;
    }
    static Vector_T const& One()
    {
        static Vector_T<T, N> const out(T(1));
        return out;
    }
    iterator begin()
    {
        return m_vVec.begin();
    }
    const_iterator begin() const
    {
        return m_vVec.begin();
    }
    T* data()
    {
        return m_vVec.data();
    }
    iterator end()
    {
        return m_vVec.end();
    }
    const_iterator end() const
    {
        return m_vVec.end();
    }
    reference operator[](size_t index)
    {
        return m_vVec[index];
    }
    const_reference operator[](size_t index) const
    {
        return m_vVec[index];
    }
    reference x()
    {
        static_assert(elem_num >= 1, "Overflow.");
        return m_vVec[0];
    }
    const_reference x() const
    {
        static_assert(elem_num >= 1, "Overflow.");
        return m_vVec[0];
    }
    reference y()
    {
        static_assert(elem_num >= 2, "Overflow.");
        return m_vVec[1];
    }
    const_reference y() const
    {
        static_assert(elem_num >= 2, "Overflow.");
        return m_vVec[1];
    }
    reference z()
    {
        static_assert(elem_num >= 3, "Overflow.");
        return m_vVec[2];
    }
    const_reference z() const
    {
        static_assert(elem_num >= 3, "Overflow.");
        return m_vVec[2];
    }
    reference w()
    {
        static_assert(elem_num >= 4, "Overflow.");
        return m_vVec[3];
    }
    const_reference w() const
    {
        static_assert(elem_num >= 4, "Overflow.");
        return m_vVec[3];
    }

    template <typename U>
    Vector_T const& operator+=(Vector_T<U, N> const& rhs)
    {
        for (size_type i = 0; i < elem_num; i++)
            m_vVec[i] += rhs[i];
        return *this;
    }
    template <typename U>
    Vector_T const& operator+=(U const& rhs)
    {
        for (size_type i = 0; i < elem_num; i++)
            m_vVec[i] += rhs;
        return *this;
    }

    template <typename U>
    Vector_T const & operator-=(Vector_T<U, N> const rhs)
    {
        for (size_type i = 0; i < elem_num; i++)
            m_vVec[i] -= rhs[i];
        return *this;
    }
    template <typename U>
    Vector_T const& operator-=(U const& rhs)
    {
        for (size_type i = 0; i < elem_num; i++)
            m_vVec[i] -= rhs;
        return *this;
    }

    template <typename U>
    Vector_T const & operator*=(Vector_T<U, N> const rhs)
    {
        for (size_type i = 0; i < elem_num; i++)
            m_vVec[i] *= rhs[i];
        return *this;
    }
    template <typename U>
    Vector_T const& operator*=(U const& rhs)
    {
        for (size_type i = 0; i < elem_num; i++)
            m_vVec[i] *= rhs;
        return *this;
    }

    template <typename U>
    Vector_T const & operator/=(Vector_T<U, N> const rhs)
    {
        for (size_type i = 0; i < elem_num; i++)
            m_vVec[i] /= rhs[i];
        return *this;
    }
    template <typename U>
    Vector_T const& operator/=(U const& rhs)
    {
        for (size_type i = 0; i < elem_num; i++)
            m_vVec[i] /= rhs;
        return *this;
    }

    Vector_T& operator=(Vector_T const& rhs)
    {
        if (this != &rhs)
            m_vVec = rhs.m_vVec;
        return *this;
    }
    template <typename U, size_type M>
    Vector_T& operator=(Vector_T<U, M> const& rhs)
    {
        static_assert(M >= N, "Could not assign to a smaller vector.");
        for (size_type i = 0; i < N; i++)
            m_vVec[i] = rhs[i];
        return *this;
    }

    Vector_T const operator+() const
    {
        return *this;
    }
    Vector_T const operator-() const
    {
        Vector_T out = *this;
        for (size_type i = 0; i < N; i++)
            out[i] = -out[i];
        return out;
    }
    bool operator==(Vector_T const& rhs) const
    {
        for (size_type i = 0; i < N; i++)
        {
            if (m_vVec[i] != rhs[i])
                return false;
        }
        return true;
    }
    bool operator!=(Vector_T const& rhs) const
    {
        return !(this->operator==(rhs));
    }

    template <typename U>
    Vector_T operator+(Vector_T<U, N> const& rhs) const
    {
        Vector_T out;
        for (size_type i = 0; i < elem_num; i++)
            out[i] = m_vVec[i] + rhs[i];
        return out;
    }
    template <typename U>
    Vector_T operator+(U const& rhs) const
    {
        Vector_T out;
        for (size_type i = 0; i < elem_num; i++)
            out[i] = m_vVec[i] + rhs;
        return out;
    }

    template <typename U>
    Vector_T operator-(Vector_T<U, N> const& rhs) const
    {
        Vector_T out;
        for (size_type i = 0; i < elem_num; i++)
            out[i] = m_vVec[i] - rhs[i];
        return out;
    }
    template <typename U>
    Vector_T operator-(U const& rhs) const
    {
        Vector_T out;
        for (size_type i = 0; i < elem_num; i++)
            out[i] = m_vVec[i] - rhs;
        return out;
    }

    template <typename U>
    Vector_T operator*(Vector_T<U, N> const& rhs) const
    {
        Vector_T out;
        for (size_type i = 0; i < elem_num; i++)
            out[i] = m_vVec[i] * rhs[i];
        return out;
    }
    template <typename U>
    Vector_T operator*(U const& rhs) const
    {
        Vector_T out;
        for (size_type i = 0; i < elem_num; i++)
            out[i] = (*this)[i] * rhs;
        return out;
    }

    template <typename U>
    Vector_T operator/(Vector_T<U, N> const& rhs) const
    {
        Vector_T out;
        for (size_type i = 0; i < elem_num; i++)
            out[i] = m_vVec[i]/rhs[i];
        return out;
    }
    template <typename U>
    Vector_T operator/(U const& rhs) const
    {
        Vector_T out;
        for (size_type i = 0; i < elem_num; i++)
            out[i] = m_vVec[i] / rhs;
        return out;
    }

    constexpr size_type size() const
    {
        return m_vVec.size();
    }

    std::string str() const
    {
        std::string res="(";
        for (size_type i = 0; i < elem_num - 1; i++)
            res += std::to_string(m_vVec[i]) + ",";
        res += std::to_string(m_vVec[elem_num-1]) + ")";
        return res;
    }

private:
    std::array<T, N> m_vVec;
};

using int2      = Vector_T<int32_t, 2>;
using int3      = Vector_T<int32_t, 3>;
using int4      = Vector_T<int32_t, 4>;
using uint2     = Vector_T<uint32_t, 2>;
using uint3     = Vector_T<uint32_t, 3>;
using uint4     = Vector_T<uint32_t, 4>;
using float2    = Vector_T<float, 2>;
using float3    = Vector_T<float, 3>;
using float4    = Vector_T<float, 4>;

SEEK_NAMESPACE_END
