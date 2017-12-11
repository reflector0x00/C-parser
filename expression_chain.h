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
using function_t = std::function<bool(parse_tree<T>&)>;

template<typename T>
class expression_chain {
	expression_chain* _next;

	bool _always;
	bool _is_terminal;

	parse_rule<T>*	_nterminal;
	T 		_terminal;

	function_t<T> _function;
public:
	expression_chain(parse_rule<T>* nterminal, T terminal = T(), bool always = true, expression_chain* next = nullptr);
	expression_chain* next();
	bool always();
	bool is_terminal();
	T& terminal();
	parse_rule<T>& nterminal();
	std::string to_string();
	void push_back(expression_chain* o);
	void set_function(function_t<T> function);
	bool function(parse_tree<T>& data);
	~expression_chain();
};


template<typename T>
expression_chain<T>::expression_chain(parse_rule<T>* nterminal, T terminal, bool always, expression_chain<T>* next) :
	_next(next),
	_always(always),
	_is_terminal(!nterminal),
	_nterminal(nterminal),
	_terminal(terminal) {
	if (next)
		_function = next->_function;
}


template<typename T>
expression_chain<T>* expression_chain<T>::next() {
	return _next;
}

template<typename T>
bool expression_chain<T>::always() {
	return _always;
}

template<typename T>
bool expression_chain<T>::is_terminal() {
	return _is_terminal;
}

template<typename T>
T& expression_chain<T>::terminal() {
	return _terminal;
}

template<typename T>
parse_rule<T>& expression_chain<T>::nterminal() {
	return *_nterminal;
}

//To-do: delete this
template<typename T>
std::string expression_chain<T>::to_string() {
	return _is_terminal ? (typeid(T) == typeid(char) ? std::string(1, _terminal) : std::to_string(_terminal) ): _nterminal->name();
}

template<typename T>
void expression_chain<T>::push_back(expression_chain* o) {
	if (_function)
		o->_function = _function;
	else if (o->_function)
		set_function(o->_function);

	expression_chain<T>* iter;
	for (iter = this; iter->_next; iter = iter->_next);
	iter->_next = o;

}

template<typename T>
void expression_chain<T>::set_function(function_t<T> function) {
	_function = function;
	for (expression_chain<T>* iter = _next; iter; iter = iter->_next)
		iter->_function = function;
}

template<typename T>
bool expression_chain<T>::function(parse_tree<T>& data) {
	if (_function)
		return _function(data);
	return true;
}

template<typename T>
expression_chain<T>::~expression_chain() {
	delete _next;
}