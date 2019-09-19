#include "version.hpp"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

using phosphor::software::updater::Version;

namespace fs = std::filesystem;

namespace
{
constexpr auto validManifest = R"(
purpose=xyz.openbmc_project.Software.Version.VersionPurpose.PSU
version=psu-dummy-test.v0.1
extended_version=model=dummy_model,manufacture=dummy_manufacture)";
}

class TestVersion : public ::testing::Test
{
  public:
    TestVersion()
    {
        auto tmpPath = fs::temp_directory_path();
        tmpDir = (tmpPath / "test_XXXXXX");
        if (!mkdtemp(tmpDir.data()))
        {
            throw "Failed to create temp dir";
        }
    }
    ~TestVersion()
    {
        fs::remove_all(tmpDir);
    }

    void writeFile(const fs::path& file, const char* data)
    {
        std::ofstream f{file};
        f << data;
        f.close();
    }
    std::string tmpDir;
};

TEST_F(TestVersion, getValuesFileNotExist)
{
    auto ret = Version::getValues("NotExist.file", {""});
    EXPECT_TRUE(ret.empty());
}

TEST_F(TestVersion, getValuesOK)
{
    auto manifestFilePath = fs::path(tmpDir) / "MANIFEST";
    writeFile(manifestFilePath, validManifest);
    auto ret = Version::getValues(manifestFilePath.string(),
                                  {"purpose", "version", "extended_version"});
    EXPECT_EQ(3u, ret.size());
    auto purpose = ret["purpose"];
    auto version = ret["version"];
    auto extVersion = ret["extended_version"];

    EXPECT_EQ("xyz.openbmc_project.Software.Version.VersionPurpose.PSU",
              purpose);
    EXPECT_EQ("psu-dummy-test.v0.1", version);
    EXPECT_EQ("model=dummy_model,manufacture=dummy_manufacture", extVersion);
}
