#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <sstream>
#include <stdint.h>
#include <tuple>

#include "json-parsing.h"

#define PI 3.14159265359
#define EPS 0.0000001

#define MATRIX_AT_IJ(n, m, row, col) col *n + row
#define MATRIX_NM_AT_IJ(row, col) MATRIX_AT_IJ(n, m, row, col)

inline constexpr uint8_t X = 0;
inline constexpr uint8_t Y = 1;
inline constexpr uint8_t Z = 2;
inline constexpr uint8_t W = 3;

namespace Engine::Maths {
template <uint8_t n, uint8_t m, typename T> struct MatrixT;

template <uint8_t n, uint8_t m> using MatrixNM = MatrixT<n, m, float>;

template <uint8_t n> using Matrix = MatrixNM<n, n>;

using Matrix2 = Matrix<2>;
using Matrix3 = Matrix<3>;
using Matrix4 = Matrix<4>;

template <uint8_t n, typename T> using VectorT = MatrixT<n, 1, T>;

template <uint8_t n> using Vector = VectorT<n, float>;
using Vector2 = Vector<2>;
using Vector3 = Vector<3>;
using Vector4 = Vector<4>;

template <typename T> constexpr uint8_t alignment(uint8_t n, uint8_t m) {
  uint8_t numBytes = n * m * sizeof(T);
  if (numBytes <= 1)
    return 1;
  if (numBytes <= 2)
    return 2;
  if (numBytes <= 4)
    return 4;
  if (numBytes <= 8)
    return 8;
  return 16;
}

} // namespace Engine::Maths

