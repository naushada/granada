#ifndef __emailservice_test_cc__
#define __emailservice_test_cc__

#include "emailservice_test.hpp"

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}


#endif /*__emailservice_test_cc__*/