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
                inline T const &operator[](uint8_t i) const { return data[i]; }
                inline VectorT<n, T>(T const data[n]);
                inline VectorT<n, T>(std::initializer_list<T> args);
                inline VectorT<n, T>& operator=(T const data[n]) { for(int i = 0; i < n; i++) { this->data[i] = data[i]; } return *this; }
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

                // "Properties" for easier access 
                class VectorX{ 
                        VectorT<n, T> &parent;
                    public:
                        VectorX(VectorT<n, T> &v) : parent(v) { }
                        inline T & operator=(T x) { return parent.data[0] = x; }
                        inline operator T () { return parent.data[0]; }
                };
                inline VectorX X() { return VectorX(*this); }

                class VectorY { 
                        VectorT<n, T> &parent;
                    public:
                        VectorY(VectorT<n, T> &v) : parent(v) { }
                        inline T & operator=(T y) { return parent.data[1] = y; }
                        inline operator T () { return parent.data[1]; }
                };
                inline VectorY Y() requires(n >= 2) { return VectorY(*this); }

                class VectorXY { 
                        VectorT<n, T> &parent;
                    public:
                        VectorXY(VectorT<n, T> &v) : parent(v) { }
                        inline void operator=(VectorT<2, T> const & xy) { 
                            parent.data[0] = xy[0];
                            parent.data[1] = xy[1];
                        }
                        inline operator VectorT<2, T> () { return VectorT<2, T> { parent.data[0], parent.data[1] }; }
                };
                inline VectorXY XY() requires(n >= 2) { return VectorXY(*this); }

                class VectorZ { 
                        VectorT<n, T> &parent;
                    public:
                        VectorZ(VectorT<n, T> &v) : parent(v) { }
                        inline T & operator=(T z) { return parent.data[2] = z; }
                        inline operator T () { return parent.data[2]; }
                };
                inline VectorZ Z() requires(n >= 3) { return VectorZ(*this); }

                class VectorXZ { 
                        VectorT<n, T> &parent;
                    public:
                        VectorXZ(VectorT<n, T> &v) : parent(v) { }
                        inline void operator=(VectorT<2, T> const & xz) { 
                            parent.data[0] = xz[0];
                            parent.data[2] = xz[2];
                        }
                        inline operator VectorT<2, T> () { return VectorT<2, T> { parent.data[0], parent.data[2] }; }
                };
                inline VectorXZ XZ() requires(n >= 3) { return VectorXZ(*this); }

                class VectorYZ { 
                        VectorT<n, T> &parent;
                    public:
                        VectorYZ(VectorT<n, T> &v) : parent(v) { }
                        inline void operator=(VectorT<2, T> const & yz) { 
                            parent.data[1] = yz[1];
                            parent.data[2] = yz[2];
                        }
                        inline operator VectorT<2, T> () { return VectorT<2, T> { parent.data[1], parent.data[2] }; }
                };
                inline VectorYZ YZ() requires(n >= 3) { return VectorYZ(*this); }

                class VectorXYZ { 
                        VectorT<n, T> &parent;
                    public:
                        VectorXYZ(VectorT<n, T> &v) : parent(v) { }
                        inline void operator=(VectorT<3, T> const & xyz) { 
                            parent.data[0] = xyz[0];
                            parent.data[1] = xyz[1];
                            parent.data[2] = xyz[2];
                        }
                        inline operator VectorT<2, T> () { return VectorT<2, T> { parent.data[1], parent.data[2] }; }
                };
                inline VectorXYZ XYZ() requires(n >= 3) { return VectorXYZ(*this); }

            private:
                inline void InitProperties();
                T data[n];
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
            T sqrMagnitude = 0;
            for(int i = 0; i < n; i++) { sqrMagnitude += data[i] * data[i]; }
            return sqrMagnitude;
        }

        template<uint8_t n, typename T>
        inline T VectorT<n, T>::Length() {
            return std::sqrt(this->SqrMagnitude());
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T> &VectorT<n, T>::operator=(VectorT<n, T> const &other)
        {
            for(int i = 0; i < n; i++) { this->data[i] = data[i]; } 
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
            T newData[n] = { 0 };
            for(int i = 0; i < n; i++) { newData[i] = data[i] + value; }
            return VectorT<n, T>(data);
        }
        
        template <uint8_t n, typename T>
        inline VectorT<n, T> VectorT<n, T>::operator-(T value) const
        {
            T newData[n] = { 0 };
            for(int i = 0; i < n; i++) { newData[i] = data[i] - value; }
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
        inline void VectorT<n, T>::InitProperties()
        {
            X(*this);
        }

        template <uint8_t n, typename T>
        inline VectorT<n, T>::VectorT(T const data[n]) { for(int i = 0; i < n; i++) { this->data[i] = data[i]; } }

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
