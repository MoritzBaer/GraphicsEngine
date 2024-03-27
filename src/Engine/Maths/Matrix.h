#pragma once

#include <cmath>
#include <initializer_list>
#include <sstream>
#include <stdint.h>

#define PI 3.14159265359
#define EPS 0.0000001

inline const uint8_t X = 0;
inline const uint8_t Y = 1;
inline const uint8_t Z = 2;
inline const uint8_t W = 3;

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

// Saved in row form (n x m means m columns, n rows)
template <uint8_t n, uint8_t m, typename T> struct MatrixT {
public:
  MatrixT<n, m, T>(T const values[n * m]) {
    for (int row = 0; row < n; row++) {
      for (int col = 0; col < m; col++) {
        data[row * m + col] = values[row * m + col];
      }
    }
  };
  MatrixT<n, m, T>(std::initializer_list<T> values);
  MatrixT<n, m, T>() : data() {}

  inline bool operator==(MatrixT<n, m, T> const &other) const;
  inline bool operator!=(MatrixT<n, m, T> const &other) const { return !(*this == other); };

  inline MatrixT<n, m, T> operator+(MatrixT<n, m, T> const &other) const;
  inline MatrixT<n, m, T> operator-(MatrixT<n, m, T> const &other) const;
  inline MatrixT<n, m, T> operator+(T value) const;
  inline MatrixT<n, m, T> operator-(T value) const;
  inline friend MatrixT<n, m, T> operator+(T value, MatrixT<n, m, T> const &matrix) { return matrix + value; }
  inline MatrixT<n, m, T> &operator+=(MatrixT<n, m, T> const &other);
  inline MatrixT<n, m, T> &operator+=(T value);
  inline MatrixT<n, m, T> &operator-=(MatrixT<n, m, T> const &other);
  inline MatrixT<n, m, T> &operator-=(T value);
  template <uint8_t l> inline MatrixT<n, l, T> operator*(MatrixT<m, l, T> const &other) const;
  inline MatrixT<n, m, T> operator*(T value) const;
  inline MatrixT<n, m, T> operator/(T value) const;
  inline friend MatrixT<n, m, T> operator*(T value, MatrixT<n, m, T> const &matrix) { return matrix * value; }
  inline MatrixT<n, m, T> &operator*=(MatrixT<n, m, T> const &other);
  inline MatrixT<n, m, T> &operator*=(T value);
  inline MatrixT<n, m, T> &operator/=(T value);
  inline MatrixT<m, n, T> Transposed() const;
  inline MatrixT<n, n, T> Inverse() const
    requires(m == n)
  {
    return MatrixT<n, n, T>(data).Invert();
  }
  MatrixT<n, n, T> &Invert()
    requires(m == n);

  inline static MatrixT<n, n, T> Identity()
    requires(m == n);
  inline static MatrixT<n, m, T> Zero();
  inline static MatrixT<n, m, T> One();

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

  class Entry {
    MatrixT<n, 1, T> &parent;
    uint8_t index;

  public:
    Entry(MatrixT<n, 1, T> &v, uint8_t i) : parent(v), index(i) {}
    inline T &operator=(T value) { return parent.data[index] = value; }
    inline T &operator+=(T value) { return parent.data[index] += value; }
    inline T &operator-=(T value) { return parent.data[index] -= value; }
    inline T &operator*=(T value) { return parent.data[index] *= value; }
    inline T &operator/=(T value) { return parent.data[index] /= value; }
    inline operator T() const { return parent.data[index]; }
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
    EntryPair(MatrixT<n, 1, T> &v, uint8_t i1, uint8_t i2) : parent(v), index1(i1), index2(i2) {}
    inline void operator=(MatrixT<2, 1, T> const &values) {
      parent.data[index1] = values[X];
      parent.data[index2] = values[Y];
    }
    inline void operator+=(MatrixT<2, 1, T> const &values) {
      parent.data[index1] += values[X];
      parent.data[index2] += values[Y];
    }
    inline void operator+=(T value) {
      parent.data[index1] += value;
      parent.data[index2] += value;
    }
    inline void operator-=(MatrixT<2, 1, T> const &values) {
      parent.data[index1] -= values[X];
      parent.data[index2] -= values[Y];
    }
    inline void operator-=(T value) {
      parent.data[index1] += value;
      parent.data[index2] -= value;
    }
    inline void operator*=(T value) {
      parent.data[index1] *= value;
      parent.data[index2] *= value;
    }
    inline void operator/=(T value) {
      parent.data[index1] /= value;
      parent.data[index2] /= value;
    }
    inline operator MatrixT<2, 1, T>() const { return MatrixT<2, 1, T>({parent.data[index1], parent.data[index2]}); }
  };

  class EntryTriplet {
    MatrixT<n, 1, T> &parent;
    uint8_t index1;
    uint8_t index2;
    uint8_t index3;

  public:
    EntryTriplet(MatrixT<n, 1, T> &v, uint8_t i1, uint8_t i2, uint8_t i3)
        : parent(v), index1(i1), index2(i2), index3(i3) {}
    inline void operator=(MatrixT<3, 1, T> const &values) {
      parent.data[index1] = values[X];
      parent.data[index2] = values[Y];
      parent.data[index3] = values[Z];
    }
    inline void operator+=(MatrixT<3, 1, T> const &values) {
      parent.data[index1] += values[X];
      parent.data[index2] += values[Y];
      parent.data[index3] += values[Z];
    }
    inline void operator+=(T value) {
      parent.data[index1] += value;
      parent.data[index2] += value;
      parent.data[index3] += value;
    }
    inline void operator-=(MatrixT<3, 1, T> const &values) {
      parent.data[index1] -= values[X];
      parent.data[index2] -= values[Y];
      parent.data[index3] -= values[Z];
    }
    inline void operator-=(T value) {
      parent.data[index1] += value;
      parent.data[index2] -= value;
      parent.data[index3] -= value;
    }
    inline void operator*=(T value) {
      parent.data[index1] *= value;
      parent.data[index2] *= value;
      parent.data[index3] *= value;
    }
    inline void operator/=(T value) {
      parent.data[index1] /= value;
      parent.data[index2] /= value;
      parent.data[index3] /= value;
    }
    inline operator MatrixT<3, 1, T>() const {
      return MatrixT<3, 1, T>{parent.data[index1], parent.data[index2], parent.data[index3]};
    }
  };

  class EntryQuadruple {
    MatrixT<n, 1, T> &parent;
    uint8_t index1;
    uint8_t index2;
    uint8_t index3;
    uint8_t index4;

  public:
    EntryQuadruple(MatrixT<n, 1, T> &v, uint8_t i1, uint8_t i2, uint8_t i3, uint8_t i4)
        : parent(v), index1(i1), index2(i2), index3(i3), index4(i4) {}
    inline void operator=(MatrixT<4, 1, T> const &values) {
      parent.data[index1] = values[X];
      parent.data[index2] = values[Y];
      parent.data[index3] = values[Z];
      parent.data[index4] = values[W];
    }
    inline void operator+=(MatrixT<4, 1, T> const &values) {
      parent.data[index1] += values[X];
      parent.data[index2] += values[Y];
      parent.data[index3] += values[Z];
      parent.data[index4] += values[W];
    }
    inline void operator+=(T value) {
      parent.data[index1] += value;
      parent.data[index2] += value;
      parent.data[index3] += value;
      parent.data[index4] += value;
    }
    inline void operator-=(MatrixT<4, 1, T> const &values) {
      parent.data[index1] -= values[X];
      parent.data[index2] -= values[Y];
      parent.data[index3] -= values[Z];
      parent.data[index4] -= values[W];
    }
    inline void operator-=(T value) {
      parent.data[index1] += value;
      parent.data[index2] -= value;
      parent.data[index3] -= value;
      parent.data[index4] -= value;
    }
    inline void operator*=(T value) {
      parent.data[index1] *= value;
      parent.data[index2] *= value;
      parent.data[index3] *= value;
      parent.data[index4] *= value;
    }
    inline void operator/=(T value) {
      parent.data[index1] /= value;
      parent.data[index2] /= value;
      parent.data[index3] /= value;
      parent.data[index4] /= value;
    }
    inline operator MatrixT<4, 1, T>() const {
      return MatrixT<4, 1, T>({parent.data[index1], parent.data[index2], parent.data[index3], parent.data[index4]});
    }
  };

public:
  inline Row operator[](uint8_t row) { return Row(*this, row); };

  Entry x()
    requires(m == 1)
  {
    return Entry(*this, X);
  }
  Entry y()
    requires(m == 1 && n >= 2)
  {
    return Entry(*this, Y);
  }
  Entry z()
    requires(m == 1 && n >= 3)
  {
    return Entry(*this, Z);
  }
  Entry w()
    requires(m == 1 && n >= 4)
  {
    return Entry(*this, W);
  }

  const Entry &x() const
    requires(m == 1)
  {
    return Entry(*this, X);
  }
  const Entry &y() const
    requires(m == 1 && n >= 2)
  {
    return Entry(*this, Y);
  }
  const Entry &z() const
    requires(m == 1 && n >= 3)
  {
    return Entry(*this, Z);
  }
  const Entry &w() const
    requires(m == 1 && n >= 4)
  {
    return Entry(*this, W);
  }

  EntryPair xy()
    requires(m == 1 && n >= 2)
  {
    return EntryPair(*this, X, Y);
  }
  EntryPair xz()
    requires(m == 1 && n >= 3)
  {
    return EntryPair(*this, X, Z);
  }
  EntryPair yz()
    requires(m == 1 && n >= 3)
  {
    return EntryPair(*this, Y, Z);
  }

  EntryTriplet xyz()
    requires(m == 1 && n >= 3)
  {
    return EntryTriplet(*this, X, Y, Z);
  }

  EntryQuadruple xyzw()
    requires(m == 1 && n >= 4)
  {
    return EntryQuadruple(*this, X, Y, Z, W);
  }

  // Matrix cannot Serializable because otherwise the Serializable* offsets the data by 64bit,
  // which breaks GPU communication.
  inline void Serialize(std::stringstream &stream) const;

private:
  // Adds factor * row1 to row2
  inline void RowOp(uint8_t row1, uint8_t row2, T factor);
  inline void ColOp(uint8_t col1, uint8_t col2, T factor);
  inline void ColSwap(uint8_t col1, uint8_t col2);
  inline void RowSwap(uint8_t row1, uint8_t row2);
  T data[n * m];

  // Needed for access of private members of other instance in matrix multiplication
  template <uint8_t n, uint8_t l, typename T> friend class MatrixT;
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
  for (int diag = 0; diag < n; diag++) // Go through rows
  {
    for (int row = diag; row < n; row++) {
      if (data[row * m + diag]) { // Find first row with non-zero entry on column j, swap to row j
        RowSwap(diag, row);
        id.RowSwap(diag, row);
        break;
      }
    }

    if (data[diag * m + diag] == 0) {
      throw "Inverse of irregular matrix requested.";
    }

    // Now the j-th diagonal entry is non-zero

    // Eliminate j-th column
    for (int row = diag + 1; row < n; row++) {
      T factor = -data[row * m + diag] / data[diag * m + diag];
      RowOp(diag, row, factor);
      id.RowOp(diag, row, factor);
    }
  }

  // Empty upper triangle
  for (int diag = n - 1; diag >= 0; diag--) // Go through columns
  {
    // Eliminate j-th row
    for (int row = 0; row < diag; row++) {
      T factor = -data[row * m + diag] / data[diag * m + diag];
      RowOp(diag, row, factor);
      id.RowOp(diag, row, factor);
    }
  }

  // Normalize
  for (int row = 0; row < n; row++) {
    T factor = data[row * n + row];
    for (int col = 0; col < n; col++) {
      data[row * n + col] = id.data[row * n + col] / data[row * n + row];
    }
  }

  return *this;
}