namespace Engine::Maths {

template <typename T2, typename T1>
concept AdditiveAutoCasting = requires(T1 const &value1, T2 const &value2) {
  { value1 + value2 } -> std::convertible_to<T1>;
  { value1 - value2 } -> std::convertible_to<T1>;
};

template <typename T2, typename T1>
concept MultiplicativeAutoCasting = requires(T1 const &value1, T2 const &value2) {
  { value1 *value2 } -> std::convertible_to<T1>;
  { value1 / value2 } -> std::convertible_to<T1>;
};

template <uint8_t n, typename T, uint8_t index> inline constexpr T &Access(VectorT<n, T> &v) { return v[index]; }
template <uint8_t n, typename T, uint8_t index> inline constexpr T const &Access(VectorT<n, T> const &v) {
  return v[index];
}

template <uint8_t n, typename T, uint8_t... indexes> class EntryReference {
  VectorT<n, T> &parent;

public:
  inline static constexpr uint8_t numEntries = static_cast<uint8_t>(sizeof...(indexes));
  EntryReference(VectorT<n, T> &v) : parent(v) {}

  // Access
  inline constexpr operator VectorT<numEntries, T>() const {
    return VectorT<numEntries, T>(Access<n, T, indexes>(parent)...);
  }
  inline constexpr operator T&() const requires(numEntries == 1) {
    constexpr uint8_t idxs[] = {indexes...};
    return Access<n, T, idxs[0]>(parent);
  }
  inline constexpr T operator[](uint8_t entry) const {
    VectorT<numEntries, T> ref = *this;
    return ref[entry];
  }
  inline constexpr VectorT<numEntries, T> operator-() const {
    return VectorT<numEntries, T>(-Access<n, T, indexes>(parent)...);
  }

  // Assignment
  template <std::convertible_to<T> T2>
  inline constexpr EntryReference<n, T, indexes...> & operator=(VectorT<numEntries, T2> const &newValues) {
    uint8_t i = 0;
    ((Access<n, T, indexes>(parent) = newValues[i++]), ...);
    return *this;
  }
  template <std::convertible_to<T> T2>
  inline constexpr EntryReference<n, T, indexes...> & operator=(T2 const &newValue) {
    uint8_t i = 0;
    ((Access<n, T, indexes>(parent) = newValue), ...);
    return *this;
  }
  template <std::convertible_to<T> T2, uint8_t other_n, uint8_t... other_indexes>
  inline constexpr EntryReference<n, T, indexes...> & operator=(EntryReference<other_n, T2, other_indexes...> const &newValues) 
    requires (EntryReference<other_n, T2, other_indexes...>::numEntries == numEntries)
  {
    VectorT<numEntries, T2> newVals = newValues;
    return (*this = newVals);
  }
  template <std::convertible_to<T> T2, uint8_t other_n, uint8_t other_index>
  inline constexpr EntryReference<n, T, indexes...> & operator=(EntryReference<other_n, T2, other_index> const &newValue) {
    T2 newVal = newValue;
    return (*this = newVal);
  }

  // Special case in addition
  template <AdditiveAutoCasting<T> T2, uint8_t other_n>
  inline friend constexpr VectorT<other_n, T> operator+(VectorT<other_n, T2> const &other,
                                                        EntryReference<n, T, indexes...> const &entries)
    requires(numEntries == 1)
  {
    return other + entries[0];
  }
  template <AdditiveAutoCasting<T> T2, uint8_t other_n>
  inline friend constexpr VectorT<other_n, T> operator-(VectorT<other_n, T2> const &other,
                                                        EntryReference<n, T, indexes...> const &entries)
    requires(numEntries == 1)
  {
    return other - entries[0];
  }

  // Adding values
  template <AdditiveAutoCasting<T> T2> inline constexpr VectorT<numEntries, T> operator+(T2 const &value) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent) + value)...);
  }
  template <AdditiveAutoCasting<T> T2>
  inline friend constexpr VectorT<numEntries, T> operator+(T2 const &value,
                                                           EntryReference<n, T, indexes...> const &entries) {
    return entries + value;
  }
  template <AdditiveAutoCasting<T> T2> inline constexpr VectorT<numEntries, T> operator-(T2 const &value) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent) - value)...);
  }
  template <AdditiveAutoCasting<T> T2>
  inline friend constexpr VectorT<numEntries, T> operator-(T2 const &value,
                                                           EntryReference<n, T, indexes...> const &entries) {
    return -entries + value;
  }
  template <AdditiveAutoCasting<T> T2> inline constexpr EntryReference<n, T, indexes...> &operator+=(T2 const &value) {
    ((Access<n, T, indexes>(parent) += value), ...);
    return *this;
  }
  template <AdditiveAutoCasting<T> T2> inline constexpr EntryReference<n, T, indexes...> &operator-=(T2 const &value) {
    ((Access<n, T, indexes>(parent) -= value), ...);
    return *this;
  }

  template <AdditiveAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline constexpr VectorT<numEntries, T> operator+(EntryReference<other_n, T2, other_index> const &value) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent) + value[0])...);
  }
  template <AdditiveAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline friend constexpr VectorT<numEntries, T> operator+(EntryReference<other_n, T2, other_index> const &value,
                                                           EntryReference<n, T, indexes...> const &entries) {
    return entries + value;
  }
  template <AdditiveAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline constexpr VectorT<numEntries, T> operator-(EntryReference<other_n, T2, other_index> const &value) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent) - value[0])...);
  }
  template <AdditiveAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline friend constexpr VectorT<numEntries, T> operator-(EntryReference<other_n, T2, other_index> const &value,
                                                           EntryReference<n, T, indexes...> const &entries) {
    return -entries + value;
  }
  template <AdditiveAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline constexpr EntryReference<n, T, indexes...> &operator+=(EntryReference<other_n, T2, other_index> const &value) {
    ((Access<n, T, indexes>(parent) += value[0]), ...);
    return *this;
  }
  template <AdditiveAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline constexpr EntryReference<n, T, indexes...> &operator-=(EntryReference<other_n, T2, other_index> const &value) {
    ((Access<n, T, indexes>(parent) -= value[0]), ...);
    return *this;
  }

  // Adding vectors
  template <AdditiveAutoCasting<T> T2>
  inline constexpr VectorT<numEntries, T> operator+(VectorT<numEntries, T2> const &other) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent))...) + other;
  }
  template <AdditiveAutoCasting<T> T2>
  inline friend constexpr VectorT<numEntries, T> operator+(VectorT<numEntries, T2> const &other,
                                                           EntryReference<n, T, indexes...> const &entries) {
    return entries + other;
  }
  template <AdditiveAutoCasting<T> T2>
  inline constexpr VectorT<numEntries, T> operator-(VectorT<numEntries, T2> const &other) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent))...) - other;
  }
  template <AdditiveAutoCasting<T> T2>
  inline friend constexpr VectorT<numEntries, T> operator-(VectorT<numEntries, T2> const &other,
                                                           EntryReference<n, T, indexes...> const &entries) {
    return -entries + other;
  }
  template <AdditiveAutoCasting<T> T2>
  inline constexpr EntryReference<n, T, indexes...> &operator+=(VectorT<numEntries, T2> const &other) {
    uint8_t i = 0;
    ((Access<n, T, indexes>(parent) += other[i++]), ...);
    return *this;
  }
  template <AdditiveAutoCasting<T> T2>
  inline constexpr EntryReference<n, T, indexes...> &operator-=(VectorT<numEntries, T2> const &other) {
    uint8_t i = 0;
    ((Access<n, T, indexes>(parent) -= other[i++]), ...);
    return *this;
  }

  template <AdditiveAutoCasting<T> T2, uint8_t other_n, uint8_t... other_indexes>
  inline constexpr VectorT<numEntries, T> operator+(EntryReference<other_n, T2, other_indexes...> const &other) const
    requires(EntryReference<other_n, T2, other_indexes...>::numEntries == numEntries)
  {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent))...) + other;
  }
  template <AdditiveAutoCasting<T> T2, uint8_t other_n, uint8_t... other_indexes>
  inline constexpr VectorT<numEntries, T> operator-(EntryReference<other_n, T2, other_indexes...> const &other) const
    requires(EntryReference<other_n, T2, other_indexes...>::numEntries == numEntries)
  {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent))...) - other;
  }
  template <AdditiveAutoCasting<T> T2, uint8_t other_n, uint8_t... other_indexes>
  inline constexpr EntryReference<n, T, indexes...> &
  operator+=(EntryReference<other_n, T2, other_indexes...> const &other)
    requires(EntryReference<other_n, T2, other_indexes...>::numEntries == numEntries)
  {
    uint8_t i = 0;
    ((Access<n, T, indexes>(parent) += other[i++]), ...);
    return *this;
  }
  template <AdditiveAutoCasting<T> T2, uint8_t other_n, uint8_t... other_indexes>
  inline constexpr EntryReference<n, T, indexes...> &
  operator-=(EntryReference<other_n, T2, other_indexes...> const &other)
    requires(EntryReference<other_n, T2, other_indexes...>::numEntries == numEntries)
  {
    uint8_t i = 0;
    ((Access<n, T, indexes>(parent) -= other[i++]), ...);
    return *this;
  }

  // Special case in multiplication
  template <MultiplicativeAutoCasting<T> T2, uint8_t other_n>
  inline friend constexpr VectorT<other_n, T> operator*(VectorT<other_n, T2> const &other,
                                                        EntryReference<n, T, indexes...> const &entries)
    requires(numEntries == 1)
  {
    return other * entries[0];
  }
  template <MultiplicativeAutoCasting<T> T2, uint8_t other_n>
  inline friend constexpr VectorT<other_n, T> operator/(VectorT<other_n, T2> const &other,
                                                        EntryReference<n, T, indexes...> const &entries)
    requires(numEntries == 1)
  {
    return other / entries[0];
  }

  // Multiplying by values (outer product)
  template <MultiplicativeAutoCasting<T> T2> inline constexpr VectorT<numEntries, T> operator*(T2 const &value) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent) * value)...);
  }
  template <MultiplicativeAutoCasting<T> T2>
  inline friend constexpr VectorT<numEntries, T> operator*(T2 const &value,
                                                           EntryReference<n, T, indexes...> const &entries) {
    return entries * value;
  }
  template <MultiplicativeAutoCasting<T> T2> inline constexpr VectorT<numEntries, T> operator/(T2 const &value) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent) - value)...);
  }
  template <MultiplicativeAutoCasting<T> T2>
  inline constexpr EntryReference<n, T, indexes...> &operator*=(T2 const &value) {
    ((Access<n, T, indexes>(parent) *= value), ...);
    return *this;
  }
  template <MultiplicativeAutoCasting<T> T2>
  inline constexpr EntryReference<n, T, indexes...> &operator/=(T2 const &value) {
    ((Access<n, T, indexes>(parent) /= value), ...);
    return *this;
  }

  template <MultiplicativeAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline constexpr VectorT<numEntries, T> operator*(EntryReference<other_n, T2, other_index> const &value) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent) * value[0])...);
  }
  template <MultiplicativeAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline friend constexpr VectorT<numEntries, T> operator*(EntryReference<other_n, T2, other_index> const &value,
                                                           EntryReference<n, T, indexes...> const &entries) {
    return entries * value;
  }
  template <MultiplicativeAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline constexpr VectorT<numEntries, T> operator/(EntryReference<other_n, T2, other_index> const &value) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent) / value[0])...);
  }
  template <MultiplicativeAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline constexpr EntryReference<n, T, indexes...> &operator*=(EntryReference<other_n, T2, other_index> const &value) {
    ((Access<n, T, indexes>(parent) *= value[0]), ...);
    return *this;
  }
  template <MultiplicativeAutoCasting<T> T2, uint8_t other_n, uint8_t other_index>
  inline constexpr EntryReference<n, T, indexes...> &operator/=(EntryReference<other_n, T2, other_index> const &value) {
    ((Access<n, T, indexes>(parent) /= value[0]), ...);
    return *this;
  }

  // Multiplying by vectors (inner product)
  template <MultiplicativeAutoCasting<T> T2> inline constexpr T operator*(VectorT<numEntries, T2> const &other) const {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent))...) * other;
  }
  template <MultiplicativeAutoCasting<T> T2>
  inline friend constexpr T operator*(VectorT<numEntries, T2> const &other,
                                      EntryReference<n, T, indexes...> const &entries) {
    return entries * other;
  }

  template <MultiplicativeAutoCasting<T> T2, uint8_t other_n, uint8_t... other_indexes>
  inline constexpr T operator*(EntryReference<other_n, T2, other_indexes...> const &other) const
    requires(EntryReference<other_n, T2, other_indexes...>::numEntries == numEntries)
  {
    return VectorT<numEntries, T>((Access<n, T, indexes>(parent))...) * other;
  }
};

