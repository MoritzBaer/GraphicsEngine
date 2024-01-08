#pragma once

#include <stdint.h>
#include <initializer_list>
#include <cmath>


namespace Engine::Math
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

            static MatrixT<4, 4, T> LookAt(VectorT<3,T> const & eye, VectorT<3, T> const & target, VectorT<3, T> const & up) requires(m == 3 && n == 3);

            // +--------------------------------+
            // |    Vector-specific operations  |
            // +--------------------------------+
            inline T SqrMagnitude() const requires(m == 1) { return (Transposed() * *this)[0]; }
            inline T Length() const requires(m == 1) { return std::sqrt(SqrMagnitude()); }
            inline T &operator[](uint8_t i) requires(m == 1) { return data[i]; }
            inline T const &operator[](uint8_t i) const requires(m == 1) { return data[i]; }
            inline VectorT<n, T> Normalized() const requires(m == 1) { return *this / this->Length(); }
            inline VectorT<n, T> & Normalize() requires(m == 1) { return (*this /= this->Length); }
            inline VectorT<3, T> Cross(VectorT<3, T> const& other) const requires(m == 1 && n == 3);

            // "Properties" for easier access
        private:
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
                    inline  operator T () { return parent.data[index]; }
                    inline T operator+(T value) { return parent.data[index] + value; }
                    inline T operator-(T value) { return parent.data[index] - value; }
                    inline T operator*(T value) { return parent.data[index] * value; }
                    inline T operator/(T value) { return parent.data[index] / value; }
            };

            class EntryPair {
                    MatrixT<n, 1, T> &parent;
                    uint8_t index1;
                    uint8_t index2;
                public:
                    EntryPair(MatrixT<n, 1, T> &v, uint8_t i1, uint8_t i2) : parent(v), index1(i1), index2(i2) { }
                    inline void operator=(MatrixT<2, 1, T> const & values) { parent.data[index1] = values[0]; parent.data[index2] = values[1]; }
                    inline void operator+=(MatrixT<2, 1, T> const & values) { parent.data[index1] += values[0]; parent.data[index2] += values[1]; }
                    inline void operator+=(T value) { parent.data[index1] += value; parent.data[index2] += value; }
                    inline void operator-=(MatrixT<2, 1, T> const & values) { parent.data[index1] -= values[0]; parent.data[index2] -= values[1]; }
                    inline void operator-=(T value) { parent.data[index1] += value; parent.data[index2] -= value; }
                    inline void operator*=(T value) { parent.data[index1] *= value; parent.data[index2] *= value; }
                    inline void operator/=(T value) { parent.data[index1] /= value; parent.data[index2] /= value; }
                    inline  operator MatrixT<2, 1, T> () { return MatrixT<2, 1, T>({ parent.data[index1], parent.data[index2] }); }
            };

            class EntryTriplet {
                    MatrixT<n, 1, T> &parent;
                    uint8_t index1;
                    uint8_t index2;
                    uint8_t index3;
                public:
                    EntryTriplet(MatrixT<n, 1, T> &v, uint8_t i1, uint8_t i2, uint8_t i3) : parent(v), index1(i1), index2(i2), index3(i3) { }
                    inline void operator=(MatrixT<3, 1, T> const & values) { parent.data[index1] = values[0]; parent.data[index2] = values[1]; parent.data[index3] = values[2]; }
                    inline void operator+=(MatrixT<3, 1, T> const & values) { parent.data[index1] += values[0]; parent.data[index2] += values[1]; parent.data[index3] += values[2]; }
                    inline void operator+=(T value) { parent.data[index1] += value; parent.data[index2] += value; parent.data[index3] += value; }
                    inline void operator-=(MatrixT<3, 1, T> const & values) { parent.data[index1] -= values[0]; parent.data[index2] -= values[1]; parent.data[index3] -= values[2]; }
                    inline void operator-=(T value) { parent.data[index1] += value; parent.data[index2] -= value; parent.data[index3] -= value; }
                    inline void operator*=(T value) { parent.data[index1] *= value; parent.data[index2] *= value; parent.data[index3] *= value; }
                    inline void operator/=(T value) { parent.data[index1] /= value; parent.data[index2] /= value; parent.data[index3] /= value; }
                    inline  operator MatrixT<3, 1, T> () { return MatrixT<3, 1, T>(parent.data[index1], parent.data[index2], parent.data[index3]); }
            };

        public:
            Entry X() { return Entry(*this, 0); }
            Entry Y() requires(n >= 2) { return Entry(*this, 1); }
            Entry Z() requires(n >= 3) { return Entry(*this, 2); }
            Entry W() requires(n >= 4) { return Entry(*this, 3); }

            EntryPair XY() requires(n >= 2) { return EntryPair(*this, 0, 1); }
            EntryPair XZ() requires(n >= 3) { return EntryPair(*this, 0, 2); }
            EntryPair YZ() requires(n >= 3) { return EntryPair(*this, 1, 2); }

            EntryTriplet XYZ() requires(n >= 3) { return EntryTriplet(*this, 0, 1, 2); }

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
    MatrixT<4, 4, T> MatrixT<n, m, T>::LookAt(VectorT<3, T> const & eye, VectorT<3, T> const & target, VectorT<3, T> const & up) requires(m == 3 && n == 3)
    {
        const VectorT<3, T> f = (target - eye).Normalized();
        const VectorT<3, T> r = forward.Cross(up).Normalized();
        const VectorT<3, T> u = right.Cross(forward).Normalized();

        return MatrixT<3, 3, T>(
            f[0], f[1], f[2], 0,
            u[0], f[1], f[2], 0,
            r[0], r[1], r[2], 0,
            eye[0], eye[1], eye[2]
        );
    }

    template <uint8_t n, uint8_t m, typename T>
    inline VectorT<3, T> MatrixT<n, m, T>::Cross(VectorT<3, T> const &other) const requires(m == 1 && n == 3)
    {
        return VectorT<3, T>(
            data[1] * other[2] - data[2] * other[1],
            data[2] * other[0] - data[0] * other[2],
            data[0] * other[1] - data[1] * other[0]
        );
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
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < l; j++) {
                newVals[i * l + j] = 0;
                for(int k = 0; k < m; k++) {
                    newVals[i * l + j] += data[i * m + k] * other.data[k * l + j];
                }
            }
        }
        return MatrixT<n, l, T>(newVals);
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

} // namespace Engine::Math
