#include "Tests/AlignmentTests.h"
#include "Tests/MathsTests.h"

using namespace Engine::Test;

BEGIN_TEST_CASE(all)

RUN_SUB_CASE(maths)
RUN_SUB_CASE(alignment)

END_TEST_CASE() // all

int main() { return run_all_tests(); }