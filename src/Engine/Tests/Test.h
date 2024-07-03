#pragma once

#include "Engine/Debug/Logging.h"
#include "Engine/Util/Macros.h"
#include <cmath>
#include <sstream>

#define EQUALITY_EPS 0.0001

#define BEGIN_TEST_CASE(label)                                                                                         \
  bool run_##label##_tests() {                                                                                         \
    TestEnv env = TestEnv(#label);

#define END_TEST_CASE()                                                                                                \
  return env.Passed();                                                                                                 \
  }

#define RUN_SUB_CASE(label)                                                                                            \
  if (!run_##label##_tests()) {                                                                                        \
    env.Fail();                                                                                                        \
  }

#define TEST_ASSERT(expression, message, ...)                                                                          \
  if (!(expression)) {                                                                                                 \
    env.Fail();                                                                                                        \
    Debug::Logging::PrintError("Test", message, __VA_ARGS__);                                                          \
  }

#define TEST_ASSERT_EQUAL(T, object1, label1, object2, label2, message)                                                \
  size_t VAR_WITH_LINE(size) = std::min(sizeof(object1), sizeof(object2)) / sizeof(float);                             \
  if (!test_T_ptr_equal<T>((T *)&object1, (T *)&object2, VAR_WITH_LINE(size))) {                                       \
    env.Fail();                                                                                                        \
    Debug::Logging::PrintError("Test", message);                                                                       \
    Debug::Logging::PrintError("Test", "{}", format_single_T_ptr((T *)&object1, label1, VAR_WITH_LINE(size)));         \
    Debug::Logging::PrintError("Test", "{}", format_single_T_ptr((T *)&object2, label2, VAR_WITH_LINE(size)));         \
  }

namespace Engine::Test {

class TestEnv {
  const char *label;
  bool passed;

public:
  TestEnv(const char *lbl) : label(lbl), passed(true) {
    Debug::Logging::PrintMessage("Test", "Running {} tests...", label);
  }
  ~TestEnv() {
    if (passed) {
      Debug::Logging::PrintSuccess("Test", "Passed all {} tests!", label);
    } else {
      Debug::Logging::PrintError("Test", "");
      if (strcmp(label, "all")) // TODO: Find more elegant solution
        Debug::Logging::PrintError("Test", "Some {} tests failed!", label);
      else
        Debug::Logging::PrintError("Test", "Some tests failed!");
    }
  }

  void Fail() { passed = false; }
  bool Passed() { return passed; }
};

template <typename T> bool test_T_ptr_equal(T const *p1, T const *p2, uint16_t numberOfFloats) {
  for (int i = 0; i < numberOfFloats; i++) {
    if (abs(p1[i] - p2[i]) > 0.0001f) {
      return false;
    }
  }
  return true;
}

template <typename T> std::string format_single_T_ptr(T const *ptr, const char *lbl, uint16_t numberOfT) {
  std::stringstream builder;

  builder << std::fixed;
  builder.precision(6);

  for (int i = 0; i < numberOfT; i++) {
    float val = ptr[i];
    if (!std::signbit(val)) {
      builder << " ";
    }
    builder << val << " ";
  }
  builder << " (" << lbl << ")";

  return builder.str();
}

} // namespace Engine::Test