// Saved in column form (n x m means m columns, n rows)
template <uint8_t n, uint8_t m, typename T> struct alignas(alignment<T>(n, m)) MatrixT {
private:
  inline void ConvertToColumnForm();
  MatrixT(bool rowWise, std::array<T, n * m> const &values) : data(values) {
    if (rowWise)
      ConvertToColumnForm();
  }
  template <typename... _T, typename std::enable_if<sizeof...(_T) == n * m, int>::type = 0>
  MatrixT(bool rowWise, _T... values) : data({static_cast<T>(values)...}) {
    if (rowWise) // TODO: Figure out how to use other constructor
      ConvertToColumnForm();
  }
  MatrixT(std::array<T, n * m> const &values) : data(values) {}

public:
  // Public constructors always expect data in row form
  template <typename... _T, typename std::enable_if<sizeof...(_T) == n * m, int>::type = 0>
  MatrixT(_T... values) : MatrixT(true, values...) {}
  MatrixT() : data() {}

  inline constexpr bool operator==(MatrixT<n, m, T> const &other) const;
  inline constexpr bool operator!=(MatrixT<n, m, T> const &other) const { return !(*this == other); };

  inline constexpr MatrixT<n, m, T> operator-() const;
  inline constexpr MatrixT<n, m, T> operator+(MatrixT<n, m, T> const &other) const;
  inline constexpr MatrixT<n, m, T> operator-(MatrixT<n, m, T> const &other) const;
  template <AdditiveAutoCasting<T> T2> inline constexpr MatrixT<n, m, T> operator+(T2 const &value) const;
  template <AdditiveAutoCasting<T> T2> inline constexpr MatrixT<n, m, T> operator-(T2 const &value) const;
  template <AdditiveAutoCasting<T> T2>
  inline constexpr friend MatrixT<n, m, T> operator+(T2 const &value, MatrixT<n, m, T> const &matrix) {
    return matrix + value;
  }
  inline MatrixT<n, m, T> &operator+=(MatrixT<n, m, T> const &other);
  template <AdditiveAutoCasting<T> T2> inline MatrixT<n, m, T> &operator+=(T2 const &value);
  inline MatrixT<n, m, T> &operator-=(MatrixT<n, m, T> const &other);
  template <AdditiveAutoCasting<T> T2> inline MatrixT<n, m, T> &operator-=(T2 const &value);
  template <uint8_t l> inline constexpr MatrixT<n, l, T> operator*(MatrixT<m, l, T> const &other) const;
  template <MultiplicativeAutoCasting<T> T2> inline constexpr MatrixT<n, m, T> operator*(T2 const &value) const;
  template <MultiplicativeAutoCasting<T> T2> inline constexpr MatrixT<n, m, T> operator/(T2 const &value) const;
  template <MultiplicativeAutoCasting<T> T2>
  inline constexpr friend MatrixT<n, m, T> operator*(T2 const &value, MatrixT<n, m, T> const &matrix) {
    return matrix * value;
  }
  inline MatrixT<n, m, T> &operator*=(MatrixT<n, m, T> const &other);
  template <MultiplicativeAutoCasting<T> T2> inline MatrixT<n, m, T> &operator*=(T2 const &value);
  template <MultiplicativeAutoCasting<T> T2> inline MatrixT<n, m, T> &operator/=(T2 const &value);
  inline MatrixT<m, n, T> Transposed() const;
  inline MatrixT<n, n, T> Inverse() const
    requires(m == n)
  {
    return MatrixT<n, n, T>(false, data).Invert();
  }
  MatrixT<n, n, T> &Invert()
    requires(m == n);
  inline T maxEntry() const { return *std::max_element(std::begin(data), std::end(data)); }
  inline T minEntry() const { return *std::min_element(std::begin(data), std::end(data)); }

  inline static MatrixT<n, n, T> Identity()
    requires(m == n);
  inline static MatrixT<n, m, T> Zero();
  inline static MatrixT<n, m, T> One();

  template <class TokenIterator>
  friend TokenIterator parse_tokenstream(TokenIterator begin, TokenIterator end, MatrixT<n, m, T> &output);
  template <class OutputIterator>
  friend OutputIterator json_deserialize(MatrixT<n, m, T> const &object, OutputIterator output);

  // +--------------------------------+
  // |    Vector-specific operations  |
  // +--------------------------------+
  inline T operator*(VectorT<n, T> const &other) const
    requires(m == 1)
  {
    return (other.Transposed() * *this)[X];
  }
  inline T SqrMagnitude() const
    requires(m == 1)
  {
    return *this * *this;
  }
  inline T Length() const
    requires(m == 1)
  {
    return std::sqrt(SqrMagnitude());
  }
  inline T &operator[](uint8_t i)
    requires(m == 1)
  {
    return data[i];
  }
  inline T const &operator[](uint8_t i) const
    requires(m == 1)
  {
    return data[i];
  }
  inline VectorT<n, T> Normalized() const
    requires(m == 1)
  {
    return *this / this->Length();
  }
  inline VectorT<n, T> &Normalize()
    requires(m == 1)
  {
    return (*this /= this->Length());
  }
  inline VectorT<3, T> Cross(VectorT<3, T> const &other) const
    requires(m == 1 && n == 3);

  inline T Volume() const
    requires(m == 1);

  // "Properties" for easier access
private:
  class Row {
    MatrixT<n, m, T> &parent;
    uint8_t row;

  public:
    Row(MatrixT<n, m, T> &mat, uint8_t row) : parent(mat), row(row) {}
    inline VectorT<m, T> operator=(VectorT<m, T> const &values) {
      for (uint8_t col = 0; col < m; col++) {
        parent.data[row * m + col] = values[col];
      }
    }

    inline operator VectorT<m, T>() {
      VectorT<m, T> res{};
      for (uint8_t col = 0; col < m; col++) {
        res[col] = parent.data[col];
      }
    }

    inline T &operator[](uint8_t column) { return parent.data[row * m + column]; }
  };

  class ConstRow {
    MatrixT<n, m, T> const &parent;
    uint8_t row;

  public:
    ConstRow(MatrixT<n, m, T> const &mat, uint8_t row) : parent(mat), row(row) {}

    inline operator VectorT<m, T>() const {
      VectorT<m, T> res{};
      for (uint8_t col = 0; col < m; col++) {
        res[col] = parent.data[col];
      }
      return res;
    }

    inline T const &operator[](uint8_t column) const { return parent.data[row * m + column]; }
  };

public:
  inline Row operator[](uint8_t row) { return Row(*this, row); };
  inline ConstRow operator[](uint8_t row) const { return ConstRow(*this, row); };

  template <uint8_t... indexes>
  EntryReference<n, T, indexes...> Entries()
    requires(m == 1)
  {
    return {*this};
  }

  // TODO: Allow value retrieval on const vectors?
  inline constexpr T x() const requires(m == 1){ return data[X]; }
  inline constexpr T y() const requires(m == 1 && n >= 2) { return data[Y]; }
  inline constexpr T z() const requires(m == 1 && n >= 3) { return data[Z]; }
  inline constexpr T w() const requires(m == 1 && n >= 4) { return data[W]; }
  inline constexpr EntryReference<n, T, X> x() requires(m == 1) { return {*this}; }
  inline constexpr EntryReference<n, T, Y> y() requires(m == 1 && n >= 2) { return {*this}; }
  inline constexpr EntryReference<n, T, Z> z() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, W> w() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, Y> xy() requires(m == 1 && n >= 2) { return {*this}; }
  inline constexpr EntryReference<n, T, X, Z> xz() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, X, W> xw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, X> yx() requires(m == 1 && n >= 2) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, Z> yz() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, W> yw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, X> zx() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, Y> zy() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, W> zw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, X> wx() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, Y> wy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, Z> wz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, Y, Z> xyz() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, X, Y, W> xyw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, Z, Y> xzy() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, X, Z, W> xzw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, W, Y> xwy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, W, Z> xwz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, X, Z> yxz() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, X, W> yxw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, Z, X> yzx() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, Z, W> yzw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, W, X> ywx() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, W, Z> ywz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, X, Y> zxy() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, X, W> zxw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, Y, X> zyx() requires(m == 1 && n >= 3) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, Y, W> zyw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, W, X> zwx() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, W, Y> zwy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, X, Y> wxy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, X, Z> wxz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, Y, X> wyx() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, Y, Z> wyz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, Z, X> wzx() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, Z, Y> wzy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, Y, Z, W> xyzw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, Y, W, Z> xywz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, Z, Y, W> xzyw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, Z, W, Y> xzwy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, W, Y, Z> xwyz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, X, W, Z, Y> xwzy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, X, Z, W> yxzw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, X, W, Z> yxwz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, Z, X, W> yzxw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, Z, W, X> yzwx() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, W, X, Z> ywxz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Y, W, Z, X> ywzx() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, X, Y, W> zxyw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, X, W, Y> zxwy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, Y, X, W> zyxw() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, Y, W, X> zywx() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, W, X, Y> zwxy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, Z, W, Y, X> zwyx() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, X, Y, Z> wxyz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, X, Z, Y> wxzy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, Y, X, Z> wyxz() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, Y, Z, X> wyzx() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, Z, X, Y> wzxy() requires(m == 1 && n >= 4) { return {*this}; }
  inline constexpr EntryReference<n, T, W, Z, Y, X> wzyx() requires(m == 1 && n >= 4) { return {*this}; }

private:
  // Adds factor * row1 to row2
  inline void RowOp(uint8_t row1, uint8_t row2, T factor);
  inline void ColOp(uint8_t col1, uint8_t col2, T factor);
  inline void RowSwap(uint8_t row1, uint8_t row2);
  inline void ColSwap(uint8_t col1, uint8_t col2);
  std::array<T, n * m> data;

  // Needed for access of private members of other instance in matrix multiplication
  template <uint8_t n, uint8_t l, typename T> friend struct MatrixT;
  friend struct json<MatrixT<n, m, T>>;
};

