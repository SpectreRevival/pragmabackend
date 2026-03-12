#pragma once
#include <gtest/gtest.h>
#include <process.hpp>

class BackendEnvironment : public ::testing::Environment {
  private:
    std::unique_ptr<TinyProcessLib::Process> server;

  public:
    void SetUp() override;

    void TearDown() override;
};
