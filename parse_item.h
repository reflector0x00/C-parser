#pragma once
#include <string>
#include <set>
#include "parse_rule.h"
#include "expression_chain.h"


template<typename T>
class parse_item {
	size_t _stage;
	parse_rule<T>* _left;
	expression_chain<T>* _right;
	expression_chain<T>* _chain;
	std::set<T> _preview;

public:
	parse_item(parse_rule<T>& left, expression_chain<T>* right, size_t stage = 0)
		: _stage(stage),
		_left(&left),
		_right(right),
		_chain(right) {
		for (size_t i = 0; i < _stage; ++i)
			_chain = _chain->next();
	}
	parse_item(parse_rule<T>& left, expression_chain<T>* right, const std::set<T>& preview, size_t stage = 0)
		: _stage(stage),
		_left(&left),
		_right(right),
		_chain(right),
		_preview(preview) {
		for (size_t i = 0; i < _stage; ++i)
			_chain = _chain->next();
	}

	parse_item(parse_rule<T>& left, expression_chain<T>* right, std::set<T>&& preview, size_t stage = 0)
		: _stage(stage),
		_left(&left),
		_right(right),
		_chain(right),
		_preview(std::move(preview)) {
		for (size_t i = 0; i < _stage; ++i)
			_chain = _chain->next();
	}

	bool assign(const T& preview) {
		return _preview.insert(preview).second;
	}

	bool assign(const std::set<T>& preview) {
		bool f = false;
		for (auto iter : preview)
			f |= _preview.insert(iter).second;
		return f;
	}

	parse_rule<T>& left() const {
		return *_left;
	}

	expression_chain<T>* right() const {
		return _right;
	}

	expression_chain<T>* chain() const {
		return _chain;
	}
	bool is_terminal() {
		return _chain->is_terminal();
	}
	std::set<T>& preview() {
		return _preview;
	}
	const std::set<T>& preview() const {
		return _preview;
	}
	size_t stage() {
		return _stage;
	}
	parse_item<T> next() {
		return parse_item<T>(*_left, _right, _preview, _stage + 1);
	}
	T terminal() {
		return _chain->terminal();
	}
	parse_rule<T>& nterminal() {
		return _chain->nterminal();
	}
	bool operator ==(const parse_item<T>& o) const {
		return 	(_stage == o._stage) &&
			(_left == o._left) &&
			(_right == o._right) &&
			(_chain == o._chain);
	}
};

//From http://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
template <typename SizeT>
inline void hash_combine(SizeT& seed, SizeT value) {
	seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename T>
class parse_item_hasher {
public:
	size_t operator()(const parse_item<T>& key) const {
		size_t result = (size_t)key.right();
		hash_combine(result, (size_t)key.chain());
		return result;
	}
};