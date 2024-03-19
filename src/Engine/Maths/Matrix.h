#pragma once

#include <stdint.h>
#include <initializer_list>
#include <cmath>

#define PI 3.14159265359
#define EPS 0.0000001

inline const uint8_t X = 0;
inline const uint8_t Y = 1;
inline const uint8_t Z = 2;
inline const uint8_t W = 3;

namespace Engine::Maths
{
    template<uint8_t n, uint8_t m, typename T> 
    struct MatrixT;

    template<uint8_t n, uint8_t m>
    using MatrixNM = MatrixT<n, m, float>;

    template<uint8_t n>
    using Matrix = MatrixNM<n, n>;

    using Matrix2 = Matrix<2>;
    using Matrix3 = Matrix<3>;
    using Matrix4 = Matrix<4>;

    template<uint8_t n, typename T>
    using VectorT = MatrixT<n, 1, T>;

    template<uint8_t n>
    using Vector = VectorT<n, float>;
    using Vector2 = Vector<2>;
    using Vector3 = Vector<3>;
    using Vector4 = Vector<4>;

    // Saved in column form
    template<uint8_t n, uint8_t m, typename T> 
    struct MatrixT
    {
        public:
            MatrixT<n, m, T>(T const values[n][m]);
            MatrixT<n, m, T>(T const values[n * m]) { for(int i = 0; i < n; i++) { for(int j = 0; j < m; j++) { data[i * m + j] = values[i * m + j]; } } };
            MatrixT<n, m, T>(std::initializer_list<T> values);
            MatrixT<n, m, T>() : data() {}

            inline bool operator==(MatrixT<n, m, T> const & other) const;
            inline bool operator!=(MatrixT<n, m, T> const & other) const {return !(*this == other); };

            inline MatrixT<n, m, T> operator+(MatrixT<n, m, T> const &other) const;
            inline MatrixT<n, m, T> operator-(MatrixT<n, m, T> const &other) const;
            inline MatrixT<n, m, T> operator+(T value) const;
            inline MatrixT<n, m, T> operator-(T value) const;
            inline friend MatrixT<n, m, T> operator+(T value, MatrixT<n, m, T> const &matrix) { return matrix + value; }
            inline MatrixT<n, m, T> & operator+=(MatrixT<n, m, T> const &other);
            inline MatrixT<n, m, T> & operator+=(T value);
            inline MatrixT<n, m, T> & operator-=(MatrixT<n, m, T> const &other);
            inline MatrixT<n, m, T> & operator-=(T value);
            template<uint8_t l>
            inline MatrixT<n, l, T> operator*(MatrixT<m, l, T> const &other) const;
            inline MatrixT<n, m, T> operator*(T value) const;
            inline MatrixT<n, m, T> operator/(T value) const;
            inline friend MatrixT<n, m, T> operator*(T value, MatrixT<n, m, T> const &matrix) { return matrix * value; }
            inline MatrixT<n, m, T> & operator*=(MatrixT<n, m, T> const &other);
            inline MatrixT<n, m, T> & operator*=(T value);
            inline MatrixT<n, m, T> & operator/=(T value);
            inline MatrixT<m, n, T> Transposed() const;
            inline MatrixT<n, n, T> Inverse() const requires(m == n) { return Matrix<n, n, T>(data).Invert(); }
            MatrixT<n, n, T>& Invert() requires(m == n);
            
            inline static MatrixT<n, n, T> Identity() requires(m == n);
            inline static MatrixT<n, m, T> Zero();

            // Transform matrices
            // TODO: Probably refactor
            static MatrixT<4, 4, T> LookAt(VectorT<3,T> const & eye, VectorT<3, T> const & target, VectorT<3, T> const & up) requires(m == 4 && n == 4);
            static MatrixT<3, 3, T> RodriguesRotation(VectorT<3, T> const & axis, T theta) requires(m == 3 && n == 3);
            static MatrixT<4, 4, T> Rotate(VectorT<3, T> const & axis, T theta) requires (m == 4 && n == 4);    // Rotate around origin
            static MatrixT<4, 4, T> Rotate(VectorT<3, T> const & axis, VectorT<3, T> const & center, T theta) requires (m == 4 && n == 4);    // Rotate around center
            static MatrixT<4, 4, T> Perspective(T near, T far, T fov, T aspectRatio) requires(m == 4 && n == 4);

