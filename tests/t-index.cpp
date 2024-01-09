#include "t-index.h"

i32 runAllTests() {
    std::cout << "\n" << "RUNNING TESTS" << "\n\n";

    RunTestSuite(runDecoderTestsSuite);

    return 0;
}
