#pragma once
#include <vector>
#include <unordered_map>
#include "parse_rule.h"
#include "parse_item.h"


template <typename T>
class parse_state : public std::vector<parse_item<T>> {
	std::unordered_map<parse_item<T>, size_t, parse_item_hasher<T>> _find;
	std::unordered_map<parse_rule<T>*, size_t> _goto_nterminals;
	std::unordered_map<T, size_t> _goto_terminals;
	size_t _kernel;
public:
	void push_back(parse_item<T>& val) {
		std::vector<parse_item<T>>::push_back(val);
		if (val.chain())
			if (val.is_terminal())
				_goto_terminals[val.terminal()];
			else
				_goto_nterminals[&val.nterminal()];
		_find[val] = size() - 1;
	}
	void set_kernel() {
		_kernel = size();
	}
	size_t kernel() const {
		return _kernel;
	}
	bool find(const parse_item<T>& key, size_t& ind) const {
		auto iter = _find.find(key);
		if (iter != _find.end()) {
			ind = iter->second;
			return true;
		}
		return false;
	}
	void set_goto(parse_rule<T>* nterminal, size_t next_state) {
		_goto_nterminals[nterminal] = next_state;
	}
	void set_goto(T terminal, size_t next_state) {
		_goto_terminals[terminal] = next_state;
	}
	std::unordered_map<parse_rule<T>*, size_t>& goto_nterminals() {
		return _goto_nterminals;
	}
	std::unordered_map<T, size_t>& goto_terminals() {
		return _goto_terminals;
	}
};