// +------------------------+
// |    IMPLEMENTATIONS     |
// +------------------------+

template <uint8_t n, uint8_t m, typename T>
inline MatrixT<n, n, T> &MatrixT<n, m, T>::Invert() // Uses the gaussean algorithm
  requires(m == n)
{
  MatrixT<n, n, T> id = MatrixT<n, n, T>::Identity();
  // Triangularize A
  for (int diag = 0; diag < n; diag++) // Pass along diagonal
  {
    for (int col = diag; col < n; col++) {
      if (data[MATRIX_NM_AT_IJ(diag, col)]) { // Find first column with non-zero entry on diagonal j, swap with column j
        ColSwap(diag, col);
        id.ColSwap(diag, col);
        break;
      }
    }

    if (data[MATRIX_NM_AT_IJ(diag, diag)] == 0) {
      throw "Inverse of irregular matrix requested.";
    }

    // Now the j-th diagonal entry is non-zero

    // Eliminate j-th column
    for (int col = diag + 1; col < n; col++) {
      T factor = -data[MATRIX_NM_AT_IJ(diag, col)] / data[MATRIX_NM_AT_IJ(diag, diag)];
      ColOp(diag, col, factor);
      id.ColOp(diag, col, factor);
    }
  }

  // Empty upper triangle
  for (int diag = n - 1; diag >= 0; diag--) // Go through columns
  {
    // Eliminate j-th column
    for (int col = 0; col < diag; col++) {
      T factor = -data[MATRIX_NM_AT_IJ(diag, col)] / data[MATRIX_NM_AT_IJ(diag, diag)];
      ColOp(diag, col, factor);
      id.ColOp(diag, col, factor);
    }
  }

  // Normalize
  for (int col = 0; col < n; col++) {
    T factor = data[MATRIX_NM_AT_IJ(col, col)];
    for (int row = 0; row < n; row++) {
      data[MATRIX_NM_AT_IJ(row, col)] = id.data[MATRIX_NM_AT_IJ(row, col)] / factor;
    }
  }

  return *this;
}

