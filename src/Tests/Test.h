#pragma once

#include "Engine/Debug/Logging.h"
#include "pp-foreach.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <format>
#include <stdexcept>
#include <stdint.h>
#include <string>
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

#define __CURRENT_FILE_NAME__ (&__FILE__[SOURCE_PATH_SIZE])

#define VERIFY(Expression)                                                                                             \
  {                                                                                                                    \
    auto decomposition = Decompose(#Expression, sizeof(#Expression) - 1)->*Expression;                                 \
    if (!decomposition) {                                                                                              \
      pass = false;                                                                                                    \
      testLogger.PrintError("Verification failed! ({}({},0))", __CURRENT_FILE_NAME__, __LINE__);                               \
      testLogger.PushSection();                                                                                        \
      DepthPrinter<decltype(decomposition), ExpressionDepth<decltype(decomposition)>::value + 1>{}(decomposition,      \
                                                                                                   testLogger);        \
      testLogger.PopSection();                                                                                         \
    }                                                                                                                  \
  }

#define VERIFY_MEM_EQUAL(A, B, Type)                                                                                   \
  {                                                                                                                    \
    Type const *aPtr = reinterpret_cast<Type const *>(&A);                                                             \
    Type const *bPtr = reinterpret_cast<Type const *>(&B);                                                             \
    for (size_t i = 0; i < std::min(sizeof(decltype(A)), sizeof(decltype(B))) / sizeof(Type); i++) {                   \
      if (aPtr[i] != bPtr[i]) {                                                                                        \
        testLogger.PrintError("Element {} of " #A " is {} but element {} of " #B " is {}! ({}({},0))", i, aPtr[i], i,  \
                              bPtr[i], __FILE__, __LINE__);                                                            \
        pass = false;                                                                                                  \
      }                                                                                                                \
    }                                                                                                                  \
  }

using namespace Engine;

inline Debug::Logging::Logger testLogger("Test");

class Test {
  const char *label;

protected:
  bool pass;
  virtual void VerifyGroup(uint32_t group, uint32_t &numGroups, bool &pass, std::string &groupLabel) const = 0;

public:
  Test(const char *label) : label(label), pass(true) {}
  virtual ~Test() {}

  bool Verify() {

    uint32_t group = -1;
    uint32_t numGroups = 0;
    uint32_t numPassed = 0;
    std::string groupLabel;

    do {
      bool pass = true;

      if (numGroups) {
        testLogger.PushSection();
      }

      numGroups = 0;

      VerifyGroup(group, numGroups, pass, groupLabel);

      if (group == uint32_t(-1) && numGroups) {
        continue;
      }

      if (numGroups) {
        testLogger.PopSection();
      }

      if (pass) {
        numPassed++;
      } else {
        if (groupLabel.length() > 0) {
          testLogger.PrintError("'{}' failed!", groupLabel);
        }
        this->pass = false;
      }
    } while (++group < numGroups);

    if (pass) {
      if (numGroups > 0) {
        testLogger.PrintSuccess("'{}' passed: {} out of {} sub-groups passed", label, numPassed, numGroups);
      } else {
        testLogger.PrintSuccess("'{}' passed", label);
      }

      return true;
    }

    // Fail
    if (numGroups > 0) {
      testLogger.PrintError("'{}' failed: {} out of {} sub-groups failed!", label, numGroups - numPassed, numGroups);
    } else {
      testLogger.PrintError("'{}' failed!", label);
    }
    return false;
  }
};

inline static std::vector<Test *> allTests = {};

template <typename Test_T> struct TestSubscription {
  TestSubscription() { allTests.push_back(new Test_T()); }
};

int main() {
  testLogger.PrintMessage("Running all tests...");
  testLogger.PushSection();

  uint32_t passed = 0;

  for (auto &test : allTests) {
    if (test->Verify()) {
      passed++;
    }
    delete test;
  }

  testLogger.PopSection();
  if (passed == allTests.size()) { // Pass
    if (allTests.size() > 0) {
      testLogger.PrintSuccess("All tests passed: {} out of {} tests passed", passed, allTests.size());
    } else {
      testLogger.PrintSuccess("All tests passed");
    }
    return 0;
  }

  // Fail

  testLogger.PrintError("Some tests failed: {} out of {} tests failed!", allTests.size() - passed, allTests.size());
  return 1;
}

//  +-------------+
//  | Expressions |
//  +-------------+

struct SplitExpression;

constexpr SplitExpression SplitAroundOperator(const char *const expression, size_t const &expressionLength,
                                              const char *const op, size_t const &opLength);
constexpr SplitExpression GenerateSplit(const char *const lhs, size_t const &lhsLength, const char *const rhs,
                                        size_t const &rhsLength, size_t const &totalLength, const char *const op,
                                        size_t const &opLength);

enum class BinaryOperator { Eq, Neq, Lt, Leq, Gt, Geq, Add, Sub, Mul, Div };

#define OP_LIST                                                                                                        \
  OP(==, Eq), OP(!=, Neq), OP(<, Lt), OP(<=, Leq), OP(>, Gt), OP(>=, Geq), OP(+, Add), OP(-, Sub), OP(*, Mul),         \
      OP(/, Div)
#define INDIRECT_FOREACH(Macro, List) FOR_EACH(Macro, List)

template <typename Lhs_T, typename Rhs_T, typename Value_T, BinaryOperator Op> class BinaryExpression;

// IMPLEMENTATIONS

struct SplitExpression {
  const char *const lhs;
  const size_t lhsLength;
  const char *const op;
  const size_t opLength;
  const char *const rhs;
  const size_t rhsLength;
  const size_t totalLength;
};

template <typename Lhs_T, typename Rhs_T, typename Value_T, BinaryOperator Op> struct EvaluateOperator;
template <typename T> struct Evaluate;

template <BinaryOperator Op> struct OperatorString {
  static char const *value;
  static size_t length;
};

#define EXP_OP(OpInfix, OpName)                                                                                        \
  template <typename Other_Rhs_T>                                                                                      \
  inline BinaryExpression<BinaryExpression<Lhs_T, Rhs_T, Value_T, Op>, Other_Rhs_T,                                    \
                          decltype(Value_T() OpInfix Other_Rhs_T()), BinaryOperator::OpName>                           \
  operator OpInfix(Other_Rhs_T const &other) const {                                                                   \
    return {*this, other, stringForm};                                                                                 \
  }

#define EXP_CONCATENATOR(Val) EXP_##Val

template <typename Lhs_T, typename Rhs_T, typename Value_T, BinaryOperator Op> struct BinaryExpression {
  Lhs_T const lhs;
  Rhs_T const rhs;
  SplitExpression const stringForm;
  Value_T const value;

  BinaryExpression(Lhs_T const &lhs, Rhs_T const &rhs, char const *expressionString, size_t stringLength)
      : lhs(lhs), rhs(rhs), stringForm(SplitAroundOperator(expressionString, stringLength, OperatorString<Op>::value,
                                                           OperatorString<Op>::length)),
        value(EvaluateOperator<Lhs_T, Rhs_T, Value_T, Op>{}(lhs, rhs)) {}

  BinaryExpression(Lhs_T const &lhs, Rhs_T const &rhs, SplitExpression const &childExpr)
      : lhs(lhs), rhs(rhs),
        stringForm(GenerateSplit(childExpr.lhs, childExpr.lhsLength, childExpr.rhs, childExpr.rhsLength,
                                 childExpr.totalLength, OperatorString<Op>::value, OperatorString<Op>::length)),
        value(EvaluateOperator<Lhs_T, Rhs_T, Value_T, Op>{}(lhs, rhs)) {}
  inline operator Value_T() const { return value; }
  inline std::string /*TODO: Find copy-free formattable alternative*/ GetFullString() const {
    return std::string(stringForm.lhs, stringForm.rhs - stringForm.lhs + stringForm.rhsLength);
  }
  inline std::string GetLhsString() const { return std::string(stringForm.lhs, stringForm.lhsLength); }
  inline std::string GetRhsString() const { return std::string(stringForm.rhs, stringForm.rhsLength); }
  inline std::string GetOpString() const { return std::string(stringForm.op, stringForm.opLength); }

  auto GetLhsValue() const { return Evaluate<Lhs_T>{}(lhs); }
  auto GetRhsValue() const { return Evaluate<Rhs_T>{}(rhs); }
  auto GetValue() const { return (Value_T)(*this); }

  INDIRECT_FOREACH(EXP_CONCATENATOR, OP_LIST)
};

#define TYPE_OP(OpInfix, OpName)                                                                                       \
  template <typename Other_Rhs_T>                                                                                      \
  inline BinaryExpression<Type<T>, Other_Rhs_T, decltype(T() OpInfix Other_Rhs_T()), BinaryOperator::OpName>           \
  operator OpInfix(Other_Rhs_T const &other) const {                                                                   \
    return {*this, other, expression, expressionLength};                                                               \
  }

#define TYPE_CONCATENATOR(Val) TYPE_##Val

template <typename T> struct Type {
  const char *expression;
  size_t expressionLength;
  T value;

  Type(T const &value, char const *expression, size_t expressionLength)
      : expression(expression), expressionLength(expressionLength), value(value) {}

  inline operator T() const { return value; }

  INDIRECT_FOREACH(TYPE_CONCATENATOR, OP_LIST)
};

struct Decompose {
  const char *expression;
  size_t expressionLength;

  Decompose(const char *expression, size_t expressionLength)
      : expressionLength(expressionLength), expression(expression) {}

  template <typename T> inline Type<T> operator->*(T const &t) { return {t, expression, expressionLength}; }
};

// Helper to treat expressions and values the same for value retrieval
template <typename T> struct Evaluate {
  inline T operator()(T const &val) { return val; }
};
template <typename T> struct Evaluate<Type<T>> {
  inline T operator()(T const &val) { return val; }
};
template <typename Lhs_T, typename Rhs_T, typename Value_T, BinaryOperator Op>
struct Evaluate<BinaryExpression<Lhs_T, Rhs_T, Value_T, Op>> {
  inline Value_T operator()(BinaryExpression<Lhs_T, Rhs_T, Value_T, Op> const &expr) { return expr; }
};

// Helper to switch on operator when calculating expression value
#define EVALUATE_OP(OpInfix, OpName)                                                                                   \
  template <typename Lhs_T, typename Rhs_T, typename Value_T>                                                          \
  struct EvaluateOperator<Lhs_T, Rhs_T, Value_T, BinaryOperator::OpName> {                                             \
    inline auto operator()(Lhs_T const &lhs, Rhs_T const &rhs) {                                                       \
      return Evaluate<Lhs_T>{}(lhs)OpInfix Evaluate<Rhs_T>{}(rhs);                                                     \
    }                                                                                                                  \
  };
#define EVALUATE_CONCATENATOR(Val) EVALUATE_##Val

INDIRECT_FOREACH(EVALUATE_CONCATENATOR, OP_LIST)

// Helper to determine whether a template parameter is a binary expression
template <typename T> struct IsExpression {
  inline static bool value = false;
};

template <typename Lhs_T, typename Rhs_T, typename Value_T, BinaryOperator Op>
struct IsExpression<BinaryExpression<Lhs_T, Rhs_T, Value_T, Op>> {
  inline static bool value = true;
};

// Printing

typedef uint16_t depth_t;

// Helper to gauge expression depth
template <typename T> struct ExpressionDepth {
  inline static constexpr depth_t value = 0;
};

template <typename Lhs_T, typename Rhs_T, typename Value_T, BinaryOperator Op>
struct ExpressionDepth<BinaryExpression<Lhs_T, Rhs_T, Value_T, Op>> {
  inline static depth_t constexpr value = 1 + std::max(ExpressionDepth<Lhs_T>::value, ExpressionDepth<Rhs_T>::value);
};

// Helper to print expression at different levels of expansion
template <typename T, depth_t NumExpansions> struct ExpressionPrint {
  inline auto operator()(T const &expression) const { return std::format("{}", expression); }
};

template <typename T, depth_t NumExpansions> struct ExpressionPrint<Type<T>, NumExpansions> {
  inline auto operator()(Type<T> const &expression) const {
    if constexpr (NumExpansions) {
      return std::format("{}", (T)expression);
    } else {
      return std::format("{}", expression.expression);
    }
  }
};

template <typename Lhs_T, typename Rhs_T, typename Value_T, BinaryOperator Op, depth_t NumExpansions>
struct ExpressionPrint<BinaryExpression<Lhs_T, Rhs_T, Value_T, Op>, NumExpansions> {
  typedef BinaryExpression<Lhs_T, Rhs_T, Value_T, Op> Expr_T;
  inline std::string operator()(Expr_T const &expression) const {
    if constexpr (NumExpansions == 0) {
      return std::format("{} {} {}", expression.GetLhsString(), OperatorString<Op>::value, expression.GetRhsString());
    }

    if constexpr (NumExpansions > ExpressionDepth<Expr_T>::value) {
      return std::format("{}", (Value_T)expression);
    }

    if constexpr (NumExpansions > 0 && NumExpansions <= ExpressionDepth<Expr_T>::value) {

      return std::format("{} {} {}", ExpressionPrint<Lhs_T, NumExpansions>{}(expression.lhs), OperatorString<Op>::value,
                         ExpressionPrint<Rhs_T, NumExpansions>{}(expression.rhs));
    }
  }
};

template <typename Expr_T, depth_t ToDepth> struct DepthPrinter {
  inline void operator()(Expr_T const &expr, Debug::Logging::Logger const &logger) {
    if constexpr (ToDepth) {
      DepthPrinter<Expr_T, ToDepth - 1>{}(expr, logger);
    }
    logger.PrintError("{}'{}'", ToDepth ? "Evaluated to " : "Expression   ", ExpressionPrint<Expr_T, ToDepth>{}(expr));
  }
};

#define FILL_STRING_FOR_OP(Op, OpName)                                                                                 \
  template <> inline const char *OperatorString<BinaryOperator::OpName>::value = #Op;                                  \
  template <> inline size_t OperatorString<BinaryOperator::OpName>::length = sizeof(#Op) - 1;

#define FILL_STRING_CONCATENATOR(Val) FILL_STRING_FOR_##Val

INDIRECT_FOREACH(FILL_STRING_CONCATENATOR, OP_LIST)

constexpr SplitExpression GenerateSplit(const char *const lhs, size_t const &lhsLength, const char *const rhs,
                                        size_t const &rhsLength, size_t const &totalLength, const char *const op,
                                        size_t const &opLength) {
  auto const splitRhs = SplitAroundOperator(rhs, rhsLength, op, opLength);
  return {.lhs = lhs,
          .lhsLength = static_cast<size_t>(splitRhs.lhs + splitRhs.lhsLength - lhs),
          .op = splitRhs.op,
          .opLength = splitRhs.opLength,
          .rhs = splitRhs.rhs,
          .rhsLength = splitRhs.rhsLength,
          .totalLength = totalLength};
}
constexpr SplitExpression SplitAroundOperator(const char *const expression, size_t const &expressionLength,
                                              const char *const op, size_t const &opLength) {
  uint16_t openParens = 0;
  size_t lastNonWhitespace = 0;
  for (size_t cursor = 0; cursor < expressionLength; cursor++) {
    if (expression[cursor] == '(') {
      openParens++;
    } else if (expression[cursor] == ')') {
      openParens--;
    } else if (openParens == 0 && expression[cursor] == op[0]) {
      bool match = true;
      for (size_t opCursor = 1; opCursor < opLength; opCursor++) {
        if (expression[cursor + opCursor] != op[opCursor]) {
          match = false;
          break;
        }
      }
      if (match) {
        const char *const opStart = expression + cursor;
        cursor += opLength;
        while (std::isspace(expression[cursor])) {
          cursor++;
        }

        return {.lhs = expression,
                .lhsLength = lastNonWhitespace + 1,
                .op = opStart,
                .opLength = opLength,
                .rhs = expression + cursor,
                .rhsLength = expressionLength - cursor,
                .totalLength = expressionLength};
      }
    }

    if (!std::isspace(expression[cursor])) {
      lastNonWhitespace = cursor;
    }
  }

  throw std::runtime_error("Tried to split string without splitting operator!");
}