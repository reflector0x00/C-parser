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
	std::string reduce_name;
	size_t reduce_size;
	function_t<T> reduce_function;
	table_node(table_node_type _type, size_t _index, std::string _reduce_name, size_t _reduce_size, function_t<T> _reduce_function) : type(_type), index(_index), reduce_name(_reduce_name), reduce_size(_reduce_size), reduce_function(_reduce_function) {}
	table_node(table_node_type node_type = tnt_error, size_t state_index = 0, parse_item<T>* reduce_item = nullptr) : type(node_type), index(state_index), reduce_size(0) {
		if (reduce_item) {
			reduce_name = reduce_item->left().name();
			reduce_size = reduce_item->stage();
			reduce_function = reduce_item->right()->function();
			index = (size_t)&reduce_item->left();
		}
	}
	bool operator ==(const table_node<T>& o) const {
		return (type == o.type) && (index == o.index) && (reduce_name == o.reduce_name) && (reduce_size == o.reduce_size);
	}
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
	struct lalr_table_data {
		uint64_t index_i;
		uint64_t index_j;
		table_node_type type;
		uint64_t index;
		uint64_t reduce_name;
		uint64_t reduce_size;
		uint64_t reduce_function;
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
	void write_chars(std::ostream& o, const void* data, size_t n, bool end = true) {
		char* ptr = (char*)data;
		for (size_t i = 0; i < n; ++i) {
			o << "0x" << ((size_t)ptr[i] & 0xFF);
			if ((end) || (i + 1 != n))
				o << ", ";
		}
	}
public:
	//lalr_table(const std::initializer_list<const std::pair<lalr_table_key, table_node<T>>>& table) : _table(table.begin(), table.end()) {}
	lalr_table(const unsigned char* data, const uint64_t data_size, const std::string* strings, const function_t<T>* functions) {
		lalr_table_data* iter = (lalr_table_data*)data;
		for (size_t i = 0; i < data_size; ++i, ++iter) {
			std::string reduce_name;
			function_t<T> reduce_function;
			if (iter->reduce_name != (uint64_t)-1)
				reduce_name = strings[iter->reduce_name];
			if (iter->reduce_function)
				reduce_function = functions[iter->reduce_function];
			_table[lalr_table_key(iter->index_i, iter->index_j)] = table_node<T>(iter->type, iter->index, reduce_name, iter->reduce_size, reduce_function);
		}			
	}

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
				check_and_set(lalr_table_key(i, std::hash<std::string>()(iter.first->name())), table_node<T>(tnt_state, iter.second));
			for (auto& iter : states[i].goto_terminals())
				check_and_set(lalr_table_key(i, iter.first), table_node<T>(tnt_shift, iter.second));
		}
	}
	void to_char_array(std::ostream& o, const std::string& name, std::string type) {
		std::unordered_map<size_t, size_t> functions;
		for (auto& iter : _table)
			if (iter.second.reduce_function) {
				auto x = functions.find(iter.second.index);
				if (x == functions.end()) {
					size_t n = functions.size();
					functions[iter.second.index] = n;
				}
			}

		std::vector<std::string> strings;
		std::unordered_map<std::string, size_t> strings_find;
		for (auto& iter : _table)
			if (!iter.second.reduce_name.empty()) {
				auto x = strings_find.find(iter.second.reduce_name);
				if (x == strings_find.end()) {
					size_t n = strings_find.size();
					strings_find[iter.second.reduce_name] = n;
					strings.push_back(iter.second.reduce_name);
				}
			}

		o << "const std::string " << name << "_strings[] = { ";
		for (auto& x : strings) {
			if (x != *strings.begin())
				o << ", ";
			o << "\"" << x << "\"";
		}
		o << "};" << std::endl;

		o << "function_t<" << type << "> " << name << "_functions[" << functions.size() <<  "];" << std::endl;
		o << "const uint64_t " << name << "_size = " << _table.size() << ";" << std::endl;
		o << "const unsigned char " << name << "_table[] = {" << std::endl;
		o << std::hex;
		for (auto& iter : _table) {
			if (iter != *_table.begin())
				o << ", " << std::endl;
			uint64_t temp;
			
			temp = iter.first.first;
			write_chars(o, &temp, sizeof(temp));
			
			temp = iter.first.second;
			write_chars(o, &temp, sizeof(temp));
			
			temp = iter.second.type;
			write_chars(o, &temp, sizeof(temp));
			
			temp = iter.second.index;
			write_chars(o, &temp, sizeof(temp));
			
			temp = -1;
			if (!iter.second.reduce_name.empty())
				temp = strings_find[iter.second.reduce_name];
			write_chars(o, &temp, sizeof(temp));

			temp = iter.second.reduce_size;
			write_chars(o, &temp, sizeof(temp));

			temp = 0;
			if (iter.second.reduce_function) 
				temp = functions[iter.second.index];
			write_chars(o, &temp, sizeof(temp), false);
		}
		o << std::dec;
		o << std::endl << "};";
	}
	table_node<T>& at(lalr_table_key key) {
		return _table[key];
	}
	table_node<T>& operator [](lalr_table_key key) {
		return _table[key];
	}
};
