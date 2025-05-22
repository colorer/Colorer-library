#include <catch2/catch.hpp>
#include "colorer/Common.h"
#include "colorer/utils/Environment.h"
#include "colorer/xml/XmlInputSource.h"

TEST_CASE("Test create XmlInputSource")
{
  auto work_dir = fs::current_path();
  const UnicodeString empty_string(u"");

  SECTION("Create XmlInputSource with not exists file")
  {
    REQUIRE_THROWS_WITH(XmlInputSource(empty_string), Catch::Contains("path is empty"));
    REQUIRE_THROWS_WITH(XmlInputSource(empty_string, nullptr), Catch::Contains("path is empty"));
    REQUIRE_THROWS_WITH(XmlInputSource(empty_string, &empty_string), Catch::Contains("path is empty"));

    auto non_exist_file = work_dir / "non_exist";
    REQUIRE_THROWS_WITH(XmlInputSource(non_exist_file.c_str()), Catch::Contains("isn't regular file"));
    REQUIRE_THROWS_WITH(XmlInputSource(non_exist_file.c_str(), nullptr), Catch::Contains("isn't regular file"));
    REQUIRE_THROWS_WITH(XmlInputSource(non_exist_file.c_str(), &empty_string), Catch::Contains("isn't regular file"));
  }

  auto temp_path = work_dir / "data";
  auto test_file1 = temp_path / "test1.xml";
  auto test_file2 = temp_path / "test2.xml";

  SECTION("Create XmlInputSource with exists file")
  {
    REQUIRE_NOTHROW(XmlInputSource(test_file1.c_str()));
    REQUIRE_NOTHROW(XmlInputSource(test_file1.c_str(), nullptr));
    REQUIRE_NOTHROW(XmlInputSource(test_file1.c_str(), &empty_string));
  }

  SECTION("Create relative XmlInputSource with exists file")
  {
    auto u_test_file2 = UnicodeString(test_file2.c_str());
    std::unique_ptr<XmlInputSource> test_source;

    REQUIRE_NOTHROW(test_source = std::make_unique<XmlInputSource>("test1.xml", &u_test_file2));
    REQUIRE_NOTHROW(test_source = test_source->createRelative(test_file2.c_str()));
    REQUIRE(test_source->getPath().compare(u_test_file2) == 0);
  }

  SECTION("Create relative XmlInputSource with not exists file")
  {
    auto u_test_file2 = UnicodeString(test_file2.c_str());
    std::unique_ptr<XmlInputSource> test_source;

    REQUIRE_NOTHROW(test_source = std::make_unique<XmlInputSource>("test1.xml", &u_test_file2));
    REQUIRE_THROWS_WITH(test_source->createRelative("test3.xml"), Catch::Contains("isn't regular file"));
  }

  SECTION("Create XmlInputSource with environment variable in path")
  {
#ifndef WIN32
    colorer::Environment::setOSEnv("COLORER_TEST_XML1", temp_path.c_str());
    UnicodeString path1("$COLORER_TEST_XML1/test1.xml");
    UnicodeString path2("$COLORER_TEST_XML2/test2.xml");

    REQUIRE_NOTHROW(XmlInputSource(path1));
    REQUIRE_THROWS_WITH(XmlInputSource(path2), Catch::Contains("$COLORER_TEST_XML2/test2.xml isn't regular file"));
    REQUIRE_NOTHROW(XmlInputSource("test1.xml", &path1));
#else
    colorer::Environment::setOSEnv("COLORER_TEST_XML1", temp_path.c_str());
    UnicodeString path1("%COLORER_TEST_XML1%/test1.xml");
    UnicodeString path2("%COLORER_TEST_XML2%/test2.xml");

    REQUIRE_NOTHROW(XmlInputSource(path1));
    REQUIRE_THROWS_WITH(XmlInputSource(path2), Catch::Contains("%COLORER_TEST_XML2%\\test2.xml isn't regular file"));
    REQUIRE_NOTHROW(XmlInputSource("test1.xml", &path1));
#endif
  }
}

TEST_CASE("Test isFsURI")
{
  const UnicodeString empty_string(u"");
  UnicodeString path1(u"/home/user/testfolder");
  UnicodeString path2(u"/home/user/testfolder2");
  UnicodeString win_path2(u"c:\\testdir\\testfolder2");
  UnicodeString jar_path1(u"jar:common.jar!hrc/1.hrc");

  REQUIRE(XmlInputSource::isFileSystemURI(path1, nullptr) == true);
  REQUIRE(XmlInputSource::isFileSystemURI(path1, &path2) == true);
  REQUIRE(XmlInputSource::isFileSystemURI(win_path2, nullptr) == true);
  REQUIRE(XmlInputSource::isFileSystemURI(jar_path1, nullptr) == false);
  REQUIRE(XmlInputSource::isFileSystemURI(jar_path1, &path2) == false);
}

