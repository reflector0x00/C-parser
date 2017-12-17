#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <unordered_set>
#include "lalr_parser.h"
#include "lexer.h"
#include "c_token.h"
#include "c_parser.h"

#define rt(x) rule_terminal(x)


int main(int argc, char* argv[]) {


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


	if (argc != 3) {
		std::cout << "usage:" << std::endl
			<< "prog input_c_file output_gv_file" << std::endl;
		return 1;
	}


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
	/*
	//TO-DO: put far away
#define ctr_bind_identifier(x) x(#x)

	parse_rule<c_token> 			
		ctr_bind_identifier(program),
		ctr_bind_identifier(translation_unit),
		ctr_bind_identifier(external_declaration),
		ctr_bind_identifier(function_definition),
		ctr_bind_identifier(declaration_specifiers),
		ctr_bind_identifier(storage_class_specifier),
		ctr_bind_identifier(type_specifier),
		ctr_bind_identifier(atomic_type_specifier),
		ctr_bind_identifier(type_name),
		ctr_bind_identifier(struct_or_union_specifier),
		ctr_bind_identifier(struct_or_union),
		ctr_bind_identifier(struct_declaration_list),
		ctr_bind_identifier(struct_declaration),
		ctr_bind_identifier(specifier_qualifier_list),
		ctr_bind_identifier(struct_declarator_list),
		ctr_bind_identifier(struct_declarator),
		ctr_bind_identifier(constant_expression),
		ctr_bind_identifier(conditional_expression),
		ctr_bind_identifier(logical_or_expression),
		ctr_bind_identifier(logical_and_expression),
		ctr_bind_identifier(inclusive_or_expression),
		ctr_bind_identifier(exclusive_or_expression),
		ctr_bind_identifier(and_expression),
		ctr_bind_identifier(equality_expression),
		ctr_bind_identifier(relational_expression),
		ctr_bind_identifier(shift_expression),
		ctr_bind_identifier(additive_expression),
		ctr_bind_identifier(multiplicative_expression),
		ctr_bind_identifier(cast_expression),
		ctr_bind_identifier(unary_expression),
		ctr_bind_identifier(postfix_expression),
		ctr_bind_identifier(primary_expression),
		ctr_bind_identifier(constant),
		ctr_bind_identifier(string),
		ctr_bind_identifier(generic_selection),
		ctr_bind_identifier(assignment_expression),
		ctr_bind_identifier(assignment_operator),
		ctr_bind_identifier(generic_assoc_list),
		ctr_bind_identifier(generic_association),
		ctr_bind_identifier(argument_expression_list),
		ctr_bind_identifier(initializer_list),
		ctr_bind_identifier(designation),
		ctr_bind_identifier(designator_list),
		ctr_bind_identifier(designator),
		ctr_bind_identifier(initializer),
		ctr_bind_identifier(unary_operator),
		ctr_bind_identifier(expression),
		ctr_bind_identifier(enum_specifier),
		ctr_bind_identifier(enumerator_list),
		ctr_bind_identifier(enumerator),
		ctr_bind_identifier(enumeration_constant),
		ctr_bind_identifier(type_qualifier),
		ctr_bind_identifier(function_specifier),
		ctr_bind_identifier(alignment_specifier),
		ctr_bind_identifier(declarator),
		ctr_bind_identifier(direct_declarator),
		ctr_bind_identifier(type_qualifier_list),
		ctr_bind_identifier(parameter_type_list),
		ctr_bind_identifier(parameter_list),
		ctr_bind_identifier(parameter_declaration),
		ctr_bind_identifier(abstract_declarator),
		ctr_bind_identifier(direct_abstract_declarator),
		ctr_bind_identifier(pointer),
		ctr_bind_identifier(identifier_list),
		ctr_bind_identifier(declaration_list),
		ctr_bind_identifier(compound_statement),
		ctr_bind_identifier(block_item_list),
		ctr_bind_identifier(block_item),
		ctr_bind_identifier(statement),
		ctr_bind_identifier(labeled_statement),
		ctr_bind_identifier(expression_statement),
		ctr_bind_identifier(selection_statement),
		ctr_bind_identifier(iteration_statement),
		ctr_bind_identifier(jump_statement),
		ctr_bind_identifier(declaration),
		ctr_bind_identifier(init_declarator_list),
		ctr_bind_identifier(init_declarator),
		ctr_bind_identifier(static_assert_declaration);

	//Grammar adapted from http://www.quut.com/c/ANSI-C-grammar-y-2011.html
	program
		= translation_unit
		;

	translation_unit
		= external_declaration
		| translation_unit + external_declaration
		;
	
	external_declaration
		= function_definition
		| declaration
		;

	function_definition
		= declaration_specifiers + declarator + declaration_list + compound_statement
		| declaration_specifiers + declarator + compound_statement
		;


	declaration_specifiers
		= storage_class_specifier + declaration_specifiers
		| storage_class_specifier
		| type_specifier + declaration_specifiers
		| type_specifier
		| type_qualifier + declaration_specifiers
		| type_qualifier
		| function_specifier + declaration_specifiers
		| function_specifier
		| alignment_specifier + declaration_specifiers
		| alignment_specifier
		;

	storage_class_specifier
		= rt(t_typedef)
		| rt(t_extern)
		| rt(t_static)
		| rt(t_thread_local)
		| rt(t_auto)
		| rt(t_register)
		;

	type_specifier
		= rt(t_void)
		| rt(t_char)
		| rt(t_short)
		| rt(t_int)
		| rt(t_long)
		| rt(t_float)
		| rt(t_double)
		| rt(t_signed)
		| rt(t_unsigned)
		| rt(t_bool)
		| rt(t_complex)
		| rt(t_imaginary)	
		| atomic_type_specifier
		| struct_or_union_specifier
		| enum_specifier
		| rt(t_typedef_name) 	
		;

	atomic_type_specifier
		= rt(t_atomic) + rt(t_round_left) + type_name + rt(t_round_right)
		;

	type_name
		= specifier_qualifier_list + abstract_declarator
		| specifier_qualifier_list
		;

	struct_or_union_specifier
		= struct_or_union + rt(t_brace_left) + struct_declaration_list + rt(t_brace_right)
		| struct_or_union + rt(t_identifier) + rt(t_brace_left) + struct_declaration_list + rt(t_brace_right)
		| struct_or_union + rt(t_identifier)
		;

	struct_or_union
		= rt(t_struct)
		| rt(t_union)
		;

	struct_declaration_list
		= struct_declaration
		| struct_declaration_list + struct_declaration
		;


	struct_declaration
		= specifier_qualifier_list + rt(t_semicolon)	
		| specifier_qualifier_list + struct_declarator_list + rt(t_semicolon)
		| static_assert_declaration
		;

	specifier_qualifier_list
		= type_specifier + specifier_qualifier_list
		| type_specifier
		| type_qualifier + specifier_qualifier_list
		| type_qualifier
		;

	struct_declarator_list
		= struct_declarator
		| struct_declarator_list + rt(t_comma) + struct_declarator
		;

	struct_declarator
		= rt(t_colon) + constant_expression
		| declarator + rt(t_colon) + constant_expression
		| declarator
		;

	constant_expression
		= conditional_expression	
		;

	conditional_expression
		= logical_or_expression
		| logical_or_expression + rt(t_qmark) + expression + rt(t_colon) + conditional_expression
		;

	logical_or_expression
		= logical_and_expression
		| logical_or_expression + rt(t_or_op) + logical_and_expression
		;

	logical_and_expression
		= inclusive_or_expression
		| logical_and_expression + rt(t_and_op) + inclusive_or_expression
		;

	inclusive_or_expression
		= exclusive_or_expression
		| inclusive_or_expression + rt(t_vertical) + exclusive_or_expression
		;

	exclusive_or_expression
		= and_expression
		| exclusive_or_expression + rt(t_carrete) + and_expression
		;

	and_expression
		= equality_expression
		| and_expression + rt(t_ampersand) + equality_expression
		;

	equality_expression
		= relational_expression
		| equality_expression + rt(t_eq_op) + relational_expression
		| equality_expression + rt(t_ne_op) + relational_expression
		;

	relational_expression
		= shift_expression
		| relational_expression + rt(t_angle_left) + shift_expression
		| relational_expression + rt(t_angle_right) + shift_expression
		| relational_expression + rt(t_le_op) + shift_expression
		| relational_expression + rt(t_ge_op) + shift_expression
		;

	shift_expression
		= additive_expression
		| shift_expression + rt(t_left_op) + additive_expression
		| shift_expression + rt(t_right_op) + additive_expression
		;

	additive_expression
		= multiplicative_expression
		| additive_expression + rt(t_plus) + multiplicative_expression
		| additive_expression + rt(t_minus) + multiplicative_expression
		;

	multiplicative_expression
		= cast_expression
		| multiplicative_expression + rt(t_star) + cast_expression
		| multiplicative_expression + rt(t_slash) + cast_expression
		| multiplicative_expression + rt(t_percent) + cast_expression
		;

	cast_expression
		= unary_expression
		| rt(t_round_left) + type_name + rt(t_round_right) + cast_expression
		;

	unary_expression
		= postfix_expression
		| rt(t_inc_op) + unary_expression
		| rt(t_dec_op) + unary_expression
		| unary_operator + cast_expression
		| rt(t_sizeof) + unary_expression
		| rt(t_sizeof) + rt(t_round_left) + type_name + rt(t_round_right)
		| rt(t_alignof) + rt(t_round_left) + type_name + rt(t_round_right)
		;

	postfix_expression
		= primary_expression
		| postfix_expression + rt(t_square_left) + expression + rt(t_square_right)
		| postfix_expression + rt(t_round_left) + rt(t_round_right)
		| postfix_expression + rt(t_round_left) + argument_expression_list + rt(t_round_right)
		| postfix_expression + rt(t_dot) + rt(t_identifier)
		| postfix_expression + rt(t_ptr_op) + rt(t_identifier)
		| postfix_expression + rt(t_inc_op)
		| postfix_expression + rt(t_dec_op)
		| rt(t_round_left) + type_name + rt(t_round_right) + rt(t_brace_left) + initializer_list + rt(t_brace_right)
		| rt(t_round_left) + type_name + rt(t_round_right) + rt(t_brace_left) + initializer_list + rt(t_comma) + rt(t_brace_right)
		;

	primary_expression
		= rt(t_identifier)
		| constant
		| string
		| rt(t_round_left) + expression + rt(t_round_right)
		| generic_selection
		;

	constant
		= rt(t_i_constant)		
		| rt(t_f_constant)
		| rt(t_enumeration_constant)	
		;

	string
		= rt(t_string_literal)
		| rt(t_func_name)
		;

	generic_selection
		= rt(t_generic) + rt(t_round_left) + assignment_expression + rt(t_comma) + generic_assoc_list + rt(t_round_right)
		;

	assignment_expression
		= conditional_expression
		| unary_expression + assignment_operator + assignment_expression
		;

	assignment_operator
		= rt(t_assign)
		| rt(t_mul_assign)
		| rt(t_div_assign)
		| rt(t_mod_assign)
		| rt(t_add_assign)
		| rt(t_sub_assign)
		| rt(t_left_assign)
		| rt(t_right_assign)
		| rt(t_and_assign)
		| rt(t_xor_assign)
		| rt(t_or_assign)
		;

	generic_assoc_list
		= generic_association
		| generic_assoc_list + rt(t_comma) + generic_association
		;

	generic_association
		= type_name + rt(t_colon) + assignment_expression
		| rt(t_default) + rt(t_colon) + assignment_expression
		;

	argument_expression_list
		= assignment_expression
		| argument_expression_list + rt(t_comma) + assignment_expression
		;

	initializer_list
		= designation + initializer
		| initializer
		| initializer_list + rt(t_comma) + designation + initializer
		| initializer_list + rt(t_comma) + initializer
		;


	designation
		= designator_list + rt(t_assign)
		;

	designator_list
		= designator
		| designator_list + designator
		;

	designator
		= rt(t_square_left) + constant_expression + rt(t_square_right)
		| rt(t_dot) + rt(t_identifier)
		;

	initializer
		= rt(t_brace_left) + initializer_list + rt(t_brace_right)
		| rt(t_brace_left) + initializer_list + rt(t_comma) + rt(t_brace_right)
		| assignment_expression
		;

	unary_operator
		= rt(t_ampersand)
		| rt(t_star)
		| rt(t_plus)
		| rt(t_minus)
		| rt(t_tilda)
		| rt(t_exmark)
		;

	expression
		= assignment_expression
		| expression + rt(t_comma) + assignment_expression
		;

	enum_specifier
		= rt(t_enum) + rt(t_brace_left) + enumerator_list + rt(t_brace_right)
		| rt(t_enum) + rt(t_brace_left) + enumerator_list + rt(t_comma) + rt(t_brace_right)
		| rt(t_enum) + rt(t_identifier) + rt(t_brace_left) + enumerator_list + rt(t_brace_right)
		| rt(t_enum) + rt(t_identifier) + rt(t_brace_left) + enumerator_list + rt(t_comma) + rt(t_brace_right)
		| rt(t_enum) + rt(t_identifier)
		;

	enumerator_list
		= enumerator
		| enumerator_list + rt(t_comma) + enumerator
		;

	enumerator	
		= enumeration_constant + rt(t_assign) + constant_expression
		| enumeration_constant
		;

	enumeration_constant		
		= rt(t_identifier) 
		[function_t<c_token>([&enums](parse_tree<c_token>& data) mutable {
			enums.insert(data.last().at(0).data()); 
			return true;
		})]
		;

	type_qualifier
		= rt(t_const)
		| rt(t_restrict)
		| rt(t_volatile)
		| rt(t_atomic)
		;

	function_specifier
		= rt(t_inline)
		| rt(t_noreturn)
		;

	alignment_specifier
		= rt(t_alignas) + rt(t_round_left) + type_name + rt(t_round_right)
		| rt(t_alignas) + rt(t_round_left) + constant_expression + rt(t_round_right)
		;

	declarator
		= pointer + direct_declarator
		| direct_declarator
		;

	direct_declarator
		= rt(t_identifier)
		| rt(t_round_left) + declarator + rt(t_round_right)
		| direct_declarator + rt(t_square_left) + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + rt(t_star) + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + rt(t_static) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + rt(t_static) + assignment_expression + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + type_qualifier_list + rt(t_star) + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + type_qualifier_list + rt(t_static) + assignment_expression + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + type_qualifier_list + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + assignment_expression + rt(t_square_right)
		| direct_declarator + rt(t_round_left) + parameter_type_list + rt(t_round_right)
		| direct_declarator + rt(t_round_left) + rt(t_round_right)
		| direct_declarator + rt(t_round_left) + identifier_list + rt(t_round_right)
		;

	type_qualifier_list
		= type_qualifier
		| type_qualifier_list + type_qualifier
		;

	parameter_type_list
		= parameter_list + rt(t_comma) + rt(t_ellipsis)
		| parameter_list
		;

	parameter_list
		= parameter_declaration
		| parameter_list + rt(t_comma) + parameter_declaration
		;

	parameter_declaration
		= declaration_specifiers + declarator
		| declaration_specifiers + abstract_declarator
		| declaration_specifiers
		;

	abstract_declarator
		= pointer + direct_abstract_declarator
		| pointer
		| direct_abstract_declarator
		;

	direct_abstract_declarator
		= rt(t_round_left) + abstract_declarator + rt(t_round_right)
		| rt(t_square_left) + rt(t_square_right)
		| rt(t_square_left) + rt(t_star) + rt(t_square_right)
		| rt(t_square_left) + rt(t_static) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| rt(t_square_left) + rt(t_static) + assignment_expression + rt(t_square_right)
		| rt(t_square_left) + type_qualifier_list + rt(t_static) + assignment_expression + rt(t_square_right)
		| rt(t_square_left) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| rt(t_square_left) + type_qualifier_list + rt(t_square_right)
		| rt(t_square_left) + assignment_expression + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + rt(t_star) + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + rt(t_static) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + rt(t_static) + assignment_expression + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + type_qualifier_list + rt(t_static) + assignment_expression + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + type_qualifier_list + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + assignment_expression + rt(t_square_right)
		| rt(t_round_left) + rt(t_round_right)
		| rt(t_round_left) + parameter_type_list + rt(t_round_right)
		| direct_abstract_declarator + rt(t_round_left) + rt(t_round_right)
		| direct_abstract_declarator + rt(t_round_left) + parameter_type_list + rt(t_round_right)
		;


	pointer
		= rt(t_star) + type_qualifier_list + pointer
		| rt(t_star) + type_qualifier_list
		| rt(t_star) + pointer
		| rt(t_star)
		;

	identifier_list
		= rt(t_identifier)
		| identifier_list + rt(t_comma) + rt(t_identifier)
		;

	declaration_list
		= declaration
		| declaration_list + declaration
		;

	compound_statement
		= rt(t_brace_left) + rt(t_brace_right)
		| rt(t_brace_left) + block_item_list + rt(t_brace_right)
		;

	block_item_list
		= block_item
		| block_item_list + block_item
		;

	block_item
		= declaration
		| statement
		;

	statement
		= labeled_statement
		| compound_statement
		| expression_statement
		| selection_statement
		| iteration_statement
		| jump_statement
		;

	labeled_statement
		= rt(t_identifier) + rt(t_colon) + statement
		| rt(t_case) + constant_expression + rt(t_colon) + statement
		| rt(t_default) + rt(t_colon) + statement
		;

	expression_statement
		= rt(t_semicolon)
		| expression + rt(t_semicolon)
		;

	selection_statement
		= rt(t_if) + rt(t_round_left) + expression + rt(t_round_right) + statement + rt(t_else) + statement
		| rt(t_if) + rt(t_round_left) + expression + rt(t_round_right) + statement
		| rt(t_switch) + rt(t_round_left) + expression + rt(t_round_right) + statement
		;

	iteration_statement
		= rt(t_while) + rt(t_round_left) + expression + rt(t_round_right) + statement
		| rt(t_do) + statement + rt(t_while) + rt(t_round_left) + expression + rt(t_round_right) + rt(t_semicolon)
		| rt(t_for) + rt(t_round_left) + expression_statement + expression_statement + rt(t_round_right) + statement
		| rt(t_for) + rt(t_round_left) + expression_statement + expression_statement + expression + rt(t_round_right) + statement
		| rt(t_for) + rt(t_round_left) + declaration + expression_statement + rt(t_round_right) + statement
		| rt(t_for) + rt(t_round_left) + declaration + expression_statement + expression + rt(t_round_right) + statement
		;

	jump_statement
		= rt(t_goto) + rt(t_identifier) + rt(t_semicolon)
		| rt(t_continue) + rt(t_semicolon)
		| rt(t_break) + rt(t_semicolon)
		| rt(t_return) + rt(t_semicolon)
		| rt(t_return) + expression + rt(t_semicolon)
		;

	declaration
		= declaration_specifiers + rt(t_semicolon)
		| declaration_specifiers + init_declarator_list + rt(t_semicolon)
		[function_t<c_token>([&typedefs](parse_tree<c_token>& data) mutable {
			parse_tree<c_token>& current = data.last();
			if (current.at(0).begin()->id() == t_typedef) {
				for (size_t i = 0; i < current.at(1).size(); ++i)
					for(auto iter = current.at(1).at(i).begin(); iter != current.at(1).at(i).end(); ++iter) {
						if (iter->id() == t_identifier) {
							typedefs.insert(iter->data());
							break;
						}
				}
			}
			return true;
		})]
		| static_assert_declaration
		;

	init_declarator_list
		= init_declarator
		| init_declarator_list + rt(t_comma) + init_declarator
		;

	init_declarator
		= declarator + rt(t_assign) + initializer
		| declarator
		;

	static_assert_declaration
		= rt(t_static_assert) + rt(t_brace_left) + constant_expression + rt(t_comma) + rt(t_string_literal) + rt(t_brace_right) + rt(t_semicolon);
		;


	lalr_states<c_token> states(program);
	lalr_table<c_token> table(states);
	std::ofstream table_output("table_output.cpp");
	table.to_char_array(table_output, "c_parser", "c_token");
	table_output.close();*/
	lalr_table<c_token> table(c_parser_table, c_parser_size, c_parser_strings, c_parser_functions);

	lalr_parser<c_token, std::string> parser(table);

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