            // +--------------------------------+
            // |    Vector-specific operations  |
            // +--------------------------------+
            inline T operator*(VectorT<n, T> const& other) const requires(m == 1) { return (other.Transposed() * *this)[X]; }
            inline T SqrMagnitude() const requires(m == 1) { return *this * *this; }
            inline T Length() const requires(m == 1) { return std::sqrt(SqrMagnitude()); }
            inline T &operator[](uint8_t i) requires(m == 1) { return data[i]; }
            inline T const &operator[](uint8_t i) const requires(m == 1) { return data[i]; }
            inline VectorT<n, T> Normalized() const requires(m == 1) { return *this / this->Length(); }
            inline VectorT<n, T> & Normalize() requires(m == 1) { return (*this /= this->Length); }
            inline VectorT<3, T> Cross(VectorT<3, T> const& other) const requires(m == 1 && n == 3);

            // "Properties" for easier access
        private:
            class Column {
                    MatrixT<n, m, T> &parent;
                    uint8_t column;
                public:
                    Column(MatrixT<n, m, T> &mat, uint8_t col) : parent(mat), column(col) { }
                    inline VectorT<m, T> operator= (VectorT<m, T> const & values) { for (uint8_t j = 0; j < m; j++) { parent.data[column * m + j] = values[j]; } }
                    inline T & operator[](uint8_t j) { return parent.data[column * m + j]; }
            };

            class Entry {
                    MatrixT<n, 1, T> &parent;
                    uint8_t index;
                public:
                    Entry(MatrixT<n, 1, T> &v, uint8_t i) : parent(v), index(i) { }
                    inline T & operator=(T value) { return parent.data[index] = value; }
                    inline T & operator+=(T value) { return parent.data[index] += value; }
                    inline T & operator-=(T value) { return parent.data[index] -= value; }
                    inline T & operator*=(T value) { return parent.data[index] *= value; }
                    inline T & operator/=(T value) { return parent.data[index] /= value; }
                    inline operator T () const { return parent.data[index]; }
                    inline T operator+(T value) const { return parent.data[index] + value; }
                    inline T operator-(T value) const { return parent.data[index] - value; }
                    inline T operator*(T value) const { return parent.data[index] * value; }
                    inline T operator/(T value) const { return parent.data[index] / value; }
            };

            class EntryPair {
                    MatrixT<n, 1, T> &parent;
                    uint8_t index1;
                    uint8_t index2;
                public:
                    EntryPair(MatrixT<n, 1, T> &v, uint8_t i1, uint8_t i2) : parent(v), index1(i1), index2(i2) { }
                    inline void operator=(MatrixT<2, 1, T> const & values) { parent.data[index1] = values[X]; parent.data[index2] = values[Y]; }
                    inline void operator+=(MatrixT<2, 1, T> const & values) { parent.data[index1] += values[X]; parent.data[index2] += values[Y]; }
                    inline void operator+=(T value) { parent.data[index1] += value; parent.data[index2] += value; }
                    inline void operator-=(MatrixT<2, 1, T> const & values) { parent.data[index1] -= values[X]; parent.data[index2] -= values[Y]; }
                    inline void operator-=(T value) { parent.data[index1] += value; parent.data[index2] -= value; }
                    inline void operator*=(T value) { parent.data[index1] *= value; parent.data[index2] *= value; }
                    inline void operator/=(T value) { parent.data[index1] /= value; parent.data[index2] /= value; }
                    inline operator MatrixT<2, 1, T> () const { return MatrixT<2, 1, T>({ parent.data[index1], parent.data[index2] }); }
            };

