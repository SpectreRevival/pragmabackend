#pragma once
#include <RequestTest.h>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using namespace nlohmann;

class SequencedRequestTest : public RequestTest {
};