template <uint8_t n, uint8_t m, typename T>
inline MatrixT<n, n, T> MatrixT<n, m, T>::Identity()
  requires(m == n)
{
  std::array<T, n * m> values = {0};
  for (int diag = 0; diag < n; diag++) {
    values[MATRIX_NM_AT_IJ(diag, diag)] = 1;
  }
  return MatrixT<n, n, T>(values);
}

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T> MatrixT<n, m, T>::Zero() { return {}; }

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T> MatrixT<n, m, T>::One() {
  return MatrixT<n, m, T>::Zero() + 1;
}

template <uint8_t n, uint8_t m, typename T>
inline VectorT<3, T> MatrixT<n, m, T>::Cross(VectorT<3, T> const &other) const
  requires(m == 1 && n == 3)
{
  return VectorT<3, T>{data[Y] * other[Z] - data[Z] * other[Y], data[Z] * other[X] - data[X] * other[Z],
                       data[X] * other[Y] - data[Y] * other[X]};
}

template <uint8_t n, uint8_t m, typename T>
inline T MatrixT<n, m, T>::Volume() const
  requires(m == 1)
{
  T res = data[0];
  for (int i = 1; i < n; i++) {
    res *= data[i];
  }
  return res;
}

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::RowOp(uint8_t row1, uint8_t row2, T factor) {
  for (int col = 0; col < n; col++) {
    data[MATRIX_NM_AT_IJ(row2, col)] += factor * data[MATRIX_NM_AT_IJ(row1, col)];
  }
}

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::ColOp(uint8_t col1, uint8_t col2, T factor) {
  for (int row = 0; row < n; row++) {
    data[MATRIX_NM_AT_IJ(row, col2)] += factor * data[MATRIX_NM_AT_IJ(row, col1)];
  }
}

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::ColSwap(uint8_t col1, uint8_t col2) {
  for (int row = 0; row < n; row++) {
    T temp = data[MATRIX_NM_AT_IJ(row, col1)];
    data[MATRIX_NM_AT_IJ(row, col1)] = data[MATRIX_NM_AT_IJ(row, col2)];
    data[MATRIX_NM_AT_IJ(row, col2)] = temp;
  }
}

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::RowSwap(uint8_t row1, uint8_t row2) {
  for (int col = 0; col < n; col++) {
    T temp = data[MATRIX_NM_AT_IJ(row1, col)];
    data[MATRIX_NM_AT_IJ(row1, col)] = data[MATRIX_NM_AT_IJ(row2, col)];
    data[MATRIX_NM_AT_IJ(row2, col)] = temp;
  }
}

