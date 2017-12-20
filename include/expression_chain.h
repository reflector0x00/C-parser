#pragma once
#include <string>
#include "parse_rule.h"
#include "rule_expression.h"

template <typename T>
class parse_rule;

template<typename T>
class rule_expression;

template <typename T>
class parse_tree;

template <typename T>
using function_t = std::function<bool(parse_tree<T>&, T&, std::string&)>;

template<typename T>
class expression_chain {
	expression_chain* _next;

	bool _always;
	bool _is_terminal;

	parse_rule<T>*	_nterminal;
	T 		_terminal;

	function_t<T> _function;
public:
	expression_chain(parse_rule<T>* nterminal, T terminal = T(), bool always = true, expression_chain* next = nullptr) :
		_next(next),
		_always(always),
		_is_terminal(!nterminal),
		_nterminal(nterminal),
		_terminal(terminal) {
		if (next)
			_function = next->_function;
	}

	expression_chain* next() {
		return _next;
	}
	bool always() {
		return _always;
	}
	bool is_terminal() {
		return _is_terminal;
	}
	T& terminal() {
		return _terminal;
	}
	parse_rule<T>& nterminal() {
		return *_nterminal;
	}
	//TO-DO: delete this
	std::string to_string() {
		return _is_terminal ? (typeid(T) == typeid(char) ? std::string(1, _terminal) : std::to_string(_terminal)) : _nterminal->name();
	}
	void push_back(expression_chain* o) {
		if (_function)
			o->_function = _function;
		else if (o->_function)
			set_function(o->_function);

		expression_chain<T>* iter;
		for (iter = this; iter->_next; iter = iter->_next);
		iter->_next = o;

	}
	void set_function(function_t<T> function) {
		_function = function;
		for (expression_chain<T>* iter = _next; iter; iter = iter->_next)
			iter->_function = function;
	}
	function_t<T> function() {
		return _function;
	}
	~expression_chain() {
		delete _next;
	}
};