#pragma once
#include <inttypes.h>
#include <cmath>
#include <initializer_list>

namespace Engine
{
    namespace Math
    {
        template<uint8_t n, typename T>
        struct VectorT
        {
                inline T SqrMagnitude();
                inline T Length();
                inline T &operator[](uint8_t i) { return data[i]; }
                inline VectorT<n, T>(T const data[n]) { for(int i = 0; i < n; i++) { this->data[i] = data[i]; } sqrMagnitude = 0; length = 0; changeSincePrecalc = true; }
                inline VectorT<n, T>(std::initializer_list<T> args);
                inline VectorT<n, T>& operator=(T const data[n]) { for(int i = 0; i < n; i++) { this->data[i] = data[i]; } this->changeSincePrecalc=true; return *this; }
                inline VectorT<n, T>& operator=(VectorT<n, T> const &other);
                inline VectorT<n, T> operator+(VectorT<n, T> const &other) const;
                inline VectorT<n, T> operator-(VectorT<n, T> const &other) const;
                inline VectorT<n, T> operator+(T value) const;
                inline VectorT<n, T> operator-(T value) const;
                inline friend VectorT<n, T> operator+(T value, VectorT<n, T> const &vector) { return vector + value; }
                inline VectorT<n, T> & operator+=(VectorT<n, T> const &other);
                inline VectorT<n, T> & operator-=(VectorT<n, T> const &other);
                inline VectorT<n, T> & operator+=(T value);
                inline VectorT<n, T> & operator-=(T value);
                inline VectorT<n, T> operator*(T value) const;
                inline VectorT<n, T> operator/(T value) const;
                inline VectorT<n, T> operator*(VectorT<n, T> const &other) const;
                inline VectorT<n, T> operator/(VectorT<n, T> const &other) const;
                inline friend VectorT<n, T> operator*(T value, VectorT<n, T> const &vector) { return vector * value; }
                inline VectorT<n, T> & operator*=(T value);
                inline VectorT<n, T> & operator/=(T value);
                inline VectorT<n, T> & operator*=(VectorT<n, T> const &other);
                inline VectorT<n, T> & operator/=(VectorT<n, T> const &other);
                inline VectorT<n, T> Normalized();
                inline VectorT<n,T> & Normalize();

            private:
                T data[n];
                T length;
                T sqrMagnitude;
                bool changeSincePrecalc = true;
        };
        
        template<uint8_t n>
        using Vector = VectorT<n, float>;
        template<uint8_t n>
        using IntVector = VectorT<n, int32_t>;

        using Vector2 = Vector<2>;
        using Vector3 = Vector<3>;
        using Vector4 = Vector<4>;

        template<uint8_t n, typename T>
        inline T VectorT<n, T>::SqrMagnitude() {
            if(changeSincePrecalc) {
                sqrMagnitude = 0;
                for(int i = 0; i < n; i++) { sqrMagnitude += data[i] * data[i]; }
            }
            return sqrMagnitude;
        }

        template<uint8_t n, typename T>
        inline T VectorT<n, T>::Length() {
            if(changeSincePrecalc) { length = std::sqrt(this->SqrMagnitude()); }
            return length;
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::operator=(VectorT<n, T> const &other)
        {
            for(int i = 0; i < n; i++) { this->data[i] = data[i]; } 
            this->length = other.length; 
            this->sqrMagnitude = other.sqrMagnitude; 
            this->changeSincePrecalc = other.changeSincePrecalc; 
            return *this;
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> VectorT<n, T>::operator+(VectorT<n, T> const &other) const
        {
            T data[n] = { 0 };
            for(int i = 0; i < n; i++) { data[i] = this->data[i] + other.data[i]; }
            return VectorT<n, T>(data);
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> VectorT<n, T>::operator-(VectorT<n, T> const &other) const
        {
            T data[n] = { 0 };
            for(int i = 0; i < n; i++) { data[i] = this->data[i] - other.data[i]; }
            return VectorT<n, T>(data);
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> VectorT<n, T>::operator+(T value) const
        {
            T[n] newData = { 0 };
            for(int i = 0; i < n; i++) { newData[i] = vector.data[i] + value; }
            return VectorT<n, T>(data);
        }
        
        template <uint8_t n, typename T>
        inline VectorT<n, T> VectorT<n, T>::operator-(T value) const
        {
            T[n] newData = { 0 };
            for(int i = 0; i < n; i++) { newData[i] = vector.data[i] - value; }
            return VectorT<n, T>(data);
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::operator+=(VectorT<n, T> const &other)
        {
            for(int i = 0; i < n; i++) { data[i] = this->data[i] += other.data[i]; }
            return *this;
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::operator-=(VectorT<n, T> const &other)
        {
            for(int i = 0; i < n; i++) { data[i] = this->data[i] -= other.data[i]; }
            return *this;
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::operator+=(T value)
        {
            for(int i = 0; i < n; i++) { data[i] = this->data[i] += value; }
            return *this;
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::operator-=(T value)
        {
            for(int i = 0; i < n; i++) { data[i] = this->data[i] -= value; }
            return *this;
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> VectorT<n, T>::operator*(T value) const
        {
            T data[n] = { 0 };
            for(int i = 0; i < n; i++) { data[i] = this->data[i] * value; }
            return VectorT<n, T>(data);
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> VectorT<n, T>::operator/(T value) const
        {
            T data[n] = { 0 };
            for(int i = 0; i < n; i++) { data[i] = this->data[i] / value; }
            return VectorT<n, T>(data);
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> VectorT<n, T>::operator*(VectorT<n, T> const &other) const
        {
            T data[n] = { 0 };
            for(int i = 0; i < n; i++) { data[i] = this->data[i] * other[i]; }
            return VectorT<n, T>(data);
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> VectorT<n, T>::operator/(VectorT<n, T> const &other) const
        {
            T data[n] = { 0 };
            for(int i = 0; i < n; i++) { data[i] = this->data[i] / other[i]; }
            return VectorT<n, T>(data);
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::operator*=(T value)
        {
            for(int i = 0; i < n; i++) { data[i] *= value; }
            return *this;
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::operator/=(T value)
        {
            for(int i = 0; i < n; i++) { data[i] /= value; }
            return *this;
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::operator*=(VectorT<n, T> const &other)
        {
            for(int i = 0; i < n; i++) { data[i] *= other.data[i]; }
            return *this;
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::operator/=(VectorT<n, T> const &other)
        {
            for(int i = 0; i < n; i++) { data[i] /= other.data[i]; }
            return *this;
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> VectorT<n, T>::Normalized()
        {
            return *this / this->Length();
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::Normalize()
        {
            return this /= this->Length();
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T>::VectorT(std::initializer_list<T> args)
        {
            int size = args.size();
            if(n < size) size = n;
            auto arg = args.begin();
            for(int i = 0; i < size; i++) { this->data[i] = *arg; arg++; }
            for(int i = size; i < n; i++) { this->data[i] = 0; }
        }

    } // namespace Math

} // namespace Engine
