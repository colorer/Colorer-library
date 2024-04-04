#define CATCH_CONFIG_MAIN

#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>
#include <catch2/catch.hpp>
#include "spdlog/spdlog.h"

std::shared_ptr<spdlog::logger> logger;

TEST_CASE("All test cases reside in other .cpp files") {
  // disable logging
  spdlog::drop_all();
  logger = spdlog::null_logger_mt("main");
  spdlog::set_default_logger(logger);
}