            class EntryTriplet {
                    MatrixT<n, 1, T> &parent;
                    uint8_t index1;
                    uint8_t index2;
                    uint8_t index3;
                public:
                    EntryTriplet(MatrixT<n, 1, T> &v, uint8_t i1, uint8_t i2, uint8_t i3) : parent(v), index1(i1), index2(i2), index3(i3) { }
                    inline void operator=(MatrixT<3, 1, T> const & values) { parent.data[index1] = values[X]; parent.data[index2] = values[Y]; parent.data[index3] = values[Z]; }
                    inline void operator+=(MatrixT<3, 1, T> const & values) { parent.data[index1] += values[X]; parent.data[index2] += values[Y]; parent.data[index3] += values[Z]; }
                    inline void operator+=(T value) { parent.data[index1] += value; parent.data[index2] += value; parent.data[index3] += value; }
                    inline void operator-=(MatrixT<3, 1, T> const & values) { parent.data[index1] -= values[X]; parent.data[index2] -= values[Y]; parent.data[index3] -= values[Z]; }
                    inline void operator-=(T value) { parent.data[index1] += value; parent.data[index2] -= value; parent.data[index3] -= value; }
                    inline void operator*=(T value) { parent.data[index1] *= value; parent.data[index2] *= value; parent.data[index3] *= value; }
                    inline void operator/=(T value) { parent.data[index1] /= value; parent.data[index2] /= value; parent.data[index3] /= value; }
                    inline operator MatrixT<3, 1, T> () const { return MatrixT<3, 1, T>(parent.data[index1], parent.data[index2], parent.data[index3]); }
            };

        public:
            inline Column operator[](uint8_t i) { return Column(*this, i); };
            inline MatrixT<m, 1, T> operator[](uint8_t i) const;
            
            Entry x() requires(m == 1) { return Entry(*this, X); }
            Entry y() requires(m == 1 && n >= 2) { return Entry(*this, Y); }
            Entry z() requires(m == 1 && n >= 3) { return Entry(*this, Z); }
            Entry w() requires(m == 1 && n >= 4) { return Entry(*this, W); }
            
            const Entry x() const requires(m == 1) { return Entry(*this, X); }
            const Entry y() const requires(m == 1 && n >= 2) { return Entry(*this, Y); }
            const Entry z() const requires(m == 1 && n >= 3) { return Entry(*this, Z); }
            const Entry w() const requires(m == 1 && n >= 4) { return Entry(*this, W); }

            EntryPair xy() requires(m == 1 && n >= 2) { return EntryPair(*this, X, Y); }
            EntryPair xz() requires(m == 1 && n >= 3) { return EntryPair(*this, X, Z); }
            EntryPair yz() requires(m == 1 && n >= 3) { return EntryPair(*this, Y, Z); }

            EntryTriplet xyz() requires(m == 1 && n >= 3) { return EntryTriplet(*this, X, Y, Z); }

        private:
            // Adds factor * row1 to row2
            inline void RowOp(uint8_t row1, uint8_t row2, T factor);
            inline void ColOp(uint8_t col1, uint8_t col2, T factor);
            inline void ColSwap(uint8_t col1, uint8_t col2);
            T data[n * m];

            // Needed for access of private members of other instance in matrix multiplication
            template<uint8_t n, uint8_t l, typename T>
            friend class MatrixT;
    };

