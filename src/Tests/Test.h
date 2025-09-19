#pragma once

#include "Engine/Debug/Logging.h"
#include <cstdint>
#include <vector>

#define __NAMED_TEST_CASE(Name, Label)                                                                                 \
  class Name : public Test {                                                                                           \
  protected:                                                                                                           \
    void VerifyGroup(uint32_t group, uint32_t &numGroups, bool &pass, std::string &groupLabel) const override;         \
                                                                                                                       \
  public:                                                                                                              \
    Name() : Test(Label) {}                                                                                            \
  };                                                                                                                   \
                                                                                                                       \
  TestSubscription<Name> __TEST_SUB_NAME{};                                                                            \
  void Name::VerifyGroup(uint32_t group, uint32_t &numGroups, bool &pass, std::string &groupLabel) const

#define __TEST_CONCAT(a, b) a##b
#define __TEST_CONCAT_INTERMEDIATE(a, b) __TEST_CONCAT(a, b)
#define __TEST_CASE_NAME __TEST_CONCAT_INTERMEDIATE(Test_, __COUNTER__)
#define __TEST_SUB_NAME __TEST_CONCAT_INTERMEDIATE(Sub_, __COUNTER__)

#define TEST_CASE(Label) __NAMED_TEST_CASE(__TEST_CASE_NAME, Label)

#define SUB_GROUP(Label) if (group == numGroups++ && !(groupLabel = Label).empty())

#define VERIFY(Expression)                                                                                             \
  if (!(ExpressionDecomposition() <=> Expression)) {                                                                   \
    pass = false;                                                                                                      \
  }

#define VERIFY_MEM_EQUAL(A, B, Type)                                                                                   \
  {                                                                                                                    \
    Type const *aPtr = reinterpret_cast<Type const *>(&A);                                                             \
    Type const *bPtr = reinterpret_cast<Type const *>(&B);                                                             \
    for (size_t i = 0; i < std::min(sizeof(decltype(A)), sizeof(decltype(B))) / sizeof(Type); i++) {                   \
      if (aPtr[i] != bPtr[i]) {                                                                                        \
        Debug::Logging::PrintError("Test",                                                                             \
                                   " |   |  Element {} of " #A " is {} but element {} of " #B " is {}! ({}({},0))", i, \
                                   aPtr[i], i, bPtr[i], __FILE__, __LINE__);                                           \
        pass = false;                                                                                                  \
      }                                                                                                                \
    }                                                                                                                  \
  }

using namespace Engine;

class Test {
  const char *label;

protected:
  bool pass;
  virtual void VerifyGroup(uint32_t group, uint32_t &numGroups, bool &pass, std::string &groupLabel) const = 0;

public:
  Test(const char *label) : label(label), pass(true) {}
  virtual ~Test() {}

  bool Verify() {

    uint32_t group = 0;
    uint32_t numGroups;
    uint32_t numPassed = 0;
    std::string groupLabel;

    do {
      numGroups = 0;
      bool pass = true;
      VerifyGroup(group, numGroups, pass, groupLabel);

      if (pass) {
        numPassed++;
      } else {
        Debug::Logging::PrintError("Test", " |   |  '{}' failed!", groupLabel);
        this->pass = false;
      }
    } while (++group < numGroups);

    if (pass) {
      if (numGroups > 0) {
        Debug::Logging::PrintSuccess("Test", " |  '{}' passed: {} out of {} sub-groups passed", label, numPassed,
                                     numGroups);
      } else {
        Debug::Logging::PrintSuccess("Test", " |  '{}' passed", label);
      }

      return true;
    }

    // Fail
    if (numGroups > 0) {
      Debug::Logging::PrintError("Test", " |  '{}' failed: {} out of {} sub-groups failed!", label,
                                 numGroups - numPassed, numGroups);
    } else {
      Debug::Logging::PrintError("Test", " |  '{}' failed!", label);
    }
    return false;
  }
};

inline static std::vector<Test *> allTests = {};

template <typename Test_T> struct TestSubscription {
  TestSubscription() { allTests.push_back(new Test_T()); }
};

