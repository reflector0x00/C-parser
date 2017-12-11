#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>


template <typename T>
class expression_chain;

template<typename T>
class rule_expression;

template <typename T>
class parse_tree;

template <typename T>
using function_t = std::function<bool(parse_tree<T>&)>;

template<typename T>
class parse_rule {
	std::string _name;
	std::vector<expression_chain<T>*> _expressions;
public:
	parse_rule(const std::string& name = "XXX");
	void 				print(std::ostream& o);
	std::string 		name();
	size_t 				size();

	bool					operator ==(const parse_rule& o) const;
	expression_chain<T>*	operator [](size_t i);
	parse_rule& 			operator =(parse_rule& o);
	parse_rule& 			operator =(rule_expression<T>&& exp);
	rule_expression<T> 	operator ~();
	rule_expression<T>	operator [](function_t<T> function);
	rule_expression<T> 	operator +(parse_rule& rule);
	rule_expression<T>&& operator +(rule_expression<T>&& exp);
	rule_expression<T> 	operator |(parse_rule& rule);
	rule_expression<T>&& operator |(rule_expression<T>&& exp);
	~parse_rule();
};

template<typename T>
parse_rule<T>::parse_rule(const std::string& name) : _name(name) {}

//TO-DO: delete this
template<typename T>
void parse_rule<T>::print(std::ostream& o) {
	o << _name << " = ";
	for (size_t i = 0; i < _expressions.size(); ++i) {
		if (i)
			o << " | ";
		for (expression_chain<T>* iter = _expressions[i]; iter; iter = iter->next()) {
			if (iter != _expressions[i])
				o << " + ";

			if (!iter->always())
				o << "[";

			if (iter->is_terminal())
				o << "'";

			o << iter->to_string();

			if (iter->is_terminal())
				o << "'";

			if (!iter->always())
				o << "]";
		}
	}
	o << std::endl;
}


template<typename T>
std::string parse_rule<T>::name() {
	return _name;
}

template<typename T>
size_t parse_rule<T>::size() {
	return _expressions.size();
}

template<typename T>
bool parse_rule<T>::operator ==(const parse_rule& o) const {
	//return _name == o._name;
	return this == &o;
}

template<typename T>
expression_chain<T>* parse_rule<T>::operator [](size_t i) {
	return _expressions[i];
}

template<typename T>
parse_rule<T>& parse_rule<T>::operator =(parse_rule& rule) {
	for (auto x : _expressions)
		delete x;
	_expressions.clear();
	_expressions.push_back(new expression_chain<T>(&rule));
	return *this;
}


template<typename T>
parse_rule<T>& parse_rule<T>::operator =(rule_expression<T>&& exp) {
	_expressions = exp.move_expressions();
	exp.clear();
	return *this;
}

template<typename T>
rule_expression<T> parse_rule<T>::operator ~() {
	return rule_expression<T>(*this, false);
}

template<typename T>
rule_expression<T>	parse_rule<T>::operator [](function_t<T> function) {
	return rule_expression<T>(*this)[function];
}

template<typename T>
rule_expression<T> parse_rule<T>::operator +(parse_rule& rule) {
	return rule_expression<T>(*this, rule);
}

template<typename T>
rule_expression<T>&& parse_rule<T>::operator +(rule_expression<T>&& exp) {
	exp.put_ahead(*this);
	return std::move(exp);
}

template<typename T>
rule_expression<T> parse_rule<T>::operator |(parse_rule& rule) {
	return rule_expression<T>(*this) | rule;
}

template<typename T>
rule_expression<T>&& parse_rule<T>::operator |(rule_expression<T>&& exp) {
	exp.put_or_ahead(*this);
	return std::move(exp);
}

template<typename T>
parse_rule<T>::~parse_rule() {
	for (auto& x : _expressions)
		delete x;
}
