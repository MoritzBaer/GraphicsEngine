#pragma once

#include "Vector.h"

namespace Engine::Math
{
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
            template<uint8_t l>
            inline MatrixT<n, l, T> operator*(MatrixT<m, l, T> const &other) const;
            inline MatrixT<n, m, T> operator*(T value) const;
            inline MatrixT<n, m, T> operator/(T value) const;
            inline friend MatrixT<n, m, T> operator*(T value, MatrixT<n, m, T> const &matrix) { return matrix * value; }
            inline VectorT<n, T> operator*(VectorT<m, T> const &vector) const;
            inline MatrixT<m, n, T> Transposed() const;
            inline MatrixT<n, n, T> Inverse() const;
            inline MatrixT<n, n, T>& Invert();
            inline static MatrixT<n, n, T> Identity();
        private:
            // Adds factor * row1 to row2
            inline void RowOp(uint8_t row1, uint8_t row2, T factor);
            inline void ColOp(uint8_t col1, uint8_t col2, T factor);
            inline void ColSwap(uint8_t col1, uint8_t col2);
            T data[n * m];
    };

    template<uint8_t n, uint8_t m>
    using MatrixNM = MatrixT<n, m, float>;

    template<uint8_t n>
    using Matrix = MatrixNM<n, n>;

    using Matrix2 = Matrix<2>;
    using Matrix3 = Matrix<3>;
    using Matrix4 = Matrix<4>;

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, n, T> MatrixT<n, m, T>::Inverse() const
    {
        return MatrixT<n, n, T>(data).Invert();
    }

    template <uint8_t n, uint8_t m, typename T>
    inline MatrixT<n, n, T>& MatrixT<n, m, T>::Invert()
    {
        static_assert(n == m, "Inverse of non-square matrix requested.");
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
    inline MatrixT<n, n, T> MatrixT<n, m, T>::Identity()
    {
        static_assert(n == m, "Non-square identity requested.");
        T values[n * n] = { 0 };
        for(int i = 0; i < n; i++) { values[i * n + i] = 1; }
        return MatrixT<n, n, T>(values);
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
    inline VectorT<n, T> MatrixT<n, m, T>::operator*(VectorT<m, T> const &vector) const
    {
        T newVals[n];
        for(int i = 0; i < n; i++) {
            newVals[i] = 0;
            for(int j = 0; j < m; j++) { newVals[i] += data[i * m + j] * vector[j]; }
        }
        return VectorT<n, T>(newVals);
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
