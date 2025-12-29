#pragma once

#include "Debug/Profiling.h"
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <type_traits>

namespace Engine::Maths {

template<std::integral T> inline constexpr T Factorial(T const & n) {
  T res = 1;
  for (int i = 1; i <= n; i++) {
    res *= i;
  }
  return res;
}

template <std::integral T, size_t n> class PermutationIterator {
  std::array<T, n> sigma;
  size_t firstIncrease;
  size_t numberOfInversions;

  inline constexpr void Advance();
  PermutationIterator(Engine::Maths::PermutationIterator<T, n> const &other)
      : sigma(other.sigma), firstIncrease(other.firstIncrease), numberOfInversions(other.numberOfInversions) {}

public:
  PermutationIterator() : sigma(), firstIncrease(0), numberOfInversions(0) {
    std::iota(std::begin(sigma), std::end(sigma), 0);
  }

  inline constexpr T operator[](T const &i) const { return sigma[i]; }

  inline constexpr size_t NumberOfInversions() const { return numberOfInversions; }
  inline constexpr int8_t Sign() const { return numberOfInversions % 2 ? -1 : 1; }

  inline constexpr PermutationIterator &operator++(int);

  inline constexpr PermutationIterator operator++() {
    auto it = {*this};
    ++*this;
    return it;
  }

  inline bool HasValidSuccessor() { return firstIncrease < n - 1; }
};

template <std::integral T, size_t n>
inline constexpr PermutationIterator<T, n> &PermutationIterator<T, n>::operator++(int) {
  if (firstIncrease == 0) { // No inversion-prefix
    std::swap(sigma[0], sigma[1]);
    numberOfInversions++; // The swap creates an inversion between sigma[0] and sigma[1]
    while (++firstIncrease < n - 1 && sigma[firstIncrease] > sigma[firstIncrease + 1])
      ;
    return {*this};
  }

  size_t swapIndex = -1; // The index of the largest element before and less than sigma[firstIncrease + 1]

  // TODO: Maybe measure for which values of n a linear scan exploiting cache coherence is faster than bin search,
  // compile differently for those values.
  if (sigma[firstIncrease + 1] > sigma[0]) { // Skip search
    swapIndex = 0;
  } else { // Search index (bin search implementation could never find 0)

    size_t fst = 0;
    size_t lst = firstIncrease;
    // Invariant: sigma[fst] > sigma[firstIncrease + 1], sigma[lst] < sigma[firstIncrease + 1].
    while (lst - fst > 1) {
      // Since lst - fst > 1, fst + lst >= 2fst + 2, so mid != fst and the invariant is maintained
      const size_t mid = (fst + lst) / 2;
      if (sigma[mid] < sigma[firstIncrease + 1]) {
        lst = mid;
      } else { // sigma[mid] < sigma[firstIncrease + 1] since elements are unique
        fst = mid;
      }
    }
    // Now sigma[lst] is the largest element before and less than sigma[firstIncrease + 1].

    swapIndex = lst;
  }

  std::swap(sigma[swapIndex], sigma[firstIncrease + 1]);
  numberOfInversions++; // The swap creates an inversion between sigma[swapIndex] and sigma[firstIncrease + 1]

  std::reverse(std::begin(sigma), std::begin(sigma) + firstIncrease + 1);
  numberOfInversions -= (firstIncrease + 1) * firstIncrease / 2; // All inversions of the reversed part are removed

  firstIncrease = 0;

  return {*this};
}
} // namespace Engine::Maths

#include <format>

namespace std {

template <std::integral T, size_t n> struct formatter<Engine::Maths::PermutationIterator<T, n>> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) const { return ctx.begin(); }

  template <typename FormatContext>
  auto format(Engine::Maths::PermutationIterator<T, n> const &sigma, FormatContext &ctx) const {
    std::ostringstream out;
    out << "[";
    for (uint8_t i = 0; i < n; i++) {

      out << sigma[i];
      if (i < n - 1) {
        out << ", ";
      }
    }
    out << "]";

    return ranges::copy(std::move(out).str(), ctx.out()).out;
  }
};
} // namespace std