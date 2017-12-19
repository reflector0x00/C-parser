#pragma once
#include <iostream>
#include <string>
#include <regex>
#include <unordered_set>
#include <functional>
#include "lalr_parser.h"
#include "lexer.h"


template <typename T>
using function_t = std::function<bool(parse_tree<T>&)>;

extern const std::string c_parser_strings[];
extern const uint64_t c_parser_size;
extern const unsigned char c_parser_table[];

enum c_token;

class c_parser {
	std::unordered_set<std::string> _typedefs;
	std::unordered_set<std::string> _enums;
	function_t<c_token> c_parser_functions[2];
	lalr_table<c_token> _table;
	lalr_parser<c_token, std::string> _parser;
	lexer _lexer;
public:
	c_parser();
	c_parser(std::istream& o);
	void set_stream(std::istream& o);
	bool eof();
	void next();
	void run();
	parse_tree<c_token>& get_tree();
};


