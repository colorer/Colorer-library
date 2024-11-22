#include <catch2/catch.hpp>
#include "colorer/parsers/HrcLibraryImpl.h"
#include "colorer/utils/FileSystems.h"

TEST_CASE("Load hrc")
{
  auto temp_path = fs::current_path();
  auto work_dir = temp_path / "data/type_cue.hrc";
  auto uwork_dir = UnicodeString(work_dir.c_str());
  XmlInputSource file1(uwork_dir, nullptr);
  HrcLibrary lib;
  lib.loadSource(&file1);
}