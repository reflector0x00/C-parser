#pragma once
#include <stack>
#include <queue>
#include <string>
#include <stdexcept>

#include "lalr_table.h"
#include "parse_tree.h"


//TODO: (make everywhere thi class) Don't, plz
template <typename T>
class alpha {
	//parse_rule<T>* _nterminal;
	std::string _nterminal;
	T _terminal;
public:
	alpha(std::string nterminal) : _nterminal(nterminal), _terminal(T()) {}
	alpha(T terminal) : _terminal(terminal) {}
	bool is_terminal() {
		return _nterminal == std::string();
	}
	std::string nterminal() {
		return _nterminal;
	}
	T terminal() {
		return _terminal;
	}
};


//TO-DO: rebase it
template <typename T, typename Q = std::string>
class lalr_parser {
	std::stack<size_t> _states;
	std::stack<alpha<T>> _symbols;
	std::stack<std::string> _strings;
	std::queue<std::string> _reduce;
	lalr_table<T>* _table;
	parse_tree<T> _tree;
public:
	lalr_parser(lalr_table<T>& table) : _table(&table) {
		_states.push(0);
	}
	bool next(T symbol, std::string data = std::string()) {
		/*		std::cout << _states.top() << "\t";
		if (!_symbols.empty()) {
		if (_symbols.top().is_terminal())
		std::cout << _symbols.top().terminal();
		else
		std::cout << _symbols.top().nterminal()->name();
		}
		else
		std::cout << "<null>";
		std::cout << "\t"
		<< (symbol ? symbol : '$') << "\t";*/

		table_node<T>& node = _table->at(lalr_table_key(_states.top(), symbol));

		switch (node.type) {
		case tnt_error:
			//std::cout << "error" << std::endl;			
			throw std::runtime_error(std::string("Syntax error at symbol ") + std::to_string(symbol));

		case tnt_done:
			//std::cout << "done" << std::endl;
			_tree.done();
			return true;

		case tnt_shift:
			//std::cout << "shift" << std::endl;
			_states.push(node.index);
			_symbols.emplace(symbol);
			_tree.push_back(symbol, data);
			break;

		case tnt_reduce: {
			//std::cout << "reduce" << std::endl;
			std::string convolution;
			for (size_t i = 0; i < node.reduce_size; ++i) {

				if (_symbols.top().is_terminal())
					convolution = std::to_string(_symbols.top().terminal()) + convolution;
				else {
					convolution = _strings.top() + convolution;
					_strings.pop();
				}
				_symbols.pop();
				_states.pop();
			}

			_tree.reduce(node.reduce_size, node.reduce_name);

			_strings.push(convolution);
			_symbols.emplace(node.reduce_name);
			alpha<T>& top = _symbols.top();
			table_node<T>& state_node = _table->at({ _states.top(), std::hash<std::string>()(_symbols.top().nterminal()) }); //TODO: сделай что-нибудь с этим... Пожалуйста
			if (!state_node.type)
				throw std::runtime_error(std::string("Syntax error at symbol ") + std::to_string(symbol));

			if (state_node.type != tnt_state)
				throw std::logic_error("Nterminal node not contain state");

			_states.push(state_node.index);
			if(node.reduce_function)
				if (!node.reduce_function(_tree))
					return true;

			return next(symbol, data); //TODO: без рекурсии
		}

		default:
			throw std::logic_error("Unknown node type");
		}
		return false;
	}
	parse_tree<T>& get_tree() {
		return _tree;
	}
};