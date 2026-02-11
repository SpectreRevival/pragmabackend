#pragma once
#include <nlohmann/json.hpp>
#include <RequestTest.h>

namespace fs = std::filesystem;
using namespace nlohmann;

class SequencedRequestTest : public RequestTest {
};