#ifndef COLORER_FEATURE_ZIPINPUTSOURCE
TEST_CASE("Work with path to zip: zip disabled")
{
  UnicodeString path2(u"hrc/1.hrc");
  UnicodeString jar_path1(u"jar:colorer.jar!hrc/1.hrc");
  REQUIRE_THROWS_WITH(XmlInputSource(jar_path1), Catch::Contains("zip input source not supported"));
  REQUIRE_THROWS_WITH(XmlInputSource(jar_path1, nullptr), Catch::Contains("zip input source not supported"));
  REQUIRE_THROWS_WITH(XmlInputSource(path2, &jar_path1), Catch::Contains("zip input source not supported"));
}
#endif

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
TEST_CASE("Check expand paths to files from jar-URI")
{
  SECTION("Simple path")
  {
    UnicodeString path_to_file(u"jar:common.zip!base/c.hrc");
    auto paths_cpp = LibXmlInputSource::getFullPathsToZip(path_to_file);

    REQUIRE(paths_cpp.full_path == path_to_file);
    REQUIRE(paths_cpp.path_in_jar == u"base/c.hrc");
    REQUIRE(paths_cpp.path_to_jar == u"common.zip");
  }

  SECTION("jar-path with env")
  {
    const char env[]="/home/user2/base/hrc";
    colorer::Environment::setOSEnv("CUR_DIR", env);
    UnicodeString path_to_file(u"jar:$CUR_DIR/common.zip!base/c.hrc");
    UnicodeString full_path(u"jar:/home/user2/base/hrc/common.zip!base/c.hrc");

    auto paths_cpp2 = LibXmlInputSource::getFullPathsToZip(path_to_file);

    REQUIRE(paths_cpp2.full_path == full_path);
    REQUIRE(paths_cpp2.path_in_jar == u"base/c.hrc");
    REQUIRE(paths_cpp2.path_to_jar == u"/home/user2/base/hrc/common.zip");
  }

  SECTION("jar-path relative normal path")
  {
    UnicodeString path_to_file(u"jar:common.zip!base/c.hrc");
    UnicodeString base_path(u"/home/user/base/hrc/proto.hrc");
    UnicodeString full_path(u"jar:/home/user/base/hrc/common.zip!base/c.hrc");

    auto paths_cpp2 = LibXmlInputSource::getFullPathsToZip(path_to_file, &base_path);

    REQUIRE(paths_cpp2.full_path == full_path);
    REQUIRE(paths_cpp2.path_in_jar == u"base/c.hrc");
    REQUIRE(paths_cpp2.path_to_jar == u"/home/user/base/hrc/common.zip");
  }

  SECTION("jar-path with env relative normal path")
  {
    constexpr char env[]="/home/user2/base/hrc";
    colorer::Environment::setOSEnv("CUR_DIR", env);
    UnicodeString path_to_file(u"jar:$CUR_DIR/common.zip!base/c.hrc");
    UnicodeString base_path(u"/home/user/base/hrc/proto.hrc");
    UnicodeString full_path(u"jar:/home/user2/base/hrc/common.zip!base/c.hrc");

    auto paths_cpp2 = LibXmlInputSource::getFullPathsToZip(path_to_file, &base_path);

    REQUIRE(paths_cpp2.full_path == full_path);
    REQUIRE(paths_cpp2.path_in_jar == u"base/c.hrc");
    REQUIRE(paths_cpp2.path_to_jar == u"/home/user2/base/hrc/common.zip");
  }

  SECTION("normal path relative jar-path")
  {
    UnicodeString full_path(u"jar:/home/user/base/hrc/common.zip!base/c.hrc");
    auto paths_cpp3 = LibXmlInputSource::getFullPathsToZip(u"c-unix.ent.hrc", &full_path);
    REQUIRE(paths_cpp3.full_path == u"jar:/home/user/base/hrc/common.zip!base/c-unix.ent.hrc");
    REQUIRE(paths_cpp3.path_in_jar == u"base/c-unix.ent.hrc");
    REQUIRE(paths_cpp3.path_to_jar == u"/home/user/base/hrc/common.zip");
  }

  SECTION("bad jar-path format")
  {
    UnicodeString bad_path_to_cpp(u"jar:common.zip/base/c.hrc");
    UnicodeString bad_full_path(u"jar:/home/user/base/hrc/common.zip/base/c.hrc");
    REQUIRE_THROWS_WITH(LibXmlInputSource::getFullPathsToZip(bad_path_to_cpp), Catch::Contains("Bad jar uri format"));
    REQUIRE_THROWS_WITH(LibXmlInputSource::getFullPathsToZip("c-unix.ent.hrc", &bad_full_path),
                        Catch::Contains("Bad jar uri format"));

    UnicodeString base_path(u"/home/user/base/hrc/proto.hrc");
    REQUIRE_THROWS_WITH(LibXmlInputSource::getFullPathsToZip(base_path),
                        Catch::Contains("The path to the jar was not found"));
    REQUIRE_THROWS_WITH(LibXmlInputSource::getFullPathsToZip("c-unix.ent.hrc", &base_path),
                        Catch::Contains("The path to the jar was not found"));
  }
}

TEST_CASE("Create XmlInputSource to zip: zip enabled")
{
  // create files for tests
  auto work_dir = fs::current_path();
  auto temp_path = work_dir / "data";
  auto test_file1 = temp_path / "test1.zip";

  UnicodeString path_to_zip = u"jar:" + UnicodeString(test_file1.c_str()) + u"!base/c.hrc";

  REQUIRE_NOTHROW(XmlInputSource(path_to_zip));
  REQUIRE_NOTHROW(XmlInputSource(u"c-unix.ent.hrc", &path_to_zip));

  UnicodeString full_path(u"jar:/home/user/base/hrc/common.zip!base/c.hrc");
  REQUIRE_THROWS_WITH(XmlInputSource(full_path), Catch::Contains("isn't regular file"));
  REQUIRE_THROWS_WITH(XmlInputSource(u"c-unix.ent.hrc", &full_path), Catch::Contains("isn't regular file"));
}
#endif