    // +------------------------+
    // |    IMPLEMENTATIONS     |
    // +------------------------+

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, n, T>& MatrixT<n, m, T>::Invert() requires(m == n)
    {
        MatrixT<n, n, T> id = MatrixT<n, n, T>::Identity();
        // Triangularize A
        for(int j = 0; j < n; j++)                      // Go through rows
        {
            for(int k = j; k < n; k++) {                
                if(data[k * m + j]) {                 // Find first column with non-zero entry on row j, swap to col j
                    ColSwap(j, k);
                    id.ColSwap(j, k);
                    break;
                }    
            }

            if(data[j * m + j] == 0) { throw "Inverse of irregular matrix requested."; }

            // Eliminate j-th row
            for (int i = j + 1; i < n; i++)
            {
                T factor = -data[i * m + j] / data[j * m + j];
                ColOp(j, i, factor);
                id.ColOp(j, i, factor);
            }
        }

        // Empty upper triangle
        for(int j = 0; j < n; j++)                      // Go through rows
        {
            // Eliminate j-th row
            for (int i = 0; i < j; i++)
            {
                T factor = -data[i * m + j] / data[j * m + j];
                ColOp(j, i, factor);
                id.ColOp(j, i, factor);
            }
        }

        // Normalize
        for(int i = 0; i < n; i++) {
            T factor = data[i * n + i];
            for(int j = 0; j < n; j++) {
                data[i * n + j] = id.data[i * n + j] / data[i * n + i];
            }
        }

        return *this;
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, n, T> MatrixT<n, m, T>::Identity() requires(m == n)
    {
        T values[n * n] = { 0 };
        for(int i = 0; i < n; i++) { values[i * n + i] = 1; }
        return MatrixT<n, n, T>(values);
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> MatrixT<n, m, T>::Zero()
    {
        return {};
    }

    template <uint8_t n, uint8_t m, typename T>
    MatrixT<4, 4, T> MatrixT<n, m, T>::LookAt(VectorT<3, T> const & eye, VectorT<3, T> const & target, VectorT<3, T> const & up) requires(m == 4 && n == 4)
    {
        const VectorT<3, T> f = (target - eye).Normalized();
        const VectorT<3, T> r = f.Cross(up).Normalized();
        const VectorT<3, T> u = r.Cross(f).Normalized();

        return MatrixT<4, 4, T>{
                  r[X],       u[X],   -f[X], 0,
                  r[Y],       u[Y],   -f[Y], 0,
                  r[Z],       u[Z],   -f[Z], 0,
            -(r * eye), -(u * eye), f * eye, 1
        };
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<3, 3, T> MatrixT<n, m, T>::RodriguesRotation(VectorT<3, T> const &axis, T theta) requires(m == 3 && n == 3)
    {
        T sinTheta = sin(theta);
        T cosTheta = cos(theta);
        auto a = axis.Normalized();
        return MatrixT<3, 3, T>{
            cosTheta + a[X] * a[X] * (T(1) - cosTheta), 
            a[Z] * sinTheta + a[X] * a[Y] * (T(1) - cosTheta), 
            a[Y] * sinTheta + a[X] * a[Z] * (T(1) - cosTheta), 

            -a[Z] * sinTheta + a[Y] * a[X] * (T(1) - cosTheta), 
            cosTheta + a[Y] * a[Y] * (T(1) - cosTheta), 
            a[X] * sinTheta + a[Y] * a[Z] * (T(1) - cosTheta), 

            a[Y] * sinTheta + a[Z] * a[X] * (T(1) - cosTheta),
            -a[X] * sinTheta + a[Z] * a[Y] * (T(1) - cosTheta),
            cosTheta + a[Z] * a[Z] * (T(1) - cosTheta)
        };
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<4, 4, T> MatrixT<n, m, T>::Rotate(VectorT<3, T> const &axis, T theta) requires(m == 4 && n == 4)
    {
        auto R = MatrixT<3, 3, T>::RodriguesRotation(axis, theta);
        return MatrixT<4, 4, T>{
            R[0][0], R[0][1], R[0][2], 0,
            R[1][0], R[1][1], R[1][2], 0,
            R[2][0], R[2][1], R[2][2], 0,
            0      , 0      , 0      , 1
        };
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<4, 4, T> MatrixT<n, m, T>::Rotate(VectorT<3, T> const &axis, VectorT<3, T> const &center, T theta) requires(m == 4 && n == 4)
    {
        auto R = MatrixT<3, 3, T>::RodriguesRotation(axis, theta);
        auto Rp = center - R * center;
        return MatrixT<4, 4, T>{
            R[0][0], R[0][1], R[0][2], 0,
            R[1][0], R[1][1], R[1][2], 0,
            R[2][0], R[2][1], R[2][2], 0,
            Rp[0]  , Rp[1]  , Rp[2]  , 1
        };
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<4, 4, T> MatrixT<n, m, T>::Perspective(T near, T far, T fov, T aspectRatio) requires(m == 4 && n == 4)
    {
        T s = T(1) / tan(PI * fov / T(360));
        return MatrixT<4, 4, T>{
            s / aspectRatio, 0, 0, 0,
            0, -s, 0, 0,
            0, 0, - (far + near) / (far - near), -1,
            0, 0, - 2 * far * near / (far - near), 0
        };
    }

    template <uint8_t n, uint8_t m, typename T>
    inline VectorT<3, T> MatrixT<n, m, T>::Cross(VectorT<3, T> const &other) const requires(m == 1 && n == 3)
    {
        return VectorT<3, T>{
            data[Y] * other[Z] - data[Z] * other[Y],
            data[Z] * other[X] - data[X] * other[Z],
            data[X] * other[Y] - data[Y] * other[X]
        };
    }

    template <uint8_t n, uint8_t m, typename T>
    inline void MatrixT<n, m, T>::RowOp(uint8_t row1, uint8_t row2, T factor)
    {
        for(int i = 0; i < n; i++) 
        {
            data[i * m + row2] += data[i * m + row1];
        }
    }

    template <uint8_t n, uint8_t m, typename T>
    inline void MatrixT<n, m, T>::ColOp(uint8_t col1, uint8_t col2, T factor)
    {
        for(int j = 0; j < n; j++) 
        {
            data[col2 * m + j] += factor * data[col1 * m + j];
        }
    }

    template <uint8_t n, uint8_t m, typename T>
    inline void MatrixT<n, m, T>::ColSwap(uint8_t col1, uint8_t col2)
    {
        for(int j = 0; j < n; j++) 
        {
            T temp = data[col2 * m + j];
            data[col2 * m + j] = data[col1 * m + j];
            data[col1 * m + j] = temp;
        }
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T>::MatrixT(T const values[n][m])
    {
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < m; j++) {
                data[i * m = j] = values[i][j];
            }
        }
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T>::MatrixT(std::initializer_list<T> values)
    {
        int size = values.size();
        if(n * m < size) size = n * m;
        auto arg = values.begin();
        for(int i = 0; i < size; i++) { this->data[i] = *arg; arg++; }
        for(int i = size; i < n * m; i++) { this->data[i] = 0; }
    }

    template <uint8_t n, uint8_t m, typename T>
    template <uint8_t l>
    inline MatrixT<n, l, T> MatrixT<n, m, T>::operator*(MatrixT<m, l, T> const &other) const
    {
        T newVals[n * l];
        for(int i = 0; i < n; i++) {                // Columns of result
            for(int j = 0; j < l; j++) {            // Rows of result
                newVals[i * n + j] = 0;
                for(int k = 0; k < m; k++) {
                    newVals[i * n + j] += data[k * n + j] * other.data[i * m + k];
                }
            }
        }
        return MatrixT<n, l, T>(newVals);
    }

    template <uint8_t n, uint8_t m, typename T>
    inline bool MatrixT<n, m, T>::operator==(MatrixT<n, m, T> const &other) const
    {
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < m; j++) {
                if (abs(data[i * m + j] - other.data[i * m + j]) > EPS) return false;
            }
        }
        return true;
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> MatrixT<n, m, T>::operator+(MatrixT<n, m, T> const &other) const
    {
        T newVals[n * m];
        for(int k = 0; k < n * m; k++) { newVals[k] = data[k] + other[k]; }
        return MatrixT<n, m, T>(newVals);
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> MatrixT<n, m, T>::operator-(MatrixT<n, m, T> const &other) const
    {
        T newVals[n * m];
        for(int k = 0; k < n * m; k++) { newVals[k] = data[k] - other[k]; }
        return MatrixT<n, m, T>(newVals);
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> MatrixT<n, m, T>::operator+(T value) const
    {
        T newVals[n * m];
        for(int k = 0; k < n * m; k++) { newVals[k] = data[k] + value; }
        return MatrixT<n, m, T>(newVals);
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> MatrixT<n, m, T>::operator-(T value) const
    {
        T newVals[n * m];
        for(int k = 0; k < n * m; k++) { newVals[k] = data[k] - value; }
        return MatrixT<n, m, T>(newVals);
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator+=(MatrixT<n, m, T> const &other)
    {
        for(int k = 0; k < n * m; k++) { data[k] += other[k]; }
        return *this;
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator+=(T value)
    {
        for(int k = 0; k < n * m; k++) { data[k] += value; }
        return *this;
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator-=(MatrixT<n, m, T> const &other)
    {
        for(int k = 0; k < n * m; k++) { data[k] += other[k]; }
        return *this;
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator-=(T value)
    {
        for(int k = 0; k < n * m; k++) { data[k] -= value; }
        return *this;
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator*=(MatrixT<n, m, T> const &other)
    {
        for(int k = 0; k < n * m; k++) { data[k] *= other[k]; }
        return *this;
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator*=(T value)
    {
        for(int k = 0; k < n * m; k++) { data[k] *= value; }
        return *this;
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator/=(T value)
    {
        for(int k = 0; k < n * m; k++) { data[k] /= value; }
        return *this;
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> MatrixT<n, m, T>::operator*(T value) const
    {
        T newVals[n * m];
        for(int k = 0; k < n * m; k++) { newVals[k] = data[k] * value; }
        return MatrixT<n, m, T>(newVals);
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, m, T> MatrixT<n, m, T>::operator/(T value) const
    {
        T newVals[n * m];
        for(int k = 0; k < n * m; k++) { newVals[k] = data[k] / value; }
        return MatrixT<n, m, T>(newVals);
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<m, n, T> MatrixT<n, m, T>::Transposed() const
    {
        T newVals[m * n];
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < m; j++) { newVals[j * n + i] = data[i * m + j]; }
        }
        return MatrixT<m, n, T>(newVals);
    }

    template<uint8_t n, uint8_t m, typename T>
    inline MatrixT<m, 1, T> MatrixT<n, m, T>::operator[](uint8_t i) const {
        T _data[n] = {0};
        for(int j = 0; j < m; j++) {
            _data[j] = data[i * m + j];
        }
        return MatrixT<m, 1, T>(data);
    }
    

} // namespace Engine::Math

#include <functional>
namespace std {
    template <uint8_t n, typename T>
    struct hash<Engine::Maths::VectorT<n, T>> {
        inline size_t operator()(Engine::Maths::VectorT<n, T> const& v) const {
            size_t h = 0;
            for(uint8_t i = 0; i < n; i++) {
                size_t a = hash<uint8_t>{}(i);
                h <<= hash<uint8_t>{}(i) % 4;
                h ^= hash<T>{}(v[i]);
            }
            return h;
        }
    };

    template <uint8_t n, uint8_t m, typename T>
    struct hash<Engine::Maths::MatrixT<n, m, T>> {
        inline size_t operator()(Engine::Maths::MatrixT<n, m, T> const& mat) const {
            size_t h = 0;
            for(uint8_t i = 0; i < n; i++) {
                for(uint8_t j = 0; j < m; j++) {
                    size_t a = hash<uint8_t>{}(i);
                    h <<= hash<uint8_t>{}(i) % 4;
                    h ^= hash<T>{}(mat[i][j]);
                    h >>= hash<uint8_t>{}(j) % 4;
                }
            }
            return h;
        }
    };
}   // namespace std