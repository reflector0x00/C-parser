#pragma once
#include <tuple>
#include <iostream> //not forgete to delete
#include <unordered_map>
#include <stdexcept>

#include "parse_item.h"
#include "lalr_states.h"

enum table_node_type {
	tnt_error,
	tnt_state,
	tnt_shift,
	tnt_reduce,
	tnt_done
};

template <typename T>
struct table_node {
	table_node_type type;
	size_t index;
	parse_item<T>* reduce;
	table_node(table_node_type node_type = tnt_error, size_t state_index = 0, parse_item<T>* reduce_item = nullptr) : type(node_type), index(state_index), reduce(reduce_item) {}
};


typedef std::pair<size_t, size_t> lalr_table_key;


template <typename T>
class lalr_table {
	class state_hash {
		size_t mask;
	public:
		state_hash() {
			mask = 0;
			for (size_t i = 0; i < sizeof(size_t) * 4; ++i)
				mask = (mask << 1) | 1;
		}
		size_t operator()(lalr_table_key key) const {
			return ((key.second & mask) << (sizeof(size_t) * 4)) | (key.first & mask);
		}
	};

	std::unordered_map<lalr_table_key, table_node<T>, state_hash> _table; //TODO: two tables

	void check_and_set(const lalr_table_key& key, table_node<T>&& set) {
		if (!_table[key].type)
			_table[key] = set;
		else {
			if ((_table[key].type == tnt_shift) && (set.type == tnt_shift))
				throw std::runtime_error("Shift-shift conflict in table " + std::to_string(key.first) + " : " + std::to_string(key.second));

			if ((_table[key].type == tnt_reduce) && (set.type == tnt_shift))
				_table[key] = set;
			//TO-DO: alternative
			std::cerr << ("Shift-reduce conflict in table " + std::to_string(key.first) + " : " + std::to_string(key.second)) << std::endl; //TO-DO: the ting goes skrrrra
		}
	}

public:
	lalr_table(lalr_states<T>& states) {
		parse_rule<T>& start = states[0][0].left();
		for (size_t i = 0; i < states.size(); ++i) {

			for (size_t j = 0; j < states[i].size(); ++j)
				if (!states[i][j].chain())
					if (states[i][j].left() == start)
						for (auto& x : states[i][j].preview()) //TO-DO: delete kostil 
							check_and_set(lalr_table_key(i, x), table_node<T>(tnt_done));
					else
						for (auto& x : states[i][j].preview())
							check_and_set(lalr_table_key(i, x), table_node<T>(tnt_reduce, 0, &states[i][j]));



			for (auto& iter : states[i].goto_nterminals())
				check_and_set(lalr_table_key(i, (size_t)iter.first), table_node<T>(tnt_state, iter.second));
			for (auto& iter : states[i].goto_terminals())
				check_and_set(lalr_table_key(i, iter.first), table_node<T>(tnt_shift, iter.second));
		}
	}
	table_node<T>& at(lalr_table_key key) {
		return _table[key];
	}
	table_node<T>& operator [](lalr_table_key key) {
		return _table[key];
	}
};
