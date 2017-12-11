#pragma once
#include <vector>
#include <string>
#include <functional>
#include "parse_rule.h"
#include "expression_chain.h"


template <typename T>
class expression_chain;

template <typename T>
class parse_rule;

template <typename T>
class parse_tree;

template <typename T>
using function_t = std::function<bool(parse_tree<T>&)>;

template <typename T>
class rule_expression {
	std::vector<expression_chain<T>*> _expressions;
	function_t<T> _function;

public:
	rule_expression(rule_expression&& o);
	rule_expression(parse_rule<T>& rule, bool always = true);
	rule_expression(std::string id);
	rule_expression(T symbol);

	rule_expression(parse_rule<T>& first, parse_rule<T>& second);
	void put_ahead(parse_rule<T>& rule);
	void put_or_ahead(parse_rule<T>& rule);
	void put_or(T symbol);
	std::vector<expression_chain<T>*>&& move_expressions();
	function_t<T> function();
	void clear();

	rule_expression&& operator [](function_t<T> function);
	rule_expression&& operator +(rule_expression&& exp);
	rule_expression&& operator +(parse_rule<T>& rule);
	rule_expression&& operator |(rule_expression&& exp);
	rule_expression&& operator |(parse_rule<T>& rule);
	~rule_expression();
};



template <typename T>
rule_expression<T>::rule_expression(rule_expression&& o) {
	std::swap(_expressions, o._expressions);
	_function = o._function;
}

template <typename T>
rule_expression<T>::rule_expression(parse_rule<T>& rule, bool always) {
	_expressions.push_back(new expression_chain<T>(&rule, T(), always));
}

template <typename T>
rule_expression<T>::rule_expression(std::string id) {
	_expressions.push_back(new expression_chain<T>(nullptr, id[0]));
	for (size_t i = 1; i < id.length(); ++i)
		_expressions[_expressions.size() - 1]->push_back(new expression_chain<T>(nullptr, id[i]));
}

template <typename T>
rule_expression<T>::rule_expression(T symbol) {
	_expressions.push_back(new expression_chain<T>(nullptr, symbol));
}

template <typename T>
rule_expression<T>::rule_expression(parse_rule<T>& first, parse_rule<T>& second) {
	expression_chain<T>* last = new expression_chain<T>(&second);
	_expressions.push_back(new expression_chain<T>(&first, T(), true, last));
}

template <typename T>
void rule_expression<T>::put_ahead(parse_rule<T>& rule) {
	expression_chain<T>* first = new expression_chain<T>(&rule, T(), true, _expressions[_expressions.size() - 1]);
	_expressions[_expressions.size() - 1] = first;
}

template <typename T>
void rule_expression<T>::put_or_ahead(parse_rule<T>& rule) {
	expression_chain<T>* last = new expression_chain<T>(&rule);
	for (size_t i = 0; i < _expressions.size(); ++i)
		std::swap(last, _expressions[i]);
	_expressions.push_back(last);
}

template <typename T>
void rule_expression<T>::put_or(T symbol) {
	_expressions.push_back(new expression_chain<T>(nullptr, symbol));
}

template <typename T>
std::vector<expression_chain<T>*>&& rule_expression<T>::move_expressions() {
	return std::move(_expressions);
}

template <typename T>
function_t<T> rule_expression<T>::function() {
	return _function;
}

template <typename T>
void rule_expression<T>::clear() {
	_expressions.clear();
}

template <typename T>
rule_expression<T>&& rule_expression<T>::operator [](function_t<T> function) {
	_expressions[_expressions.size() - 1]->set_function(function);
	return std::move(*this);
}

template <typename T>
rule_expression<T>&& rule_expression<T>::operator +(rule_expression&& exp) {
	_expressions[_expressions.size() - 1]->push_back(exp._expressions[0]);
	for (size_t i = 1; i < exp._expressions.size(); ++i)
		_expressions.push_back(exp._expressions[i]);
	exp.clear();
	return std::move(*this);
}

template <typename T>
rule_expression<T>&& rule_expression<T>::operator +(parse_rule<T>& rule) {
	_expressions[_expressions.size() - 1]->push_back(new expression_chain<T>(&rule));
	return std::move(*this);
}

template <typename T>
rule_expression<T>&& rule_expression<T>::operator |(rule_expression&& exp) {
	for (auto x : exp._expressions)
		_expressions.push_back(x);
	exp._expressions.clear();
	return std::move(*this);
}

template <typename T>
rule_expression<T>&& rule_expression<T>::operator |(parse_rule<T>& rule) {
	_expressions.push_back(new expression_chain<T>(&rule));
	return std::move(*this);
}

template <typename T>
rule_expression<T>::~rule_expression() {
	for (auto x : _expressions)
		delete x;
}

//TO-DO: del this?
template <typename T>
rule_expression<T> rule_identifier(const std::basic_string<T>& id) {
	return rule_expression<T>(id);
}

template <typename T>
rule_expression<T> rule_chars(const std::basic_string<T>& symbols) {
	rule_expression<T> exp(symbols[0]);
	for (size_t i = 1; i < symbols.length(); ++i)
		exp.put_or(symbols[i]);
	return rule_expression<T>(std::move(exp));
}

template <typename T>
rule_expression<T> rule_term(T symbol) {
	return rule_expression<T>(symbol);
}