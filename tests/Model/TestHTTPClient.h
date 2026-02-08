#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

boost::beast::http::response<boost::beast::http::string_body> HTTPFetch(unsigned short port, std::string path, std::string packet, boost::beast::http::verb method);