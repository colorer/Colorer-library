#include <catch2/catch.hpp>
#include <fstream>
#include <filesystem>
#include "colorer/parsers/HrcLibraryImpl.h"

namespace fs = std::filesystem;

TEST_CASE("Create files")
{
  auto file1_str = R"(<?xml version="1.0" encoding="UTF-8"?>
<hrc>
  <prototype name="cue" group="other" description="Cue sheet">
    <filename>/\.cue$/</filename>
	<parameters>
      <param value='1' description="lines for 'firstline'"/>
      <param name='firstlinebytes' value='5' description="bytes for 'firstline'"/>
    </parameters>
  </prototype>
	<type name="cue">
	</type>
</hrc>
)";
  auto temp_path = fs::current_path();
  auto work_dir = temp_path / "test1.hrc";
  std::ofstream file_handler;
  file_handler.open(work_dir.c_str());
  file_handler << file1_str;
  file_handler.close();
}

TEST_CASE("Load hrc")
{
  xercesc::XMLPlatformUtils::Initialize();
  auto temp_path = fs::current_path();
  auto work_dir = temp_path / "test1.hrc";
  auto uwork_dir = UnicodeString(work_dir.c_str());
  auto file1 = XmlInputSource::newInstance(&uwork_dir, nullptr);
  HrcLibrary lib;
  lib.loadSource(file1.get());
}