template <uint8_t n, uint8_t m, typename T>
inline MatrixT<n, n, T> MatrixT<n, m, T>::Identity()
  requires(m == n)
{
  T values[n * n] = {0};
  for (int diag = 0; diag < n; diag++) {
    values[diag * n + diag] = 1;
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

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::Serialize(std::stringstream &stream) const {
  stream << "{ ";
  for (int row = 0; row < n; row++) {
    for (int col = 0; col < m; col++) {
      stream << data[row * m + col] << " ";
    }
  }
  stream << "}";
}

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::RowOp(uint8_t row1, uint8_t row2, T factor) {
  for (int col = 0; col < n; col++) {
    data[row2 * m + col] += factor * data[row1 * m + col];
  }
}

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::ColOp(uint8_t col1, uint8_t col2, T factor) {
  for (int row = 0; row < n; row++) {
    data[row * m + col2] += factor * data[row * m + col1];
  }
}

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::ColSwap(uint8_t col1, uint8_t col2) {
  for (int row = 0; row < n; row++) {
    T temp = data[row * m + col2];
    data[row * m + col2] = data[row * m + col1];
    data[row * m + col1] = temp;
  }
}

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::RowSwap(uint8_t row1, uint8_t row2) {
  for (int col = 0; col < n; col++) {
    T temp = data[row1 * m + col];
    data[row1 * m + col] = data[row2 * m + col];
    data[row2 * m + col] = temp;
  }
}

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T>::MatrixT(std::initializer_list<T> values) {
  int size = values.size();
  if (n * m < size)
    size = n * m;
  auto arg = values.begin();
  for (int i = 0; i < size; i++) {
    this->data[i] = *arg;
    arg++;
  }
  for (int i = size; i < n * m; i++) {
    this->data[i] = 0;
  }
}

template <uint8_t n, uint8_t m, typename T>
template <uint8_t l>
inline MatrixT<n, l, T> MatrixT<n, m, T>::operator*(MatrixT<m, l, T> const &other) const {
  T newVals[n * l];
  for (int resRow = 0; resRow < n; resRow++) {
    for (int resCol = 0; resCol < l; resCol++) {
      newVals[resRow * l + resCol] = 0;
      for (int i = 0; i < m; i++) {
        newVals[resRow * l + resCol] += data[resRow * m + i] * other.data[i * l + resCol];
      }
    }
  }
  return MatrixT<n, l, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
inline bool MatrixT<n, m, T>::operator==(MatrixT<n, m, T> const &other) const {
  for (int row = 0; row < n; row++) {
    for (int col = 0; col < m; col++) {
      if (abs(data[row * m + col] - other.data[row * m + col]) > EPS)
        return false;
    }
  }
  return true;
}

template <uint8_t n, uint8_t m, typename T>
inline MatrixT<n, m, T> MatrixT<n, m, T>::operator+(MatrixT<n, m, T> const &other) const {
  T newVals[n * m];
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] + other[i];
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
inline MatrixT<n, m, T> MatrixT<n, m, T>::operator-(MatrixT<n, m, T> const &other) const {
  T newVals[n * m];
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] - other[i];
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T> MatrixT<n, m, T>::operator+(T value) const {
  T newVals[n * m];
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] + value;
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T> MatrixT<n, m, T>::operator-(T value) const {
  T newVals[n * m];
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

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator+=(T value) {
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

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator-=(T value) {
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

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator*=(T value) {
  for (int i = 0; i < n * m; i++) {
    data[i] *= value;
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator/=(T value) {
  for (int i = 0; i < n * m; i++) {
    data[i] /= value;
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T> MatrixT<n, m, T>::operator*(T value) const {
  T newVals[n * m];
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] * value;
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T> inline MatrixT<n, m, T> MatrixT<n, m, T>::operator/(T value) const {
  T newVals[n * m];
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] / value;
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T> inline MatrixT<m, n, T> MatrixT<n, m, T>::Transposed() const {
  T newVals[m * n];
  for (int row = 0; row < n; row++) {
    for (int col = 0; col < m; col++) {
      newVals[col * n + row] = data[row * m + col];
    }
  }
  return MatrixT<m, n, T>(newVals);
}

} // namespace Engine::Maths

#include <functional>
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
} // namespace std