#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <unordered_set>
#include "lalr_parser.h"
#include "lexer.h"
#include "c_token.h"
#include "c_parser.h"


int main(int argc, char* argv[]) {

	if (argc != 3) { //TO-DO: help
		std::cout << "usage:" << std::endl
			<< "c_parser input.c output.gv" << std::endl;
		return 1;
	}
	
	std::ifstream file_in(argv[1]);
	std::ofstream file_out(argv[2]);
	if (!file_in.is_open()) {
		std::cout << "Can't open " << argv[1] << std::endl; //TO-DO: shell load
		return 1;
	}
	if (!file_out.is_open()) {
		std::cout << "Can't open " << argv[2] << std::endl; //TO-DO: shell load
		return 1;
	}


	std::unordered_set<std::string> typedefs;
	std::unordered_set<std::string> enums;

	function_t<c_token> c_parser_functions[] = {

		[&enums](parse_tree<c_token>& data) mutable {
			enums.insert(data.last().at(0).data());
			return true;
		} ,

		[&typedefs](parse_tree<c_token>& data) mutable {
		parse_tree<c_token>& current = data.last();
		if (current.at(0).begin()->id() == t_typedef) {
			for (size_t i = 0; i < current.at(1).size(); ++i)
				for (auto iter = current.at(1).at(i).begin(); iter != current.at(1).at(i).end(); ++iter) {
					if (iter->id() == t_identifier) {
						typedefs.insert(iter->data());
						break;
					}
				}
			}
			return true;
		}

	};


	lexer lex(" \t\n", "'\"",
	{
		{"//", "\n"},
		{"/*", "*/"}
	},
		true,
	{
		"...",
		">>=",
		"<<=",
		"+=",
		"-=",
		"*=",
		"/=",
		"%=",
		"&=",
		"^=",
		"|=",
		">>",
		"<<",
		"++",
		"--",
		"->",
		"&&",
		"||",
		"<=",
		">=",
		"==",
		"!=",
		";",
		"{",
		"<%",
		"}",
		"%>",
		",",
		":",
		"=",
		"(",
		")",
		"[",
		"<:",
		"]",
		":>",
		".",
		"&",
		"!",
		"~",
		"-",
		"+",
		"*",
		"/",
		"%",
		"<",
		">",
		"^",
		"|",
		"?"
	});


	//Lexems adapted from http://www.quut.com/c/ANSI-C-grammar-l-2011.html
	std::string r_O("[0-7]");
	std::string r_D("[0-9]");
	std::string r_NZ("[1-9]");
	std::string r_L("[a-zA-Z_]");
	std::string r_A("[a-zA-Z_0-9]");
	std::string r_H("[a-fA-F0-9]");
	std::string r_HP("(0[xX])");
	std::string r_E("([Ee][+-]?[0-9]+)");
	std::string r_P("([Pp][+-]?[0-9]+)");
	std::string r_FS("(f|F|l|L)");
	std::string r_IS("(((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))");
	std::string r_CP("(u|U|L)");
	std::string r_SP("(u8|u|U|L)");
	std::string r_ES("(\\\\(['\"\\?\\\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))");
	std::string r_WS("[ \t\v\n\f]");

	lex.add_keyword("auto", [](std::string& name) { return t_auto; });
	lex.add_keyword("break", [](std::string& name) { return t_break; });
	lex.add_keyword("case", [](std::string& name) { return t_case; });
	lex.add_keyword("char", [](std::string& name) { return t_char; });
	lex.add_keyword("const", [](std::string& name) { return t_const; });
	lex.add_keyword("continue", [](std::string& name) { return t_continue; });
	lex.add_keyword("default", [](std::string& name) { return t_default; });
	lex.add_keyword("do", [](std::string& name) { return t_do; });
	lex.add_keyword("double", [](std::string& name) { return t_double; });
	lex.add_keyword("else", [](std::string& name) { return t_else; });
	lex.add_keyword("enum", [](std::string& name) { return t_enum; });
	lex.add_keyword("extern", [](std::string& name) { return t_extern; });
	lex.add_keyword("float", [](std::string& name) { return t_float; });
	lex.add_keyword("for", [](std::string& name) { return t_for; });
	lex.add_keyword("goto", [](std::string& name) { return t_goto; });
	lex.add_keyword("if", [](std::string& name) { return t_if; });
	lex.add_keyword("inline", [](std::string& name) { return t_inline; });
	lex.add_keyword("int", [](std::string& name) { return t_int; });
	lex.add_keyword("long", [](std::string& name) { return t_long; });
	lex.add_keyword("register", [](std::string& name) { return t_register; });
	lex.add_keyword("restrict", [](std::string& name) { return t_restrict; });
	lex.add_keyword("return", [](std::string& name) { return t_return; });
	lex.add_keyword("short", [](std::string& name) { return t_short; });
	lex.add_keyword("signed", [](std::string& name) { return t_signed; });
	lex.add_keyword("sizeof", [](std::string& name) { return t_sizeof; });
	lex.add_keyword("static", [](std::string& name) { return t_static; });
	lex.add_keyword("struct", [](std::string& name) { return t_struct; });
	lex.add_keyword("switch", [](std::string& name) { return t_switch; });
	lex.add_keyword("typedef", [](std::string& name) { return t_typedef; });
	lex.add_keyword("union", [](std::string& name) { return t_union; });
	lex.add_keyword("unsigned", [](std::string& name) { return t_unsigned; });
	lex.add_keyword("void", [](std::string& name) { return t_void; });
	lex.add_keyword("volatile", [](std::string& name) { return t_volatile; });
	lex.add_keyword("while", [](std::string& name) { return t_while; });
	lex.add_keyword("_Alignas", [](std::string& name) { return t_alignas; });
	lex.add_keyword("_Alignof", [](std::string& name) { return t_alignof; });
	lex.add_keyword("_Atomic", [](std::string& name) { return t_atomic; });
	lex.add_keyword("_Bool", [](std::string& name) { return t_bool; });
	lex.add_keyword("_Complex", [](std::string& name) { return t_complex; });
	lex.add_keyword("_Generic", [](std::string& name) { return t_generic; });
	lex.add_keyword("_Imaginary", [](std::string& name) { return t_imaginary; });
	lex.add_keyword("_Noreturn", [](std::string& name) { return t_noreturn; });
	lex.add_keyword("_Static_assert", [](std::string& name) { return t_static_assert; });
	lex.add_keyword("_Thread_local", [](std::string& name) { return t_thread_local; });
	lex.add_keyword("__func__", [](std::string& name) { return t_func_name; });

	lex.add_regular(std::regex(r_L + r_A + "*"), [&typedefs, &enums](std::string& name) { 
		if (typedefs.find(name) != typedefs.end())
			return t_typedef_name;
		if (enums.find(name) != enums.end())
			return t_enumeration_constant;
		return t_identifier; 
	});

	lex.add_regular(std::regex(r_HP + r_H + "+" + r_IS + "?"), [](std::string& name) { return t_i_constant; });
	lex.add_regular(std::regex(r_NZ + r_D + "*" + r_IS + "?"), [](std::string& name) { return t_i_constant; });
	lex.add_regular(std::regex("0" + r_O + "*" + r_IS + "?"), [](std::string& name) { return t_i_constant; });
	lex.add_regular(std::regex(r_CP + "?'([^'\\\n]|" + r_ES + ")+'"), [](std::string& name) { return t_i_constant; });

	lex.add_regular(std::regex(r_D + "+" + r_E + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_D + "*\\." + r_D + "+" + r_E + "?" + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_D + "+\\." + r_E + "?" + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_D + "+\\." + r_E + "?" + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_HP + r_H + "+" + r_P + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_HP + r_H + "*\\." + r_H + "+" + r_P + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_HP + r_H + "+\\." + r_P + r_FS + "?"), [](std::string& name) { return t_f_constant; });

	lex.add_regular(std::regex("(" + r_SP + "?\\\"([^\"\\\n]|" + r_ES + ")*\\\"" + r_WS + "*)+"), [](std::string& name) { return t_string_literal; });

	lex.add_keyword("...", [](std::string& name) { return t_ellipsis; });
	lex.add_keyword(">>=", [](std::string& name) { return t_right_assign; });
	lex.add_keyword("<<=", [](std::string& name) { return t_left_assign; });
	lex.add_keyword("+=", [](std::string& name) { return t_add_assign; });
	lex.add_keyword("-=", [](std::string& name) { return t_sub_assign; });
	lex.add_keyword("*=", [](std::string& name) { return t_mul_assign; });
	lex.add_keyword("/=", [](std::string& name) { return t_div_assign; });
	lex.add_keyword("%=", [](std::string& name) { return t_mod_assign; });
	lex.add_keyword("&=", [](std::string& name) { return t_and_assign; });
	lex.add_keyword("^=", [](std::string& name) { return t_xor_assign; });
	lex.add_keyword("|=", [](std::string& name) { return t_or_assign; });
	lex.add_keyword(">>", [](std::string& name) { return t_right_op; });
	lex.add_keyword("<<", [](std::string& name) { return t_left_op; });
	lex.add_keyword("++", [](std::string& name) { return t_inc_op; });
	lex.add_keyword("--", [](std::string& name) { return t_dec_op; });
	lex.add_keyword("->", [](std::string& name) { return t_ptr_op; });
	lex.add_keyword("&&", [](std::string& name) { return t_and_op; });
	lex.add_keyword("||", [](std::string& name) { return t_or_op; });
	lex.add_keyword("<=", [](std::string& name) { return t_le_op; });
	lex.add_keyword(">=", [](std::string& name) { return t_ge_op; });
	lex.add_keyword("==", [](std::string& name) { return t_eq_op; });
	lex.add_keyword("!=", [](std::string& name) { return t_ne_op; });
	lex.add_keyword(";", [](std::string& name) { return t_semicolon; });
	lex.add_keyword("{", [](std::string& name) { return t_brace_left; });
	lex.add_keyword("<%", [](std::string& name) { return t_brace_left; });
	lex.add_keyword("}", [](std::string& name) { return t_brace_right; });
	lex.add_keyword("%>", [](std::string& name) { return t_brace_right; });
	lex.add_keyword(",", [](std::string& name) { return t_comma; });
	lex.add_keyword(":", [](std::string& name) { return t_colon; });
	lex.add_keyword("=", [](std::string& name) { return t_assign; });
	lex.add_keyword("(", [](std::string& name) { return t_round_left; });
	lex.add_keyword(")", [](std::string& name) { return t_round_right; });
	lex.add_keyword("[", [](std::string& name) { return t_square_left; });
	lex.add_keyword("<:", [](std::string& name) { return t_square_left; });
	lex.add_keyword("]", [](std::string& name) { return t_square_right; });
	lex.add_keyword(":>", [](std::string& name) { return t_square_right; });
	lex.add_keyword(".", [](std::string& name) { return t_dot; });
	lex.add_keyword("&", [](std::string& name) { return t_ampersand; });
	lex.add_keyword("!", [](std::string& name) { return t_exmark; });
	lex.add_keyword("~", [](std::string& name) { return t_tilda; });
	lex.add_keyword("-", [](std::string& name) { return t_minus; });
	lex.add_keyword("+", [](std::string& name) { return t_plus; });
	lex.add_keyword("*", [](std::string& name) { return t_star; });
	lex.add_keyword("/", [](std::string& name) { return t_slash; });
	lex.add_keyword("%", [](std::string& name) { return t_percent; });
	lex.add_keyword("<", [](std::string& name) { return t_angle_left; });
	lex.add_keyword(">", [](std::string& name) { return t_angle_right; });
	lex.add_keyword("^", [](std::string& name) { return t_carrete; });
	lex.add_keyword("|", [](std::string& name) { return t_vertical; });
	lex.add_keyword("?", [](std::string& name) { return t_qmark; });

	lex.add_keyword("\n", [](std::string& name) { return 0; });
	lex.add_keyword(" ", [](std::string& name) { return 0; });

	lalr_table<c_token> table(c_parser_table, c_parser_size, c_parser_strings, c_parser_functions);

	lalr_parser<c_token, std::string> parser(table);

	lex.set_stream(file_in);
	while (!lex.eof()) {
		token t = lex.next(); //TO-DO: template lexer
		parser.next((c_token)t.index(), t.data());
	}
	
	parse_tree<c_token>& tree = parser.get_tree();

	tree.to_dot(file_out);

	file_in.close();
	file_out.close();

	return 0;
}