template <uint8_t n, uint8_t m, typename T>
template <uint8_t l>
inline constexpr MatrixT<n, l, T> MatrixT<n, m, T>::operator*(MatrixT<m, l, T> const &other) const {
  std::array<T, n * l> newVals{};
  for (int resCol = 0; resCol < l; resCol++) {
    for (int resRow = 0; resRow < n; resRow++) {
      for (int i = 0; i < m; i++) {
        T d = data[MATRIX_AT_IJ(n, m, resRow, i)];
        T od = other.data[MATRIX_AT_IJ(m, l, i, resCol)];
        newVals[MATRIX_AT_IJ(n, l, resRow, resCol)] +=
            data[MATRIX_AT_IJ(n, m, resRow, i)] * other.data[MATRIX_AT_IJ(m, l, i, resCol)];
      }
    }
  }
  return MatrixT<n, l, T>(false, newVals);
}

template <uint8_t n, uint8_t m, typename T>
inline constexpr bool MatrixT<n, m, T>::operator==(MatrixT<n, m, T> const &other) const {
  for (int row = 0; row < n; row++) {
    for (int col = 0; col < m; col++) {
      if constexpr (std::is_integral<T>::value) {
        if (data[MATRIX_NM_AT_IJ(row, col)] != other.data[MATRIX_NM_AT_IJ(row, col)])
          return false;
      } else {
        if (std::abs(data[MATRIX_NM_AT_IJ(row, col)] - other.data[MATRIX_NM_AT_IJ(row, col)]) > EPS)
          return false;
      }
    }
  }
  return true;
}

