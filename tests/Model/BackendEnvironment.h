#pragma once
#include <gtest/gtest.h>
#include <process.hpp>

class BackendEnvironment : public ::testing::Environment {
  public:
    std::unique_ptr<TinyProcessLib::Process> server;

    void SetUp() override;

    void TearDown() override;
};
