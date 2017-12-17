#pragma once
#include <string>
#include <functional>
#include "parse_tree.h"
#include "c_token.h"

template <typename T>
using function_t = std::function<bool(parse_tree<T>&)>;

extern const std::string c_parser_strings[];
extern const uint64_t c_parser_size;
extern const unsigned char c_parser_table[];
