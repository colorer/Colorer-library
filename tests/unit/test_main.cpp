#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "colorer/Common.h"

#ifndef COLORER_FEATURE_DUMMYLOGGER
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>
std::shared_ptr<spdlog::logger> logger;
#else
std::shared_ptr<DummyLogger> logger;
#endif

TEST_CASE("All test cases reside in other .cpp files") {

#ifndef COLORER_FEATURE_DUMMYLOGGER
  // disable logging
  spdlog::drop_all();
  logger = spdlog::null_logger_mt("main");
  spdlog::set_default_logger(logger);
#else
  logger = std::make_shared<DummyLogger>();
#endif
}