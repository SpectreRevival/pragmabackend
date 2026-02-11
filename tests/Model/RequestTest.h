#pragma once
#include <gtest/gtest.h>
#include <filesystem>
#include <BackendEnvironment.h>

namespace fs = std::filesystem;

class RequestTest : public ::testing::TestWithParam<fs::path> {
    std::unique_ptr<BackendEnvironment> backend;

    void SetUp() override;

    void TearDown() override;
};