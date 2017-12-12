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
	rule_expression(rule_expression&& o) {
		std::swap(_expressions, o._expressions);
		_function = o._function;
	}
	rule_expression(parse_rule<T>& rule, bool always = true) {
		_expressions.push_back(new expression_chain<T>(&rule, T(), always));
	}
	rule_expression(std::string id) {
		_expressions.push_back(new expression_chain<T>(nullptr, id[0]));
		for (size_t i = 1; i < id.length(); ++i)
			_expressions[_expressions.size() - 1]->push_back(new expression_chain<T>(nullptr, id[i]));
	}
	rule_expression(T symbol) {
		_expressions.push_back(new expression_chain<T>(nullptr, symbol));
	}
	rule_expression(parse_rule<T>& first, parse_rule<T>& second) {
		expression_chain<T>* last = new expression_chain<T>(&second);
		_expressions.push_back(new expression_chain<T>(&first, T(), true, last));
	}

	void put_ahead(parse_rule<T>& rule) {
		expression_chain<T>* first = new expression_chain<T>(&rule, T(), true, _expressions[_expressions.size() - 1]);
		_expressions[_expressions.size() - 1] = first;
	}
	void put_or_ahead(parse_rule<T>& rule) {
		expression_chain<T>* last = new expression_chain<T>(&rule);
		for (size_t i = 0; i < _expressions.size(); ++i)
			std::swap(last, _expressions[i]);
		_expressions.push_back(last);
	}
	void put_or(T symbol) {
		_expressions.push_back(new expression_chain<T>(nullptr, symbol));
	}
	std::vector<expression_chain<T>*>&& move_expressions() {
		return std::move(_expressions);
	}
	function_t<T> function() {
		return _function;
	}
	void clear() {
		_expressions.clear();
	}

	rule_expression&& operator [](function_t<T> function) {
		_expressions[_expressions.size() - 1]->set_function(function);
		return std::move(*this);
	}
	rule_expression&& operator +(rule_expression&& exp) {
		_expressions[_expressions.size() - 1]->push_back(exp._expressions[0]);
		for (size_t i = 1; i < exp._expressions.size(); ++i)
			_expressions.push_back(exp._expressions[i]);
		exp.clear();
		return std::move(*this);
	}
	rule_expression&& operator +(parse_rule<T>& rule) {
		_expressions[_expressions.size() - 1]->push_back(new expression_chain<T>(&rule));
		return std::move(*this);
	}
	rule_expression&& operator |(rule_expression&& exp) {
		for (auto x : exp._expressions)
			_expressions.push_back(x);
		exp._expressions.clear();
		return std::move(*this);
	}
	rule_expression&& operator |(parse_rule<T>& rule) {
		_expressions.push_back(new expression_chain<T>(&rule));
		return std::move(*this);
	}
	~rule_expression() {
		for (auto x : _expressions)
			delete x;
	}
};


template <typename T>
rule_expression<T> rule_terminal(T symbol) {
	return rule_expression<T>(symbol);
}