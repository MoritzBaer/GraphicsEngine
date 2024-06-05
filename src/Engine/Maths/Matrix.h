#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <sstream>
#include <stdint.h>

#define PI 3.14159265359
#define EPS 0.0000001

#define MATRIX_AT_IJ(n, m, row, col) col *n + row
#define MATRIX_NM_AT_IJ(row, col) MATRIX_AT_IJ(n, m, row, col)

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

// Saved in column form (n x m means m columns, n rows)
template <uint8_t n, uint8_t m, typename T> struct MatrixT {
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

public:
  // Public constructors always expect data in row form
  MatrixT(std::array<T, n * m> const &values) : MatrixT(true, values) {}
  // Public constructors always expect data in row form
  template <typename... _T, typename std::enable_if<sizeof...(_T) == n * m, int>::type = 0>
  MatrixT(_T... values) : MatrixT(true, values...) {}
  MatrixT() : data() {}

  inline bool operator==(MatrixT<n, m, T> const &other) const;
  inline bool operator!=(MatrixT<n, m, T> const &other) const { return !(*this == other); };

  inline MatrixT<n, m, T> operator+(MatrixT<n, m, T> const &other) const;
  inline MatrixT<n, m, T> operator-(MatrixT<n, m, T> const &other) const;
  template <typename T2> inline MatrixT<n, m, T> operator+(T2 const &value) const;
  template <typename T2> inline MatrixT<n, m, T> operator-(T2 const &value) const;
  template <typename T2> inline friend MatrixT<n, m, T> operator+(T2 const &value, MatrixT<n, m, T> const &matrix) {
    return matrix + value;
  }
  inline MatrixT<n, m, T> &operator+=(MatrixT<n, m, T> const &other);
  template <typename T2> inline MatrixT<n, m, T> &operator+=(T2 const &value);
  inline MatrixT<n, m, T> &operator-=(MatrixT<n, m, T> const &other);
  template <typename T2> inline MatrixT<n, m, T> &operator-=(T2 const &value);
  template <uint8_t l> inline MatrixT<n, l, T> operator*(MatrixT<m, l, T> const &other) const;
  template <typename T2> inline MatrixT<n, m, T> operator*(T2 const &value) const;
  template <typename T2> inline MatrixT<n, m, T> operator/(T2 const &value) const;
  template <typename T2> inline friend MatrixT<n, m, T> operator*(T2 const &value, MatrixT<n, m, T> const &matrix) {
    return matrix * value;
  }
  inline MatrixT<n, m, T> &operator*=(MatrixT<n, m, T> const &other);
  template <typename T2> inline MatrixT<n, m, T> &operator*=(T2 const &value);
  template <typename T2> inline MatrixT<n, m, T> &operator/=(T2 const &value);
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
    inline T &operator=(T const &value) { return parent.data[index] = value; }
    template <typename T2> inline T &operator+=(T2 const &value) { return parent.data[index] += value; }
    template <typename T2> inline T &operator-=(T2 const &value) { return parent.data[index] -= value; }
    template <typename T2> inline T &operator*=(T2 const &value) { return parent.data[index] *= value; }
    template <typename T2> inline T &operator/=(T2 const &value) { return parent.data[index] /= value; }
    inline operator T() const { return parent.data[index]; }
    template <typename T2> inline T operator+(T2 const &value) const { return parent.data[index] + value; }
    template <typename T2> inline T operator-(T2 const &value) const { return parent.data[index] - value; }
    template <typename T2> inline T operator*(T2 const &value) const { return parent.data[index] * value; }
    template <typename T2> inline T operator/(T2 const &value) const { return parent.data[index] / value; }
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
    template <typename T2> inline void operator+=(T2 const &value) {
      parent.data[index1] += value;
      parent.data[index2] += value;
    }
    inline void operator-=(MatrixT<2, 1, T> const &values) {
      parent.data[index1] -= values[X];
      parent.data[index2] -= values[Y];
    }
    template <typename T2> inline void operator-=(T2 const &value) {
      parent.data[index1] += value;
      parent.data[index2] -= value;
    }
    template <typename T2> inline void operator*=(T2 const &value) {
      parent.data[index1] *= value;
      parent.data[index2] *= value;
    }
    template <typename T2> inline void operator/=(T2 const &value) {
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
    template <typename T2> inline void operator+=(T2 const &value) {
      parent.data[index1] += value;
      parent.data[index2] += value;
      parent.data[index3] += value;
    }
    inline void operator-=(MatrixT<3, 1, T> const &values) {
      parent.data[index1] -= values[X];
      parent.data[index2] -= values[Y];
      parent.data[index3] -= values[Z];
    }
    template <typename T2> inline void operator-=(T2 const &value) {
      parent.data[index1] += value;
      parent.data[index2] -= value;
      parent.data[index3] -= value;
    }
    template <typename T2> inline void operator*=(T2 const &value) {
      parent.data[index1] *= value;
      parent.data[index2] *= value;
      parent.data[index3] *= value;
    }
    template <typename T2> inline void operator/=(T2 const &value) {
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
    template <typename T2> inline void operator+=(T2 const &value) {
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
    template <typename T2> inline void operator-=(T2 const &value) {
      parent.data[index1] += value;
      parent.data[index2] -= value;
      parent.data[index3] -= value;
      parent.data[index4] -= value;
    }
    template <typename T2> inline void operator*=(T2 const &value) {
      parent.data[index1] *= value;
      parent.data[index2] *= value;
      parent.data[index3] *= value;
      parent.data[index4] *= value;
    }
    template <typename T2> inline void operator/=(T2 const &value) {
      parent.data[index1] /= value;
      parent.data[index2] /= value;
      parent.data[index3] /= value;
      parent.data[index4] /= value;
    }
    inline operator MatrixT<4, 1, T>() const {
      return MatrixT<4, 1, T>(parent.data[index1], parent.data[index2], parent.data[index3], parent.data[index4]);
    }
  };

public:
  inline Row operator[](uint8_t row) { return Row(*this, row); };

  // TODO: Allow value retrieval on const vectors?
  inline T x() const { return data[X]; }
  inline T y() const { return data[Y]; }
  inline T z() const { return data[Z]; }
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
  inline void RowSwap(uint8_t row1, uint8_t row2);
  inline void ColSwap(uint8_t col1, uint8_t col2);
  std::array<T, n * m> data;

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

template <uint8_t n, uint8_t m, typename T> inline void MatrixT<n, m, T>::Serialize(std::stringstream &stream) const {
  stream << "{ ";
  for (int row = 0; row < n; row++) {
    for (int col = 0; col < m; col++) {
      stream << data[MATRIX_NM_AT_IJ(row, col)] << " ";
    }
  }
  stream << "}";
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
inline MatrixT<n, l, T> MatrixT<n, m, T>::operator*(MatrixT<m, l, T> const &other) const {
  std::array<T, n * l> newVals{};
  for (int resCol = 0; resCol < l; resCol++) {
    for (int resRow = 0; resRow < n; resRow++) {
      for (int i = 0; i < m; i++) {
        newVals[MATRIX_AT_IJ(n, l, resRow, resCol)] +=
            data[MATRIX_AT_IJ(n, m, resRow, i)] * other.data[MATRIX_AT_IJ(m, l, i, resCol)];
      }
    }
  }
  return MatrixT<n, l, T>(false, newVals);
}

template <uint8_t n, uint8_t m, typename T>
inline bool MatrixT<n, m, T>::operator==(MatrixT<n, m, T> const &other) const {
  for (int row = 0; row < n; row++) {
    for (int col = 0; col < m; col++) {
      if (abs(data[MATRIX_NM_AT_IJ(row, col)] - other.data[MATRIX_NM_AT_IJ(row, col)]) > EPS)
        return false;
    }
  }
  return true;
}

template <uint8_t n, uint8_t m, typename T>
inline MatrixT<n, m, T> MatrixT<n, m, T>::operator+(MatrixT<n, m, T> const &other) const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] + other[i];
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
inline MatrixT<n, m, T> MatrixT<n, m, T>::operator-(MatrixT<n, m, T> const &other) const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] - other[i];
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
template <typename T2>
inline MatrixT<n, m, T> MatrixT<n, m, T>::operator+(T2 const &value) const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] + value;
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
template <typename T2>
inline MatrixT<n, m, T> MatrixT<n, m, T>::operator-(T2 const &value) const {
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
template <typename T2>
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
template <typename T2>
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
template <typename T2>
inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator*=(T2 const &value) {
  for (int i = 0; i < n * m; i++) {
    data[i] *= value;
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T>
template <typename T2>
inline MatrixT<n, m, T> &MatrixT<n, m, T>::operator/=(T2 const &value) {
  for (int i = 0; i < n * m; i++) {
    data[i] /= value;
  }
  return *this;
}

template <uint8_t n, uint8_t m, typename T>
template <typename T2>
inline MatrixT<n, m, T> MatrixT<n, m, T>::operator*(T2 const &value) const {
  std::array<T, n * m> newVals;
  for (int i = 0; i < n * m; i++) {
    newVals[i] = data[i] * value;
  }
  return MatrixT<n, m, T>(newVals);
}

template <uint8_t n, uint8_t m, typename T>
template <typename T2>
inline MatrixT<n, m, T> MatrixT<n, m, T>::operator/(T2 const &value) const {
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