template <uint8_t n, uint8_t m, typename T> inline constexpr MatrixT<n, m, T> MatrixT<n, m, T>::operator-() const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = -data[i];
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
inline constexpr MatrixT<n, m, T> MatrixT<n, m, T>::operator+(MatrixT<n, m, T> const &other) const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] + other[i];
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
inline constexpr MatrixT<n, m, T> MatrixT<n, m, T>::operator-(MatrixT<n, m, T> const &other) const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] - other[i];
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
template <AdditiveAutoCasting<T> T2>
inline constexpr MatrixT<n, m, T> MatrixT<n, m, T>::operator+(T2 const &value) const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] + value;
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
template <AdditiveAutoCasting<T> T2>
inline constexpr MatrixT<n, m, T> MatrixT<n, m, T>::operator-(T2 const &value) const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] - value;
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator+=(MatrixT<n, m, T> const &other) {
  for (int i = 0; i < n * m; i++) {
    data[i] += other[i];
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T>
template <AdditiveAutoCasting<T> T2>
inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator+=(T2 const &value) {
  for (int i = 0; i < n * m; i++) {
    data[i] += value;
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T>
inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator-=(MatrixT<n, m, T> const &other) {
  for (int i = 0; i < n * m; i++) {
    data[i] -= other[i];
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T>
template <AdditiveAutoCasting<T> T2>
inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator-=(T2 const &value) {
  for (int i = 0; i < n * m; i++) {
    data[i] -= value;
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T>
inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator*=(MatrixT<n, m, T> const &other) {
  for (int i = 0; i < n * m; i++) {
    data[i] *= other[i];
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T>
template <MultiplicativeAutoCasting<T> T2>
inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator*=(T2 const &value) {
  for (int i = 0; i < n * m; i++) {
    data[i] *= value;
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T>
template <MultiplicativeAutoCasting<T> T2>
inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator/=(T2 const &value) {
  for (int i = 0; i < n * m; i++) {
    data[i] /= value;
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T>
template <MultiplicativeAutoCasting<T> T2>
inline constexpr MatrixT<n, m, T> MatrixT<n, m, T>::operator*(T2 const &value) const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] * value;
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
template <MultiplicativeAutoCasting<T> T2>
inline constexpr MatrixT<n, m, T> MatrixT<n, m, T>::operator/(T2 const &value) const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] / value;
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T> inline MatrixT<m, n, T> MatrixT<n, m, T>::Transposed() const {
  std::array<T, n * m> newVals;
  for (int row = 0; row < n; row++) {
    for (int col = 0; col < m; col++) {
      newVals[MATRIX_AT_IJ(m, n, col, row)] = data[MATRIX_AT_IJ(n, m, row, col)];
    }
  }
  return MatrixT<m, n, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::ConvertToColumnForm() {
  std::array<T, n * m> transposedData;
  for (int row = 0; row < n; row++) {
    for (int col = 0; col < m; col++) {
      transposedData[MATRIX_AT_IJ(n, m, row, col)] =
          data[MATRIX_AT_IJ(m, n, col, row)]; // values are row-wise but data is column-wise
    }
  }
  data = transposedData;
}

} // namespace Engine::Maths

#include <format>
#include <functional>
#include <sstream>

namespace std {
template <uint8_t n, typename T> struct hash<Engine::Maths::VectorT<n, T>> {
  inline size_t operator()(Engine::Maths::VectorT<n, T> const &v) const {
    size_t h = 0;
    for (uint8_t i = 0; i < n; i++) {
      size_t a = hash<uint8_t>{}(i);
      h <<= hash<uint8_t>{}(i) % 4;
      h ^= hash<T>{}(v[i]);
    }
    return h;
  }
};

template <uint8_t n, uint8_t m, typename T> struct hash<Engine::Maths::MatrixT<n, m, T>> {
  inline size_t operator()(Engine::Maths::MatrixT<n, m, T> const &mat) const {
    size_t h = 0;
    for (uint8_t i = 0; i < n; i++) {
      for (uint8_t j = 0; j < m; j++) {
        size_t a = hash<uint8_t>{}(i);
        h <<= hash<uint8_t>{}(i) % 4;
        h ^= hash<T>{}(mat[i][j]);
        h >>= hash<uint8_t>{}(j) % 4;
      }
    }
    return h;
  }
};

template <uint8_t n, typename T> struct formatter<Engine::Maths::VectorT<n, T>> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) const { return ctx.begin(); }

  template <typename FormatContext> auto format(Engine::Maths::VectorT<n, T> const &v, FormatContext &ctx) const {
    std::ostringstream out;
    out << "{";
    for (uint8_t i = 0; i < n; i++) {

      out << v[i];
      if (i < n - 1) {
        out << ", ";
      }
    }
    out << "}";

    return ranges::copy(std::move(out).str(), ctx.out()).out;
  }
};

} // namespace std

template <uint8_t n, uint8_t m, typename T> PARTIALLY_SPECIALIZED_JSON(Engine::Maths::MatrixT<n COMMA m COMMA T>);
TEMPLATED_JSON(TEMPLATE_ARGS(uint8_t n, uint8_t m, typename T), Engine::Maths::MatrixT<TEMPLATE_ARGS(n, m, T)>,
               FIELDS(data));