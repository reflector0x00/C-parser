#include <fstream>
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "c_parser.h"


SCENARIO("parse \"Hello, world!\" program", "[c_parser]" ) {
	std::ifstream file_in("hello_world.c");
	REQUIRE(file_in.is_open());
	c_parser parser(file_in);
	REQUIRE_NOTHROW(parser.run());
	SECTION("output of program - tree structure") {
		REQUIRE_NOTHROW(parser.get_tree());
	}
}

SCENARIO("typedefs and unions also processed", "[c_parser]") {
	SECTION("typedefs and uinions as type") {
		{
			std::ifstream file_in("typedef_union.c");
			REQUIRE(file_in.is_open());
			c_parser parser(file_in);
			REQUIRE_NOTHROW(parser.run());
		}
	}
	SECTION("non-type identifier as type") {
		{
			std::ifstream file_in("bad_type.c");
			REQUIRE(file_in.is_open());
			c_parser parser(file_in);
			REQUIRE_THROWS_AS(parser.run(), std::runtime_error);
		}
	}
}

SCENARIO("empty file isn't program", "[c_parser]") {
	std::ifstream file_in("empty.c");
	REQUIRE(file_in.is_open());
	c_parser parser(file_in);
	REQUIRE_THROWS_AS(parser.run(), std::runtime_error);
}
