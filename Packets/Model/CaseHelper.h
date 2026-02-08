#pragma once

#include <algorithm> 
#include <cctype>  
#include <stdexcept>

static bool iequals(const std::string& a, const std::string& b) {
	return std::ranges::equal(a, b,
	                          [](const char a, const char b) {
		                          return tolower(a) == tolower(b);
	                          });
}