int main() {
  Debug::Logging::PrintMessage("Test", "Running all tests...");

  uint32_t passed = 0;

  for (auto &test : allTests) {
    if (test->Verify()) {
      passed++;
    }
    delete test;
  }

  if (passed == allTests.size()) { // Pass
    if (allTests.size() > 0) {
      Debug::Logging::PrintSuccess("Test", "All tests passed: {} out of {} tests passed", passed, allTests.size());
    } else {
      Debug::Logging::PrintSuccess("Test", "All tests passed");
    }

    return 0;
  }

  // Fail

  Debug::Logging::PrintError("Test", "Some tests failed: {} out of {} tests failed!", allTests.size() - passed,
                             allTests.size());
  return 0;
}

#define DECLARE_BINARY_OPERATOR_EVAL(Op, OpName) template <typename Expr_T1, typename Expr_T2> class OpName##Eval;

#define DEFINE_BINARY_OPERATOR_EVAL(Op, OpName, Type)                                                                  \
  template <typename Expr_T1, typename Expr_T2> class OpName##Eval {                                                   \
    Expr_T1 const a;                                                                                                   \
    Expr_T2 const b;                                                                                                   \
    Type const value;                                                                                                  \
                                                                                                                       \
  public:                                                                                                              \
    OpName##Eval(Expr_T1 const &a, Expr_T2 const &b) : a(a), b(b), value(a Op b) {                                     \
      if (!value) {                                                                                                    \
        Debug::Logging::PrintError("Test", " |   |  Condition 'a " #Op " b' evaluated to false with a = {}, b = {}",   \
                                   a, b);                                                                              \
      }                                                                                                                \
    }                                                                                                                  \
    inline operator bool() const { return value; }                                                                     \
                                                                                                                       \
    CREATE_BINARY_OPERATOR_EVALS(Type)                                                                                 \
  };

#define CREATE_BINARY_OPERATOR_EVAL(Op, OpName, Type)                                                                  \
  template <typename Other_T> inline OpName##Eval<Type, Other_T> operator Op(Other_T const &other) const {             \
    return {value, other};                                                                                             \
  }

DECLARE_BINARY_OPERATOR_EVAL(==, Equality)
DECLARE_BINARY_OPERATOR_EVAL(&&, Conjunction)
DECLARE_BINARY_OPERATOR_EVAL(<, LessThan)
DECLARE_BINARY_OPERATOR_EVAL(>, GreaterThan)
DECLARE_BINARY_OPERATOR_EVAL(+, Addition)
DECLARE_BINARY_OPERATOR_EVAL(-, Subtraction)
DECLARE_BINARY_OPERATOR_EVAL(*, Multiplication)
DECLARE_BINARY_OPERATOR_EVAL(/, Division)

#define CREATE_BINARY_OPERATOR_EVALS(Type)                                                                             \
  CREATE_BINARY_OPERATOR_EVAL(==, Equality, Type)                                                                      \
  CREATE_BINARY_OPERATOR_EVAL(&&, Conjunction, Type)                                                                   \
  CREATE_BINARY_OPERATOR_EVAL(<, LessThan, Type)                                                                       \
  CREATE_BINARY_OPERATOR_EVAL(>, GreaterThan, Type)                                                                    \
  CREATE_BINARY_OPERATOR_EVAL(+, Addition, Type)                                                                       \
  CREATE_BINARY_OPERATOR_EVAL(-, Subtraction, Type)                                                                    \
  CREATE_BINARY_OPERATOR_EVAL(*, Multiplication, Type)                                                                 \
  CREATE_BINARY_OPERATOR_EVAL(/, Division, Type)

DEFINE_BINARY_OPERATOR_EVAL(==, Equality, bool)
DEFINE_BINARY_OPERATOR_EVAL(&&, Conjunction, bool)
DEFINE_BINARY_OPERATOR_EVAL(<, LessThan, bool)
DEFINE_BINARY_OPERATOR_EVAL(>, GreaterThan, bool)
DEFINE_BINARY_OPERATOR_EVAL(+, Addition, Expr_T1)
DEFINE_BINARY_OPERATOR_EVAL(-, Subtraction, Expr_T1)
DEFINE_BINARY_OPERATOR_EVAL(*, Multiplication, Expr_T1)
DEFINE_BINARY_OPERATOR_EVAL(/, Division, Expr_T1)

template <typename T> class TypeDecomposition {
  T const value;

public:
  TypeDecomposition(T const &value) : value(value) {}

  CREATE_BINARY_OPERATOR_EVALS(T)
};

class ExpressionDecomposition {
public:
  template <typename T> inline TypeDecomposition<T> operator<=>(T const &value) const { return {value}; }
};