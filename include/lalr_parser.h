#pragma once
#include <stack>
#include <queue>
#include <string>
#include <stdexcept>

#include "lalr_table.h"
#include "parse_tree.h"


template <typename T, typename Q = std::string>
class lalr_parser {
	std::stack<size_t> _states;
	lalr_table<T>* _table;
	parse_tree<T> _tree;
public:
	lalr_parser(lalr_table<T>& table) : _table(&table) {
		_states.push(0);
	}
	bool next(T symbol, std::string data = std::string()) {
		
		
		for (bool cont = true; cont; ) {
			table_node<T>& node = _table->at(lalr_table_key(_states.top(), symbol));
			cont = false;
			switch (node.type) {
			case tnt_error:
				throw std::runtime_error(std::string("Syntax error at symbol ") + std::to_string(symbol));

			case tnt_done:
				_tree.done();
				return true;

			case tnt_shift:
				_states.push(node.index);
				_tree.push_back(symbol, data);
				break;

			case tnt_reduce: {
				for (size_t i = 0; i < node.reduce_size; ++i)
					_states.pop();

				_tree.reduce(node.reduce_size, node.reduce_name);

				lalr_table_key key{ _states.top(), std::hash<std::string>()(node.reduce_name) };
				table_node<T>& state_node = _table->at(key);
				if (!state_node.type)
					throw std::runtime_error(std::string("Syntax error at symbol ") + std::to_string(symbol));

				if (state_node.type != tnt_state)
					throw std::logic_error("Nterminal node not contain state");

				_states.push(state_node.index);
				if (node.reduce_function)
					if (!node.reduce_function(_tree, symbol, data))
						return true;

				cont = true;
				break;
			}

			default:
				throw std::logic_error("Unknown node type");
			}
		}
		return false;
	}
	parse_tree<T>& get_tree() {
		return _tree;
	}
};