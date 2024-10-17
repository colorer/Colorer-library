#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "test_common.h"

std::unique_ptr<TestLogger> logger;

TEST_CASE("All test cases reside in other .cpp files")
{
  logger = std::make_unique<TestLogger>();
  Log::registerLogger(*logger);
}