#pragma once
#include <BackendEnvironment.h>
#include <filesystem>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

class RequestTest : public ::testing::TestWithParam<fs::path> {
    std::unique_ptr<BackendEnvironment> backend;

  public:
    void SetUp() override;

    void TearDown() override;
};