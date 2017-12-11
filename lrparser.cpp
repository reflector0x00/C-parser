#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <list>
#include <regex>
#include <fstream>
#include "parse_rule.h"
#include "rule_expression.h"
#include "expression_chain.h"

#define rt(x) rule_term(x)


//From http://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
template <typename SizeT>
inline void hash_combine(SizeT& seed, SizeT value)
{
	seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename T>
class item {
	size_t _stage;
	parse_rule<T>* _left;
	expression_chain<T>* _right;
	expression_chain<T>* _chain;
	std::set<T> _preview;

public:
	item(parse_rule<T>& left, expression_chain<T>* right, size_t stage = 0) 
		: _stage(stage), 
		_left(&left), 
		_right(right), 
		_chain(right) {
		for (size_t i = 0; i < _stage; ++i)
			_chain = _chain->next();
	}
	item(parse_rule<T>& left, expression_chain<T>* right, const std::set<T>& preview, size_t stage = 0)
		: _stage(stage),
		_left(&left),
		_right(right),
		_chain(right),
		_preview(preview) {
		for (size_t i = 0; i < _stage; ++i)
			_chain = _chain->next();
	}

	item(parse_rule<T>& left, expression_chain<T>* right, std::set<T>&& preview, size_t stage = 0)
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
		for(auto iter : preview)
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
	const std::set<T>& preview() const{
		return _preview;
	}
	size_t stage() {
		return _stage;
	}
	item<T> next() {
		return item<T>(*_left, _right, _preview, _stage + 1);
	}
	T terminal() {
		return _chain->terminal();
	}
	parse_rule<T>& nterminal() {
		return _chain->nterminal();
	}
	bool operator ==(const item<T>& o) const {
		return 	(_stage == o._stage) &&
			(_left == o._left) &&
			(_right == o._right) &&
			(_chain == o._chain); /*&&
			(_preview == o._preview);*/
	}
};


template <typename T>
class item_hasher {
//	size_t mask;
//	 hv;
public:
/*	item_hasher() {
		mask = 0;
		for (size_t i = 0; i < sizeof(size_t) * 2; ++i)
			mask = (mask << 1) | 1;
	}*/
	size_t operator()(const item<T>& key) const {
		size_t result = (size_t)key.right();
		//hash_combine(result, (size_t)&key.left());
		hash_combine(result, (size_t)key.chain());		
		//size_t vhash = std::hash<std::vector<T>>()();
		
		return result;
			
				/* (((size_t)(&key.left()) & mask) << sizeof(size_t) * 6)
				| (((size_t)key.right() & mask) << sizeof(size_t) * 4)
				| (((size_t)key.chain() & mask) << sizeof(size_t) * 2)
				| (hv(key.preview() & mask);*/
	}
};


template <typename T>
class state : public std::vector<item<T>> {
	


	std::unordered_map<item<T>, size_t, item_hasher<T>> _find;
	std::unordered_map<parse_rule<T>*, size_t> _goto_nterminals;
	std::unordered_map<T, size_t> _goto_terminals;
	size_t _kernel;
public:
	void push_back(item<T>& val) {
		std::vector<item<T>>::push_back(val);
		if(val.chain())
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
	bool find(const item<T>& key, size_t& ind) const {
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
		/*
	void add_goto(parse_rule<T>* nterminal, size_t next_state) {
		_goto_nterminals.push_back(std::pair<parse_rule<T>*, size_t>(nterminal, next_state));
	}
	void add_goto(T terminal, size_t next_state) {
		_goto_terminals.push_back(std::pair<T, size_t>(terminal, next_state));
	}*/
	/*
	bool is_exist(parse_rule<T>* nterminal) {
		for (auto element : _goto_nterminals)
			if (element.first == nterminal)
				return true;
		return false;
	}
	bool is_exist(T terminal) {
		for (auto element : _goto_terminals)
			if (element.first == terminal)
				return true;
		return false;
	}
	*/
	/*
	std::pair<parse_rule<T>*, size_t> goto_nterminal(size_t i) {
		return _goto_nterminals.begin() + i;
	}
	std::pair<T, size_t> goto_terminal(size_t i) {
		return _goto_terminals[i];
	}*/
};


template <typename T>
class kernel_item_hasher {
	//	size_t mask;
	//	 hv;
public:
	/*	item_hasher() {
	mask = 0;
	for (size_t i = 0; i < sizeof(size_t) * 2; ++i)
	mask = (mask << 1) | 1;
	}*/
	size_t operator()(const item<T>& key) const {
		size_t result = 0;
		//bool f = false;
		//hash_combine(result, (size_t)&key.left());
		hash_combine(result, (size_t)key.right());
		//if (f) std::cout << result << std::endl;
		hash_combine(result, (size_t)key.chain());
		//if (f) std::cout << result << std::endl << std::endl;
		for (auto& x : key.preview()) {
			//result ^= std::hash<T>()(x);
			hash_combine(result, std::hash<T>()(x));
		//	if (f) std::cout << result << std::endl;
		}
		//if (f) std::cout << std::endl;

		//size_t vhash = std::hash<std::vector<T>>()();

		return result;

		/* (((size_t)(&key.left()) & mask) << sizeof(size_t) * 6)
		| (((size_t)key.right() & mask) << sizeof(size_t) * 4)
		| (((size_t)key.chain() & mask) << sizeof(size_t) * 2)
		| (hv(key.preview() & mask);*/
	}
};



template <typename T>
class lr_states {
	class kernel_hasher {
		item_hasher<T> h;
	public:
		size_t operator()(const state<T>* key) const {
			size_t r = 0;
			for (size_t i = 0; i < key->kernel(); ++i)
				r ^= h((*key)[i]);
				//hash_combine(r, h((*key)[i]));
			return r;
		}
	};
	class kernel_eq2 {
	public:
		bool operator()(const state<T>* lhs, const state<T>* rhs) const {
			if (lhs->kernel() != rhs->kernel())
				return false;
			for (size_t i = 0; i < lhs->kernel(); ++i) {
				size_t ind;
				if (!lhs->find((*rhs)[i], ind))
					return false;
				if (ind >= lhs->kernel())
					return false;
				if ((*lhs)[ind].preview() != (*rhs)[i].preview())
					return false;
			}
			return true;
		}

	};

	class kernel_eq {
	public:
		bool operator()(const state<T>* lhs, const state<T>* rhs) const {
			if (lhs->kernel() != rhs->kernel())
				return false;
			for (size_t i = 0; i < lhs->kernel(); ++i) {
				size_t ind;
				if (!lhs->find((*rhs)[i], ind))
					return false;
				if (ind >= lhs->kernel())
					return false;
			}
			return true;
		}
		
	};

	std::vector<state<T>*> _states;
	std::unordered_map<parse_rule<T>*, std::set<T>> _first;
	std::unordered_map<state<T>*, size_t, kernel_hasher, kernel_eq> _kernels;
	/*template <typename Q>
	size_t vector_find(std::vector<Q>& vector, const Q& element) { //TO-DO: Negage notation
		for (size_t i = 0; i < vector.size(); ++i)
			if (vector[i] == element)
				return i + 1;
		return 0;
	}*/
	void closure(state<T>& vector) {
		//std::vector<expression_chain<T>*> nterminals;
		//std::vector<T> terminals;
		std::queue<size_t> queue;
		std::unordered_set<size_t> queued;
		for (size_t i = 0; i < vector.size(); ++i) {
			queue.push(i);
			queued.insert(i);
		}
		
		while(!queue.empty()) {
			size_t i = queue.front();
			queue.pop();
			queued.erase(i);

			if ((!vector[i].chain()) || (vector[i].is_terminal()))
				continue;
			
			expression_chain<T>* beta = vector[i].chain()->next();
			if ((!beta) || (beta->is_terminal())) {
				/*T preview_symbol = T();
				std::vector<T>& preview = vector[i].preview();
				if (beta)
					preview_symbol = beta->terminal();
				*/
				
				parse_rule<T>& sub = vector[i].chain()->nterminal();
				if (beta)  //TO-DO: ustal_kostil
					for (size_t j = 0; j < sub.size(); ++j) {
						item<T> temp(sub, sub[j]);
						size_t ind;
						if (vector.find(temp, ind)) {
							if (vector[ind].assign(beta->terminal()) && (queued.find(ind) == queued.end())) {
								queue.push(ind);
								queued.insert(ind);
							}
						}
						else {
							temp.assign(beta->terminal());
							vector.push_back(temp);
							ind = vector.size() - 1;
							if (queued.find(ind) == queued.end()) {
								queue.push(ind);
								queued.insert(ind);
							}
						}
					}
				else
					for (size_t j = 0; j < sub.size(); ++j) {
						item<T> temp(sub, sub[j]);
						size_t ind;
						if (vector.find(temp, ind)) {
							if(vector[ind].assign(vector[i].preview()) && (queued.find(ind) == queued.end())) {
								queue.push(ind);
								queued.insert(ind);
							}
						}
						else {
							temp.assign(vector[i].preview());
							vector.push_back(temp);
							ind = vector.size() - 1;
							if (queued.find(ind) == queued.end()) {
								queue.push(ind);
								queued.insert(ind);
							}
						}

					}

			}
			else {
				/*nterminals.clear();
				terminals.clear();

				nterminals.push_back(beta);
				for (size_t j = 0; j < nterminals.size(); ++j) {
					if (nterminals[j]->is_terminal())
						terminals.push_back(nterminals[j]->terminal());
					else { //TO-DO: один раз рассчитывать терминалы
						parse_rule<T>& rule = nterminals[j]->nterminal();
						for (size_t k = 0; k < rule.size(); ++k) {
							if (vector_find(nterminals, rule[k]))
								break;
							nterminals.push_back(rule[k]);
						}
					}
				}*/
				parse_rule<T>& sub = vector[i].chain()->nterminal();
				for (size_t j = 0; j < sub.size(); ++j) {
					//std::unordered_set<T>& first = _first[&vector[i].nterminal()];
					//std::vector<T> temp_first(first.size());
					//std::copy(first.begin(), first.end(), temp_first.begin());
					item<T> temp(sub, sub[j]);
					size_t ind;
					if (vector.find(temp, ind)) {
						if (vector[ind].assign(_first[&beta->nterminal()]) && (queued.find(ind) == queued.end())) {
							queue.push(ind);
							queued.insert(ind);
						}
					}
					else {
						temp.assign(_first[&beta->nterminal()]);
						vector.push_back(temp);
						ind = vector.size() - 1;
						if (queued.find(ind) == queued.end()) {
							queue.push(ind);
							queued.insert(ind);
						}
					}
				}
					/*for (auto symbol : ) {
						item<T> temp(sub, sub[j], symbol);
						size_t ind;
						if (vector.find(temp, ind))
							continue;
						vector.push_back(temp);
					}*/
			}
		}
	}
	void _goto(state<T>& result, state<T>& current, T terminal) {
		for (size_t i = 0; i < current.size(); ++i) {
			if (!current[i].chain())
				continue;
			if (current[i].is_terminal() && (current[i].terminal() == terminal))
				result.push_back(current[i].next());
		}
		result.set_kernel();
		//closure(result);
	}
	void _goto(state<T>& result, state<T>& current, parse_rule<T>& nterminal) {
		for (size_t i = 0; i < current.size(); ++i) {
			if (!current[i].chain())
				continue;
			if ((!current[i].is_terminal()) && (current[i].nterminal() == nterminal))
				result.push_back(current[i].next());
		}
		result.set_kernel();
		//closure(result);
	}
	void build_first(parse_rule<T>& rule) { //TO-DO: queue -> un_set
		std::queue<parse_rule<T>*> queue;	//Очередь из нетерминалов, которые следует обработать (которые не являются первыми символами какого-либо правила)
		std::list<parse_rule<T>*> stack_symbols;	//Стек из символов, служащий для рекурсивного спуска по первым нетерминалам
		std::stack<size_t> stack_indexes;			//Стек индексов правил нетерминалов прошлого стека
		std::unordered_set<parse_rule<T>*> processed;		//Хэш-таблица для быстрого отсечения уже обработанных(-ываемых) элементов
		
		//Помещаем входной нетерминал
		queue.push(&rule);
		//До тех пор, пока очередь не пуста
		while (!queue.empty()) {	
			//Извлекаем из очереди нетерминал
			parse_rule<T>& next_symbol = *queue.front(); 
			queue.pop();

			//Если он уже обработан(-ывается), то пропускаем 
			if (processed.find(&next_symbol) != processed.end())
				continue;

			
			processed.insert(&next_symbol);			//Иначе помечаем как обрабатываемый
			stack_symbols.push_back(&next_symbol);	//Помещаем его в вершину одного стека
			stack_indexes.push(0);					//И ноль в вершину другого
			
			//До тех пор, пока стек не пуст
			while (!stack_symbols.empty()) {

				//Смотрим на вершины стеков
				parse_rule<T>& left = *stack_symbols.back();
				size_t& i = stack_indexes.top();
	
				//Если еще не все правила обработаны
				if (i != left.size()) {
					//Если первый элемент правила терминал
					if (left[i]->is_terminal()) {
						//То проходимся по всему "стеку", чтобы добавить еще один first-символ
						for (auto& x : stack_symbols) 
							_first[x].insert(left[i]->terminal());
						++i;
					}
					//Если нетерминал, то проверяем, был ли он уже обработан, чтобы не уйти в бесконечную рекурсию
					else if (processed.find(&left[i]->nterminal()) == processed.end()) { //TO-DO: one time find for last branches
						processed.insert(&left[i]->nterminal());		//Помечаем его как обрабатываемый 
						stack_symbols.push_back(&left[i]->nterminal()); //Помещаем его в вершину стека
						++i;											//Не забываем увеличить индекс
						stack_indexes.push(0);							//И помещаем новый в стек
					}
					else {
						for (auto& x : stack_symbols)
							for (auto& y : _first[&left[i]->nterminal()])
								_first[x].insert(y);
						++i;
					}
				}
				//После обработки всех правил
				else {
					//Проходимся еще раз по всем правилам
					for (i = 0; i < left.size(); ++i)
						//В каждом правиле по всем элементам цепочки
						for(expression_chain<T>* iter = left[i]->next(); iter; iter = iter->next())
							//И если элемент является нетерминалом и еще не помечен как обработанный(-ывающийся), то добавляем в очередь
							if ((!iter->is_terminal()) && (processed.find(&iter->nterminal()) == processed.end()))
								queue.push(&iter->nterminal());
					//И поднимаемся из рекурсии
					stack_symbols.pop_back();
					stack_indexes.pop();
				}

			}
			
		}
		/*for (auto x : _first) {
			std::cout << x.first->name() << " : ";
			for (auto y : x.second)
				std::cout << y << " ";
			std::cout << std::endl;
		}*/
	}
public:
	lr_states(parse_rule<T>& start) {
		build_first(start);
		//TO-DO: добавлять нулевое правило не извне, а здесь
		state<T>* first = new state<T>;
		item<T> first_item(start, start[0]);
		first_item.assign(T());
		first->push_back(first_item); //Первое правило
		first->set_kernel();
		_kernels[first] = 0;

		
		
		closure(*first);
		_states.push_back(first);

		std::queue<size_t> queue;
		std::unordered_set<size_t> queued;
		queue.push(0);
		queued.insert(0);
//		bool needtoprint = false;
		//for (size_t i = 0; i < _states.size(); ++i) {
//			
		while(!queue.empty()) {
			size_t i = queue.front();
			queue.pop();
			queued.erase(i);


//			std::cout << (100. * i / (double) _states.size()) << std::endl;
//			if(needtoprint)
//				to_dot(std::cout);
			
			for (auto& terminal : _states[i]->goto_terminals()) {
				state<T> temp;
				_goto(temp, *_states[i], terminal.first);
				auto check = _kernels.find(&temp);
				if (check != _kernels.end()) {
					_states[i]->set_goto(terminal.first, check->second);
					//kernel_eq2 eq;
					//if(!eq(temp, check->first))
					
					bool changed = false;
					state<T>& s = *check->first;
					for (size_t j = 0; j < temp.kernel(); ++j) {
						size_t ind;
						s.find(temp[j], ind);
						changed |= s[ind].assign(temp[j].preview());
					}
					if (changed) {
						closure(s);
						if (queued.find(check->second) == queued.end()) {
							queue.push(check->second);
							queued.insert(check->second);
						}

						/*for (auto& x : s.goto_nterminals())
							if ((x.second) && (queued.find(x.second) == queued.end())) {
								queue.push(x.second);
								queued.insert(x.second);
							}
						for (auto& x : s.goto_terminals())
							if ((x.second) && (queued.find(x.second) == queued.end())) {
								queue.push(x.second);
								queued.insert(x.second);
							}*/
					}
				}
				else {
					closure(temp);
					_states[i]->set_goto(terminal.first, _states.size()); //TO-DO: set directly?
					_states.push_back(new state<T>(std::move(temp)));
					size_t ind = _states.size() - 1;
					_kernels[_states[ind]] = ind;
					queue.push(ind);
					queued.insert(ind);
				}

			}
			
			for (auto& nterminal : _states[i]->goto_nterminals()) {
				
				
				state<T> temp;
				_goto(temp, *_states[i], *nterminal.first);
				auto check = _kernels.find(&temp);
				if (check != _kernels.end()) {
					_states[i]->set_goto(nterminal.first, check->second);
					bool changed = false;
					state<T>& s = *check->first;
					for (size_t j = 0; j < temp.kernel(); ++j) {
						size_t ind;
						s.find(temp[j], ind);
						changed |= s[ind].assign(temp[j].preview());
					}
					if (changed) {
						closure(s);
						if (queued.find(check->second) == queued.end()) {
							queue.push(check->second);
							queued.insert(check->second);
						}

						/*for (auto& x : s.goto_nterminals())
							if ((x.second) && (queued.find(x.second) == queued.end())) {
								queue.push(x.second);
								queued.insert(x.second);
							}
						for (auto& x : s.goto_terminals())
							if ((x.second) && (queued.find(x.second) == queued.end())) {
								queue.push(x.second);
								queued.insert(x.second);
							}*/
					}
				}
				else {
					closure(temp);
					_states[i]->set_goto(nterminal.first, _states.size());
					_states.push_back(new state<T>(std::move(temp)));
					size_t ind = _states.size() - 1;
					_kernels[_states[ind]] = ind;
					queue.push(ind);
					queued.insert(ind);
				}

			}
			
			/*
			for (size_t j = 0; j < _states[i].size(); ++j) {
				item<T>& current = _states[i][j];
				expression_chain<T>* chain = current.chain();
				if (!chain)
					continue;
				if (chain->is_terminal()) {
					T terminal = chain->terminal();
					if (_states[i].is_exist(terminal))
						continue;
					state<T> temp;
					_goto(temp, _states[i], terminal);
					if (temp.empty())
						continue;
					size_t ind = vector_find(_states, temp);
					if (ind)
						_states[i].add_goto(terminal, ind - 1);
					else {
						_states[i].add_goto(terminal, _states.size());
						_states.push_back(std::move(temp));
					}
				}
				else {
					parse_rule<T>& nterminal = chain->nterminal();
					if (_states[i].is_exist(&nterminal))
						continue;
					state<T> temp;
					_goto(temp, _states[i], nterminal);
					if (temp.empty())
						continue;
					size_t ind = vector_find(_states, temp);
					if (ind)
						_states[i].add_goto(&nterminal, ind - 1);
					else {
						_states[i].add_goto(&nterminal, _states.size());
						_states.push_back(std::move(temp));
					}
				}
			}*/
			
		}

	}
	void to_dot(std::ostream& o) {

		o << "digraph states {" << std::endl
			<< "\trankdir=LR;" << std::endl;
		for (size_t i = 0; i < _states.size(); ++i) {
			state<T>& current = *_states[i];
			o << "\ts" << i << " [ label = \"state " << i;
			for (size_t j = 0; j < current.size(); ++j) {
				o << "\\n[" << current[j].left().name() << " -> ";
				for (expression_chain<T>* iter = current[j].right(); iter; iter = iter->next()) {
					if (iter == current[j].chain())
						o << ". ";
					o << iter->to_string() << " ";
				}
				if (!current[j].chain())
					o << ".";
				o << ", ";
				std::set<T>& preview = current[j].preview();
				for (auto& p : preview) {
					if(p != *preview.begin())
						o << "/";
					
					if(p)
						o << p;
					else 
						o << '$';
				}
				o << "]";
			}
			o << "\"];" << std::endl;
		}
		o << std::endl;
		for (size_t i = 0; i < _states.size(); ++i) {
			for (auto& nterminal : _states[i]->goto_nterminals())
				o << "\ts" << i << " -> " << "s" << nterminal.second
				<< " [ label = \"" << nterminal.first->name() << "\"]" << std::endl;
			for (auto& terminal : _states[i]->goto_terminals())
				o << "\ts" << i << " -> " << "s" << terminal.second
				<< " [ label = \"'" << terminal.first << "'\"]" << std::endl;
		}
		o << "}" << std::endl;
	}
	size_t size() {
		return _states.size();
	}
	state<T>& operator[](size_t i) {
		return *_states[i];
	}
};

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
	item<T>* reduce;
	//table_node() : _type(tnt_error), _index(0), _item(nullptr) {}
	table_node(table_node_type node_type = tnt_error, size_t state_index = 0, item<T>* reduce_item = nullptr) : type(node_type), index(state_index), reduce(reduce_item) {}

};

typedef std::pair<size_t, size_t> key_t;

template <typename T>
class lr_table {
	class state_hash {
		size_t mask;
	public:
		state_hash() {
			mask = 0;
			for (size_t i = 0; i < sizeof(size_t) * 4; ++i)
				mask = (mask << 1) | 1;
		}
		size_t operator()(key_t key) const {
			return ((key.second & mask) << (sizeof(size_t) * 4)) | (key.first & mask);
		}
	};
	std::unordered_map<key_t, table_node<T>, state_hash> _table; //TODO: two tables
	void check_and_set(const key_t& key, table_node<T>&& set) {
		if (!_table[key].type)
			_table[key] = set;
		else {
			if ((_table[key].type == tnt_shift) && (set.type == tnt_shift))
				throw std::runtime_error("Shift-shift conflict in table " + std::to_string(key.first) + " : " + std::to_string(key.second));

			if ((_table[key].type == tnt_reduce) && (set.type == tnt_shift))
				_table[key] = set;
			std::cerr << ("Shift-reduce conflict in table " + std::to_string(key.first) + " : " + std::to_string(key.second)) << std::endl; //TO-DO: the ting goes skrrrra
		}
	}
public:
	lr_table(lr_states<T>& states) {
		parse_rule<T>& start = states[0][0].left();
		for (size_t i = 0; i < states.size(); ++i) {

			for (size_t j = 0; j < states[i].size(); ++j)
				if (!states[i][j].chain())
					if (states[i][j].left() == start)
						for(auto& x : states[i][j].preview()) //TO-DO: delete kostil 
							check_and_set(key_t(i, x), table_node<T>(tnt_done));
					else
						for (auto& x : states[i][j].preview())
							check_and_set(key_t(i, x), table_node<T>(tnt_reduce, 0, &states[i][j]));



			for (auto& iter : states[i].goto_nterminals())
				check_and_set(key_t(i, (size_t)iter.first), table_node<T>(tnt_state, iter.second));
			for (auto& iter : states[i].goto_terminals())
				check_and_set(key_t(i, iter.first), table_node<T>(tnt_shift, iter.second));
		}
	}
	table_node<T>& at(key_t key) {
		return _table[key];
	}
	table_node<T>& operator [](key_t key) {
		return _table[key];
	}
};


//TODO: сделать везде этот класс
template <typename T>
class alpha {
	parse_rule<T>* _nterminal;
	T _terminal;
public:
	alpha(parse_rule<T>* nterminal) : _nterminal(nterminal), _terminal(T()) {}
	alpha(T terminal) : _nterminal(nullptr), _terminal(terminal) {}
	bool is_terminal() {
		return !_nterminal;
	}
	parse_rule<T>* nterminal() {
		return _nterminal;
	}
	T terminal() {
		return _terminal;
	}
};

template <typename T>
class leaf_iterator {
	std::stack<parse_tree<T>*> _stack_nodes;
	std::stack<size_t> _stack_inds;
	parse_tree<T>* _current;
public:
	leaf_iterator() : _current(nullptr) {}
	leaf_iterator(parse_tree<T>* root) {
		for (_current = &(root->at(0)); !_current->leaf(); _current = &(_current->at(0))) {
			_stack_nodes.push(_current);
			_stack_inds.push(0);
		}
	}
	leaf_iterator& operator ++() {
		bool here = false; 
		while (!here) {
			if (_stack_nodes.empty()) {
				_current = nullptr;
				return *this;
			}

			parse_tree<T>* node = _stack_nodes.top();
			size_t& ind = _stack_inds.top();
			++ind;
			if (ind < node->size())
				here = true; 
			else {
				_stack_nodes.pop();
				_stack_inds.pop();
			}
				
		}
		parse_tree<T>* node = _stack_nodes.top();
		size_t& ind = _stack_inds.top();
		for (_current = &(node->at(ind)); !_current->leaf(); _current = &(_current->at(0))) {
			_stack_nodes.push(_current);
			_stack_inds.push(0);
		}
		return *this;
	}
	parse_tree<T>& operator *() {
		return *_current;
	}
	parse_tree<T>* operator ->() {
		return _current;
	}
	bool operator ==(const leaf_iterator& o) const {
		return _current == o._current;
	}
	bool operator !=(const leaf_iterator& o) const {
		return _current != o._current;
	}
};

template <typename T>
class parse_tree {
	std::string _node_name;
	parse_tree<T>* _root;
	bool _leaf;
	T _id;
	std::string _data;
	std::vector<parse_tree<T>*> _childrens;
public:
	parse_tree() : _node_name("root"), _root(nullptr), _leaf(false) {}
	parse_tree(parse_tree<T>* root, std::string name) : _node_name(name), _root(root), _leaf(false) {}
	parse_tree(T id, std::string data, parse_tree<T>* root) : _root(root), _leaf(true), _id(id), _data(data) {}
	void done() {
		parse_tree<T>* next = *_childrens.begin();
		_childrens.erase(_childrens.begin());
		_node_name = next->_node_name;
		for (auto x : next->_childrens) {
			x->_root = this;
			_childrens.push_back(x);
		}
		next->_childrens.clear();
		delete next;
	}
	//TO-DO: вывести to_dot оттдельно
	void to_dot(std::ostream& o) {
		o << std::hex << "digraph program {" << std::endl;
		std::queue<parse_tree<T>*> queue;
		queue.push(this);
		while (!queue.empty()) {
			parse_tree<T>* next = queue.front();
			queue.pop();
			o << "\tnode_" << next << " [ label = \"";
			if (next->_leaf) {
				std::string temp = next->_data;
				if (temp[0] == '\"') {
					temp.insert(temp.begin(), '\\');
					temp.insert(temp.begin() + temp.length() - 1, '\\');
				}
				o << temp << "\" color = blue] " << std::endl;
			}
			else
				o << next->_node_name << "\" color = red] " << std::endl;

			for (auto x : next->_childrens) {
				queue.push(x);
				o << "\tnode_" << next << " -> " << "node_" << x << std::endl;
			}
		}
		o << "}" << std::dec;
	}
	parse_tree<T>& last() {
		return *(*_childrens.rbegin());
	}
	parse_tree<T>& at(size_t i) {
		return *(_childrens[i]);
	}
	std::string data() {
		return _data;
	}
	T id() {
		return _id;
	}
	size_t size() {
		return _childrens.size();
	}
	bool leaf() {
		return _leaf;
	}
	void push_back(T id, std::string data) {
		_childrens.push_back(new parse_tree<T>(id, data, this));
	}
	void reduce(size_t n, std::string name) {
		parse_tree<T>* temp = new parse_tree<T>(this, name);
		for (auto iter = _childrens.begin() + _childrens.size() - n; iter != _childrens.end(); ++iter) {
			(*iter)->_root = temp;
			temp->_childrens.push_back(*iter);
		}
		_childrens.erase(_childrens.begin() + _childrens.size() - n, _childrens.end());
		_childrens.push_back(temp);
	}
	leaf_iterator<T> begin() {
		return leaf_iterator<T>(this);
	}
	leaf_iterator<T> end() {
		return leaf_iterator<T>();
	}
	~parse_tree() {
		for (auto x : _childrens)
			delete x;
	}
};


template <typename T, typename Q = std::string>
class lr {
	std::stack<size_t> _states;
	std::stack<alpha<T>> _symbols;
	std::stack<std::string> _strings;
	std::queue<std::string> _reduce;
	lr_table<T>* _table;
	parse_tree<T> _tree;
public:
	lr(lr_table<T>& table) : _table(&table) {
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

		table_node<T>& node = _table->at(key_t(_states.top(), symbol));

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
			for (size_t i = 0; i < node.reduce->stage(); ++i) {
				//_reduce.push(_symbols.top());

				if (_symbols.top().is_terminal())
					//_strings.push(std::string(1, _symbols.top());
					convolution = std::to_string(_symbols.top().terminal()) + convolution;
				else {
					convolution = _strings.top() + convolution;
					_strings.pop();
				}
				_symbols.pop();
				_states.pop();
			}

			_tree.reduce(node.reduce->stage(), node.reduce->left().name());

			_strings.push(convolution);
			_symbols.emplace(&(node.reduce->left()));
			//std::cout << "IT:";
			//std::cout << _symbols.top().is_terminal() << std::endl;
			//std::cout << "curstate:" << _states.top() << " curnterm: " << _symbols.top().nterminal() << std::endl;
			alpha<T>& top = _symbols.top();
			table_node<T>& state_node = _table->at(key_t(_states.top(), (size_t)_symbols.top().nterminal())); //TODO: сделай что-нибудь с этим... Пожалуйста
			if (!state_node.type)
				throw std::runtime_error(std::string("Syntax error at symbol ") + std::to_string(symbol));

			if (state_node.type != tnt_state)
				throw std::logic_error("Nterminal node not contain state");

			_states.push(state_node.index);

			if (!node.reduce->right()->function(_tree))
				//if(!_symbols.top().nterminal()->function(convolution))
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
	/*
	bool parse(std::string input) {
		for (size_t i = 0; i < input.length(); ++i)
			next(input[i]);
		return next(0);
	}*/

};

/*
class tree_vector {
	std::stack<size_t> ns;
public:
	bool add_n(std::string s) {
		ns.push(s[0] - '0');
		return true;
	}
	bool sum(std::string s) {
		size_t a = ns.top();
		ns.pop();
		size_t b = ns.top();
		ns.pop();
		ns.push(a + b);

		return true;
	}
	void print() {
		while (!ns.empty()) {
			//for(auto& x : ns)
			std::cout << ns.top() << " ";
			ns.pop();
		}
	}
};*/
/*
template<typename T, typename Q>
auto bind(T function, Q object) {
return std::bind(function, object, std::placeholders::_1);
}
*/

typedef std::list<std::string>::iterator lexem_iterator;
typedef std::function<size_t(std::string)> lexem_func;

class lexem_regular {
	std::regex _regex;
	lexem_func _function;
public:
	lexem_regular(const std::regex& regex, lexem_func func) : _regex(regex), _function(func) {}
	std::regex& regex() {
		return _regex;
	}
	lexem_func& function() {
		return _function;
	}
};


class token {
	size_t _index;
	std::string _data;
public:
	token(size_t index, std::string data = std::string()) : _index(index), _data(data) {}
	size_t index() {
		return _index;
	}
	std::string data() {
		return _data;
	}
};


class lexer {
	std::unordered_set<char> _empty_dividers;
	std::unordered_set<char> _flags;
	std::vector<std::pair<std::string, std::string>> _commentary;
	std::vector<std::string> _dividers;
	std::unordered_map<std::string, lexem_func> _keywords;
	std::vector<lexem_regular> _regulars;
	std::vector<token> _tokens;
	std::istream* _stream;
	bool _escape;
	std::list<std::string> _substrings;
	char _flag;
	int _comment;
	std::string _line;
	std::string _buffer;
	/*std::vector<std::pair<std::string, >> _empty_dividers;
	std::vector<std::string> _dividers;
	std::vector<> _regulars;
	std::unordered_map<std::string, > constants; */
	std::string preprocess() {
		std::string token_string;

		if (!_buffer.empty()) {
			token_string = std::move(_buffer);
			_buffer.clear();
			return token_string;
		}

		while (_stream->good()) {
			char sym = _stream->get();
			if (!(*_stream))
				break;
			/*				if (_line.empty())
								return token(0);
							else
								break;
								*/
			if ((_escape) && (sym == '\\')) {
				_line += sym;
				_line += _stream->get(); //avoiding preprocessing first escape-symbol
				if (!_stream)
					throw std::runtime_error("File ended at escape-symbol");
				continue;
			}

			if (_flags.find(sym) != _flags.end()) {
				if (_flag == sym) {
					_line += sym;
					token_string = std::move(_line);
					_line.clear();
					_flag = 0;
					break;
				}
				else if (!_flag) {
					if (!_line.empty())
						token_string = std::move(_line);
					_line = std::string(1, sym);
					_flag = sym;

					if (!token_string.empty())
						break;
					else
						continue;
				}
			}

			_line += sym;


			bool f = false;
			if (!_flag)
				for (size_t i = 0; i < _commentary.size(); ++i) {
					std::string& first = _commentary[i].first;
					if (sym != first[0])
						continue;
					//auto left = _line.rbegin();
					//auto right = first.begin();
					//for (; (right != first.end()) /*&& (left != _line.rend()*/); /*++left, */++right) {
					size_t pos = _stream->tellg();
					size_t j;
					for (j = 1; j < first.length(); ++j) {
						char x = _stream->get();
						if (!(*_stream))
							break;
						if (x != first[j])
							break;
					}
						//if (*left != *right)
					
					if (j != first.length()) {
						_stream->seekg(pos);
						continue;
					}

					f = true;

					std::string& second = _commentary[i].second;
					for (int j = 0; j < second.length(); ++j) {
						char x = _stream->get();
						if (!(*_stream))
							throw std::runtime_error("Comment not closed");
						if (x != second[j])
							j = -1;
					}

					//for (auto x : first)
						_line.pop_back();

					if (!_line.empty()) {
						token_string = std::move(_line);
						_line.clear();
					}
					break;
				}

			if (!token_string.empty())
				break;
			else if (f)
				continue;

			if ((!_flag) && (_empty_dividers.find(sym) != _empty_dividers.end())) {
				_line.pop_back();
				if (!_line.empty()) {
					token_string = std::move(_line);
					_line.clear();
					break;
				}
				continue;
			}

			f = false;
			if (!_flag)
				for (size_t i = 0; i < _dividers.size(); ++i) {
					if (sym != _dividers[i][0])
						continue;

					size_t pos = _stream->tellg();
					size_t j;
					for (j = 1; j < _dividers[i].length(); ++j) {
						char x = _stream->get();
						if (!(*_stream))
							break;
						if (x != _dividers[i][j])
							break;
					}
					//if (*left != *right)

					if (j != _dividers[i].length()) {
						_stream->seekg(pos);
						continue;
					}

					f = true;



					/*
					
					auto left = _line.rbegin();
					auto right = _dividers[i].rbegin();

					for (; (right != _dividers[i].rend()) && (left != _line.rend()); ++left, ++right)
						if (*left != *right)
							break;

					if (right != _dividers[i].rend())
						continue;

					f = true;
					*/
					//_line = _line.substr(0, _line.length() - _dividers[i].length());
					_line.pop_back();

					if (!_line.empty()) {
						token_string = std::move(_line);
						_line.clear();
						_buffer = _dividers[i];
					}
					else 
						token_string = _dividers[i];
					break;
				}


			if (!token_string.empty())
				break;
			else if (f)
				continue;
		}
		return token_string;
		/*
		size_t j = 0;
		size_t i;
		for (size_t i = 0; i < _commentary.size(); ++i) {
			size_t b;
			while ((b = line.find(_commentary[i].first)) != 0) {
				if()
			
			}


		}

		for (i = 0; i < line.length(); ++i) {
			//std::cout << "\"" << line[i]
			if ((_escape) && (line[i] == '\\')) {
				if (++i == line.length())
					throw std::runtime_error("End of line at escape-symbol");
				
				continue;
			}
			
				
			if(_comment == -1)
			if (_flags.find(line[i]) != _flags.end()) {
				if (_flag == line[i]) {
					std::string temp = line.substr(j, i - j + 1);
					//std::cout << "<" << temp << ">" << std::endl;
					if (!temp.empty())
						_substrings.push_back(temp);
					j = i + 1;
					_flag = 0;
				}
				else if (!_flag) {
					std::string temp = line.substr(j, i - j);
					if (!temp.empty())
						_substrings.push_back(temp);
					j = i;
					_flag = line[i];
				}
			}

			//jddaib
			if (!_flag)
				if (_comment == -1) 
					for (size_t k = 0; k < _commentary.size(); ++k) {
						size_t len = _commentary[k].first.length() - 1;
						if ((i < len) || (i - j < len))
							continue;
						if (line.substr(i - len) == _flags[k].first) {
							_flag = k;
							std::string temp = line.substr(j, i - j);
							if (!temp.empty())
								_substrings.push_back(temp);
							if (_dividers.find(_flags[k].first) != _dividers.end()) {
								_substrings.push_back(_flags[k].first);
								j = i + 1;
							}
							else
								j = i - len;
						}
					}
					else {
						size_t len = _flags[k].second.length() - 1;
						if ((i < len) || (i - j < len))
							continue;
						if (line.substr(i - len) == _flags[k].second) {
							_flag = -1;
							if (_dividers.find(_flags[k].second) != _dividers.end()) {
								std::string temp = line.substr(j, i - j - len);
								if (!temp.empty())
									_substrings.push_back(temp);
								_substrings.push_back(_flags[k].second);
								j = i + 1;
							}
							else {
								std::string temp = line.substr(j, i - j + 1);
								if (!temp.empty())
									_substrings.push_back(temp);
								j = i - len;
							}
						}
					}


			if ((_flag == -1) && (_empty_dividers.find(line[i]) != _empty_dividers.end())) {
				std::string temp = line.substr(j, i - j);
				if (!temp.empty())
					_substrings.push_back(temp);
				j = i + 1;
				continue;
			}
		}
		std::string temp = line.substr(j, i - j);
		if (!temp.empty())
			_substrings.push_back(temp);

		//if (_flag)
		//	throw std::runtime_error(std::string("Literal symbol ") + std::to_string(_flag) + " is not paired"); //TO-DO: add normal conversion


		for (auto iter = _substrings.begin(); iter != _substrings.end(); ++iter) {
			bool f = true;
			for(auto& x : _flags) 
				if (iter->substr(0, x.first.length()) == x.first) {
					f = false;
					break;
				}

			if (f)
				for (auto& divider : _dividers) {
					if (*(iter) == divider) //TO-DO: unordered set
						break;

					size_t pos = iter->find(divider);
					if (pos != std::string::npos) {
						auto copy_iter(iter);
						_substrings.insert(++copy_iter, divider);
						std::string temp = iter->substr(pos + divider.length());
						if (!temp.empty())
							_substrings.insert(copy_iter, temp);
						//auto copy_iter_2(copy_iter);
						temp = iter->substr(0, pos);
						if (!temp.empty())
							*iter = temp;
						else {
							_substrings.erase(iter--);
							break;
						}
					}
				}
		}*/
	}


public:  
	lexer(std::string empty_dividers, std::string flags, std::vector<std::pair<std::string, std::string>> commentary, bool escape, std::vector<std::string>&& dividers) : _commentary(commentary), _dividers(dividers), _escape(escape), _flag(0), _comment(-1) {
		for (auto x : flags)
			_flags.insert(x);
		/*for (auto x : dividers)
			_dividers.insert(x);*/
		for (auto x : empty_dividers)
			_empty_dividers.insert(x);
		//std::cout << _empty_dividers.size() << std::endl;
	}
	void set_stream(std::istream& stream) {
		_stream = &stream;
		/*while (stream.good()) {
			std::string line;
			std::getline(stream, line);
			line.push_back('\n');
			preprocess(line);
		}*/
	}
	void add_keyword(std::string keyword, lexem_func function) {
		_keywords[keyword] = function;
	}
	void add_regular(const std::regex& regular, lexem_func function) {
		_regulars.emplace_back(regular, function);
	}
	token next() {
		std::string token_string(preprocess());
		if (token_string.empty())
			return token(0);

		auto keyword = _keywords.find(token_string);
		if (keyword != _keywords.end()) {
			size_t ind;
			if (keyword->second) {
				ind = keyword->second(token_string);
				if (ind) {
					token t = token(ind, token_string);
					return t;
				}
				else return next(); //TO-DO: without recoursion
			}
		}


		for (auto& regular : _regulars) {
			if (regex_match(token_string, regular.regex())) {
				size_t ind;
				if (regular.function()) {
					ind = regular.function()(token_string);
					if (ind) {
						token t = token(ind, token_string);
						return t;
					}
					else return next(); //TO-DO: without recoursion
				}
				break;
			}
		}

		throw std::runtime_error("Unknown token: " + token_string);

	}


	/*
	auto iter = _substrings.begin();
	auto keyword = _keywords.find(*iter);
	if (keyword != _keywords.end()) {
	size_t ind;
	if (keyword->second) {
	ind = keyword->second(iter);
	if (ind) {
	token t = token(ind, *iter);
	_substrings.erase(_substrings.begin(), ++iter);
	return t;
	}
	else {
	_substrings.erase(_substrings.begin(), ++iter);
	return next(); //TO-DO: without recoursion
	}
	}
	}

	bool f = true;
	for (auto& regular : _regulars) {
	if (regex_match(*iter, regular.regex())) {
	size_t ind;
	if (regular.function()) {
	ind = regular.function()(iter);
	if (ind) {
	token t = token(ind, *iter);
	_substrings.erase(_substrings.begin(), ++iter);
	return t;
	}
	else {
	_substrings.erase(_substrings.begin(), ++iter);
	return next(); //TO-DO: without recoursion
	}
	}
	//f = false;
	break;
	}
	}
	if (f)
	throw std::runtime_error("Unknown token: " + *iter);
	*/

	/*void parse(std::string line) {
		//std::string buffer;
		size_t j = 0;
		size_t i;
		char flag = 0;
		for (i = 0; i < line.length(); ++i) {
			//std::cout << "\"" << line[i]
			if ((_escape) && (line[i] == '\\')) {
				if (++i == line.length())
					throw std::runtime_error("End of line at escape-symbol");
				continue;
			}

			if (_flags.find(line[i]) != _flags.end()) {
				if (flag == line[i]) {
					std::string temp = line.substr(j, i - j + 1);
					std::cout << "<" << temp << ">" << std::endl;
					if (!temp.empty())
						substrings.push_back(temp);
					j = i + 1;
					flag = 0;
				}
				else if (!flag) {
					std::string temp = line.substr(j, i - j);
					if (!temp.empty())
						substrings.push_back(temp);
					j = i;
					flag = line[i];
				}
			}

			if ((!flag) && (_empty_dividers.find(line[i]) != _empty_dividers.end())) {
				std::string temp = line.substr(j, i - j);
				if (!temp.empty())
					substrings.push_back(temp);
				j = i + 1;
				continue;
			}
		}
		std::string temp = line.substr(j, i - j);
		if (!temp.empty())
			substrings.push_back(temp);

		if (flag)
			throw std::runtime_error(std::string("Literal symbol ") + flag + " is not paired");

		for (auto x : substrings)
			std::cout << x << std::endl;
		std::cout << "---------" << std::endl;

		//	i = 0;
		for (auto iter = substrings.begin(); iter != substrings.end(); ++iter) {
			if (_flags.find((*iter)[0]) == _flags.end())
				for (auto& divider : _dividers) {
					/*std::cout << "> " << divider << " " << *iter << std::endl;
					for(auto x : substrings)
					std::cout << x << std::endl;
					std::cout << "---------" << std::endl;
					/
					
					if (*iter == divider)
						break;

					size_t pos = iter->find(divider);
					if (pos != std::string::npos) {
						auto copy_iter(iter);
						substrings.insert(++copy_iter, divider);
						std::string temp = iter->substr(pos + divider.length());
						if (!temp.empty())
							substrings.insert(copy_iter, temp);
						//auto copy_iter_2(copy_iter);
						temp = iter->substr(0, pos);
						if (!temp.empty())
							*iter = temp;
						else {
							substrings.erase(iter--);
							break;
						}
					}

				}
		}

		for (auto iter = substrings.begin(); iter != substrings.end(); ++iter) {

			auto keyword = _keywords.find(*iter);
			if (keyword != _keywords.end()) {
				size_t ind;
				if (keyword->second) {
					ind = keyword->second(iter);
					if (ind)
						_tokens.emplace_back(ind, *iter);
				}
				continue;
			}

			bool f = true;
			for (auto& regular : _regulars) {
				if (regex_match(*iter, regular.regex())) {
					size_t ind;
					if (regular.function()) {
						ind = regular.function()(iter);
						if (ind)
							_tokens.emplace_back(ind, *iter);
					}
					f = false;
					break;
				}
			}
			if (f)
				throw std::runtime_error("Unknown token: " + *iter);
		}


		for (auto x : _tokens) {
			std::cout << x.index() << " : " << x.data() << std::endl;
		}
		/*for(auto x : substrings)
		std::cout << x << std::endl;/
	}*/
	bool eof() {
		return !_stream->operator bool();
	}
	std::vector<token>& get_tokens() {
		return _tokens;
	}
};

enum c_token {
	t_empty,
	t_auto,
	t_break,
	t_case,
	t_char,
	t_const,
	t_continue,
	t_default,
	t_do,
	t_double,
	t_else,
	t_enum,
	t_extern,
	t_float,
	t_for,
	t_goto,
	t_if,
	t_inline,
	t_int,
	t_long,
	t_register,
	t_restrict,
	t_return,
	t_short,
	t_signed,
	t_sizeof,
	t_static,
	t_struct,
	t_switch,
	t_typedef,
	t_union,
	t_unsigned,
	t_void,
	t_volatile,
	t_while,
	t_alignas,
	t_alignof,
	t_atomic,
	t_bool,
	t_complex,
	t_generic,
	t_imaginary,
	t_noreturn,
	t_static_assert,
	t_thread_local,
	t_func_name,
	t_identifier,
	t_i_constant,
	t_f_constant,
	t_string_literal,
	t_ellipsis,
	t_right_assign,
	t_left_assign,
	t_add_assign,
	t_sub_assign,
	t_mul_assign,
	t_div_assign,
	t_mod_assign,
	t_and_assign,
	t_xor_assign,
	t_or_assign,
	t_right_op,
	t_left_op,
	t_inc_op,
	t_dec_op,
	t_ptr_op,
	t_and_op,
	t_or_op,
	t_le_op,
	t_ge_op,
	t_eq_op,
	t_ne_op,
	t_semicolon,
	t_brace_left,
	t_brace_right,
	t_comma,
	t_colon,
	t_assign,
	t_round_left,
	t_round_right,
	t_square_left,
	t_square_right,
	t_dot,
	t_ampersand,
	t_exmark,
	t_tilda,
	t_minus,
	t_plus,
	t_star,
	t_slash,
	t_percent,
	t_angle_left,
	t_angle_right,
	t_carrete,
	t_vertical,
	t_qmark,
	t_typedef_name,
	t_enumeration_constant
};


int main(int argc, char* argv[]) {

	if (argc != 3) {
		std::cout << "usage:" << std::endl
			<< "prog input_c_file output_gv_file" << std::endl;
		return 1;
	}


	std::unordered_set<std::string> typedefs;
	std::unordered_set<std::string> enums;

	lexer lex(" \t\n", "'\"",
	{
		{"//", "\n"},
		{"/*", "*/"}
	},
		true,
	{
		"...",
		">>=",
		"<<=",
		"+=",
		"-=",
		"*=",
		"/=",
		"%=",
		"&=",
		"^=",
		"|=",
		">>",
		"<<",
		"++",
		"--",
		"->",
		"&&",
		"||",
		"<=",
		">=",
		"==",
		"!=",
		";",
		"{",
		"<%",
		"}",
		"%>",
		",",
		":",
		"=",
		"(",
		")",
		"[",
		"<:",
		"]",
		":>",
		".",
		"&",
		"!",
		"~",
		"-",
		"+",
		"*",
		"/",
		"%",
		"<",
		">",
		"^",
		"|",
		"?"
	});

	std::string r_O("[0-7]");
	std::string r_D("[0-9]");
	std::string r_NZ("[1-9]");
	std::string r_L("[a-zA-Z_]");
	std::string r_A("[a-zA-Z_0-9]");
	std::string r_H("[a-fA-F0-9]");
	std::string r_HP("(0[xX])");
	std::string r_E("([Ee][+-]?[0-9]+)");
	std::string r_P("([Pp][+-]?[0-9]+)");
	std::string r_FS("(f|F|l|L)");
	std::string r_IS("(((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))");
	std::string r_CP("(u|U|L)");
	std::string r_SP("(u8|u|U|L)");
	std::string r_ES("(\\\\(['\"\\?\\\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))");
	std::string r_WS("[ \t\v\n\f]");


//	lex.add_keyword("/*", [](std::string& name) { while (*name != "*/") ++name; return 0; });
//	lex.add_keyword("//", [](std::string& name) {while (*name != "\n") ++name; return 0; });

	lex.add_keyword("auto", [](std::string& name) { return t_auto; });
	lex.add_keyword("break", [](std::string& name) { return t_break; });
	lex.add_keyword("case", [](std::string& name) { return t_case; });
	lex.add_keyword("char", [](std::string& name) { return t_char; });
	lex.add_keyword("const", [](std::string& name) { return t_const; });
	lex.add_keyword("continue", [](std::string& name) { return t_continue; });
	lex.add_keyword("default", [](std::string& name) { return t_default; });
	lex.add_keyword("do", [](std::string& name) { return t_do; });
	lex.add_keyword("double", [](std::string& name) { return t_double; });
	lex.add_keyword("else", [](std::string& name) { return t_else; });
	lex.add_keyword("enum", [](std::string& name) { return t_enum; });
	lex.add_keyword("extern", [](std::string& name) { return t_extern; });
	lex.add_keyword("float", [](std::string& name) { return t_float; });
	lex.add_keyword("for", [](std::string& name) { return t_for; });
	lex.add_keyword("goto", [](std::string& name) { return t_goto; });
	lex.add_keyword("if", [](std::string& name) { return t_if; });
	lex.add_keyword("inline", [](std::string& name) { return t_inline; });
	lex.add_keyword("int", [](std::string& name) { return t_int; });
	lex.add_keyword("long", [](std::string& name) { return t_long; });
	lex.add_keyword("register", [](std::string& name) { return t_register; });
	lex.add_keyword("restrict", [](std::string& name) { return t_restrict; });
	lex.add_keyword("return", [](std::string& name) { return t_return; });
	lex.add_keyword("short", [](std::string& name) { return t_short; });
	lex.add_keyword("signed", [](std::string& name) { return t_signed; });
	lex.add_keyword("sizeof", [](std::string& name) { return t_sizeof; });
	lex.add_keyword("static", [](std::string& name) { return t_static; });
	lex.add_keyword("struct", [](std::string& name) { return t_struct; });
	lex.add_keyword("switch", [](std::string& name) { return t_switch; });
	lex.add_keyword("typedef", [](std::string& name) { return t_typedef; });
	lex.add_keyword("union", [](std::string& name) { return t_union; });
	lex.add_keyword("unsigned", [](std::string& name) { return t_unsigned; });
	lex.add_keyword("void", [](std::string& name) { return t_void; });
	lex.add_keyword("volatile", [](std::string& name) { return t_volatile; });
	lex.add_keyword("while", [](std::string& name) { return t_while; });
	lex.add_keyword("_Alignas", [](std::string& name) { return t_alignas; });
	lex.add_keyword("_Alignof", [](std::string& name) { return t_alignof; });
	lex.add_keyword("_Atomic", [](std::string& name) { return t_atomic; });
	lex.add_keyword("_Bool", [](std::string& name) { return t_bool; });
	lex.add_keyword("_Complex", [](std::string& name) { return t_complex; });
	lex.add_keyword("_Generic", [](std::string& name) { return t_generic; });
	lex.add_keyword("_Imaginary", [](std::string& name) { return t_imaginary; });
	lex.add_keyword("_Noreturn", [](std::string& name) { return t_noreturn; });
	lex.add_keyword("_Static_assert", [](std::string& name) { return t_static_assert; });
	lex.add_keyword("_Thread_local", [](std::string& name) { return t_thread_local; });
	lex.add_keyword("__func__", [](std::string& name) { return t_func_name; });

	lex.add_regular(std::regex(r_L + r_A + "*"), [&typedefs, &enums](std::string& name) { 
		if (typedefs.find(name) != typedefs.end())
			return t_typedef_name;
		if (enums.find(name) != enums.end())
			return t_enumeration_constant;
		return t_identifier; 
	});

	lex.add_regular(std::regex(r_HP + r_H + "+" + r_IS + "?"), [](std::string& name) { return t_i_constant; });
	lex.add_regular(std::regex(r_NZ + r_D + "*" + r_IS + "?"), [](std::string& name) { return t_i_constant; });
	lex.add_regular(std::regex("0" + r_O + "*" + r_IS + "?"), [](std::string& name) { return t_i_constant; });
	lex.add_regular(std::regex(r_CP + "?'([^'\\\n]|" + r_ES + ")+'"), [](std::string& name) { return t_i_constant; });

	lex.add_regular(std::regex(r_D + "+" + r_E + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_D + "*\\." + r_D + "+" + r_E + "?" + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_D + "+\\." + r_E + "?" + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_D + "+\\." + r_E + "?" + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_HP + r_H + "+" + r_P + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_HP + r_H + "*\\." + r_H + "+" + r_P + r_FS + "?"), [](std::string& name) { return t_f_constant; });
	lex.add_regular(std::regex(r_HP + r_H + "+\\." + r_P + r_FS + "?"), [](std::string& name) { return t_f_constant; });

	lex.add_regular(std::regex("(" + r_SP + "?\\\"([^\"\\\n]|" + r_ES + ")*\\\"" + r_WS + "*)+"), [](std::string& name) { return t_string_literal; });

	lex.add_keyword("...", [](std::string& name) { return t_ellipsis; });
	lex.add_keyword(">>=", [](std::string& name) { return t_right_assign; });
	lex.add_keyword("<<=", [](std::string& name) { return t_left_assign; });
	lex.add_keyword("+=", [](std::string& name) { return t_add_assign; });
	lex.add_keyword("-=", [](std::string& name) { return t_sub_assign; });
	lex.add_keyword("*=", [](std::string& name) { return t_mul_assign; });
	lex.add_keyword("/=", [](std::string& name) { return t_div_assign; });
	lex.add_keyword("%=", [](std::string& name) { return t_mod_assign; });
	lex.add_keyword("&=", [](std::string& name) { return t_and_assign; });
	lex.add_keyword("^=", [](std::string& name) { return t_xor_assign; });
	lex.add_keyword("|=", [](std::string& name) { return t_or_assign; });
	lex.add_keyword(">>", [](std::string& name) { return t_right_op; });
	lex.add_keyword("<<", [](std::string& name) { return t_left_op; });
	lex.add_keyword("++", [](std::string& name) { return t_inc_op; });
	lex.add_keyword("--", [](std::string& name) { return t_dec_op; });
	lex.add_keyword("->", [](std::string& name) { return t_ptr_op; });
	lex.add_keyword("&&", [](std::string& name) { return t_and_op; });
	lex.add_keyword("||", [](std::string& name) { return t_or_op; });
	lex.add_keyword("<=", [](std::string& name) { return t_le_op; });
	lex.add_keyword(">=", [](std::string& name) { return t_ge_op; });
	lex.add_keyword("==", [](std::string& name) { return t_eq_op; });
	lex.add_keyword("!=", [](std::string& name) { return t_ne_op; });
	lex.add_keyword(";", [](std::string& name) { return t_semicolon; });
	lex.add_keyword("{", [](std::string& name) { return t_brace_left; });
	lex.add_keyword("<%", [](std::string& name) { return t_brace_left; });
	lex.add_keyword("}", [](std::string& name) { return t_brace_right; });
	lex.add_keyword("%>", [](std::string& name) { return t_brace_right; });
	lex.add_keyword(",", [](std::string& name) { return t_comma; });
	lex.add_keyword(":", [](std::string& name) { return t_colon; });
	lex.add_keyword("=", [](std::string& name) { return t_assign; });
	lex.add_keyword("(", [](std::string& name) { return t_round_left; });
	lex.add_keyword(")", [](std::string& name) { return t_round_right; });
	lex.add_keyword("[", [](std::string& name) { return t_square_left; });
	lex.add_keyword("<:", [](std::string& name) { return t_square_left; });
	lex.add_keyword("]", [](std::string& name) { return t_square_right; });
	lex.add_keyword(":>", [](std::string& name) { return t_square_right; });
	lex.add_keyword(".", [](std::string& name) { return t_dot; });
	lex.add_keyword("&", [](std::string& name) { return t_ampersand; });
	lex.add_keyword("!", [](std::string& name) { return t_exmark; });
	lex.add_keyword("~", [](std::string& name) { return t_tilda; });
	lex.add_keyword("-", [](std::string& name) { return t_minus; });
	lex.add_keyword("+", [](std::string& name) { return t_plus; });
	lex.add_keyword("*", [](std::string& name) { return t_star; });
	lex.add_keyword("/", [](std::string& name) { return t_slash; });
	lex.add_keyword("%", [](std::string& name) { return t_percent; });
	lex.add_keyword("<", [](std::string& name) { return t_angle_left; });
	lex.add_keyword(">", [](std::string& name) { return t_angle_right; });
	lex.add_keyword("^", [](std::string& name) { return t_carrete; });
	lex.add_keyword("|", [](std::string& name) { return t_vertical; });
	lex.add_keyword("?", [](std::string& name) { return t_qmark; });


	lex.add_keyword("\n", [](std::string& name) { return 0; });
	lex.add_keyword(" ", [](std::string& name) { return 0; });

	//TO-DO: change this name and put far away
#define debug(x) x(#x)

	parse_rule<c_token> 			
		debug(program),
		debug(translation_unit),
		debug(external_declaration),
		debug(function_definition),
		debug(declaration_specifiers),
		debug(storage_class_specifier),
		debug(type_specifier),
		debug(atomic_type_specifier),
		debug(type_name),
		debug(struct_or_union_specifier),
		debug(struct_or_union),
		debug(struct_declaration_list),
		debug(struct_declaration),
		debug(specifier_qualifier_list),
		debug(struct_declarator_list),
		debug(struct_declarator),
		debug(constant_expression),
		debug(conditional_expression),
		debug(logical_or_expression),
		debug(logical_and_expression),
		debug(inclusive_or_expression),
		debug(exclusive_or_expression),
		debug(and_expression),
		debug(equality_expression),
		debug(relational_expression),
		debug(shift_expression),
		debug(additive_expression),
		debug(multiplicative_expression),
		debug(cast_expression),
		debug(unary_expression),
		debug(postfix_expression),
		debug(primary_expression),
		debug(constant),
		debug(string),
		debug(generic_selection),
		debug(assignment_expression),
		debug(assignment_operator),
		debug(generic_assoc_list),
		debug(generic_association),
		debug(argument_expression_list),
		debug(initializer_list),
		debug(designation),
		debug(designator_list),
		debug(designator),
		debug(initializer),
		debug(unary_operator),
		debug(expression),
		debug(enum_specifier),
		debug(enumerator_list),
		debug(enumerator),
		debug(enumeration_constant),
		debug(type_qualifier),
		debug(function_specifier),
		debug(alignment_specifier),
		debug(declarator),
		debug(direct_declarator),
		debug(type_qualifier_list),
		debug(parameter_type_list),
		debug(parameter_list),
		debug(parameter_declaration),
		debug(abstract_declarator),
		debug(direct_abstract_declarator),
		debug(pointer),
		debug(identifier_list),
		debug(declaration_list),
		debug(compound_statement),
		debug(block_item_list),
		debug(block_item),
		debug(statement),
		debug(labeled_statement),
		debug(expression_statement),
		debug(selection_statement),
		debug(iteration_statement),
		debug(jump_statement),
		debug(declaration),
		debug(init_declarator_list),
		debug(init_declarator),
		debug(static_assert_declaration);

	program
		= translation_unit
		;

	translation_unit
		= external_declaration
		| translation_unit + external_declaration
		;
	
	external_declaration
		= function_definition
		| declaration
		;

	function_definition
		= declaration_specifiers + declarator + declaration_list + compound_statement
		| declaration_specifiers + declarator + compound_statement
		;


	declaration_specifiers
		= storage_class_specifier + declaration_specifiers
		| storage_class_specifier
		| type_specifier + declaration_specifiers
		| type_specifier
		| type_qualifier + declaration_specifiers
		| type_qualifier
		| function_specifier + declaration_specifiers
		| function_specifier
		| alignment_specifier + declaration_specifiers
		| alignment_specifier
		;

	storage_class_specifier
		= rt(t_typedef) 	/* identifiers must be flagged as TYPEDEF_NAME */
		/*[function_t<c_token>([&is_typedef](parse_tree<c_token>& data) mutable {
			is_typedef = true;
			return true;
		})]*/
		| rt(t_extern)
		| rt(t_static)
		| rt(t_thread_local)
		| rt(t_auto)
		| rt(t_register)
		;

	type_specifier
		= rt(t_void)
		| rt(t_char)
		| rt(t_short)
		| rt(t_int)
		| rt(t_long)
		| rt(t_float)
		| rt(t_double)
		| rt(t_signed)
		| rt(t_unsigned)
		| rt(t_bool)
		| rt(t_complex)
		| rt(t_imaginary)	  	/* non-mandated extension */
		| atomic_type_specifier
		| struct_or_union_specifier
		| enum_specifier
		| rt(t_typedef_name) 		/* after it has been defined as such */
		;

	atomic_type_specifier
		= rt(t_atomic) + rt(t_round_left) + type_name + rt(t_round_right)
		;

	type_name
		= specifier_qualifier_list + abstract_declarator
		| specifier_qualifier_list
		;

	struct_or_union_specifier
		= struct_or_union + rt(t_brace_left) + struct_declaration_list + rt(t_brace_right)
		| struct_or_union + rt(t_identifier) + rt(t_brace_left) + struct_declaration_list + rt(t_brace_right)
		| struct_or_union + rt(t_identifier)
		;

	struct_or_union
		= rt(t_struct)
		| rt(t_union)
		;

	struct_declaration_list
		= struct_declaration
		| struct_declaration_list + struct_declaration
		;


	struct_declaration
		= specifier_qualifier_list + rt(t_semicolon)	/* for anonymous struct/union */
		| specifier_qualifier_list + struct_declarator_list + rt(t_semicolon)
		| static_assert_declaration
		;

	specifier_qualifier_list
		= type_specifier + specifier_qualifier_list
		| type_specifier
		| type_qualifier + specifier_qualifier_list
		| type_qualifier
		;

	struct_declarator_list
		= struct_declarator
		| struct_declarator_list + rt(t_comma) + struct_declarator
		;

	struct_declarator
		= rt(t_colon) + constant_expression
		| declarator + rt(t_colon) + constant_expression
		| declarator
		;

	constant_expression
		= conditional_expression	/* with constraints */
		;

	conditional_expression
		= logical_or_expression
		| logical_or_expression + rt(t_qmark) + expression + rt(t_colon) + conditional_expression
		;

	logical_or_expression
		= logical_and_expression
		| logical_or_expression + rt(t_or_op) + logical_and_expression
		;

	logical_and_expression
		= inclusive_or_expression
		| logical_and_expression + rt(t_and_op) + inclusive_or_expression
		;

	inclusive_or_expression
		= exclusive_or_expression
		| inclusive_or_expression + rt(t_vertical) + exclusive_or_expression
		;

	exclusive_or_expression
		= and_expression
		| exclusive_or_expression + rt(t_carrete) + and_expression
		;

	and_expression
		= equality_expression
		| and_expression + rt(t_ampersand) + equality_expression
		;

	equality_expression
		= relational_expression
		| equality_expression + rt(t_eq_op) + relational_expression
		| equality_expression + rt(t_ne_op) + relational_expression
		;

	relational_expression
		= shift_expression
		| relational_expression + rt(t_angle_left) + shift_expression
		| relational_expression + rt(t_angle_right) + shift_expression
		| relational_expression + rt(t_le_op) + shift_expression
		| relational_expression + rt(t_ge_op) + shift_expression
		;

	shift_expression
		= additive_expression
		| shift_expression + rt(t_left_op) + additive_expression
		| shift_expression + rt(t_right_op) + additive_expression
		;

	additive_expression
		= multiplicative_expression
		| additive_expression + rt(t_plus) + multiplicative_expression
		| additive_expression + rt(t_minus) + multiplicative_expression
		;

	multiplicative_expression
		= cast_expression
		| multiplicative_expression + rt(t_star) + cast_expression
		| multiplicative_expression + rt(t_slash) + cast_expression
		| multiplicative_expression + rt(t_percent) + cast_expression
		;

	cast_expression
		= unary_expression
		| rt(t_round_left) + type_name + rt(t_round_right) + cast_expression
		;

	unary_expression
		= postfix_expression
		| rt(t_inc_op) + unary_expression
		| rt(t_dec_op) + unary_expression
		| unary_operator + cast_expression
		| rt(t_sizeof) + unary_expression
		| rt(t_sizeof) + rt(t_round_left) + type_name + rt(t_round_right)
		| rt(t_alignof) + rt(t_round_left) + type_name + rt(t_round_right)
		;

	postfix_expression
		= primary_expression
		| postfix_expression + rt(t_square_left) + expression + rt(t_square_right)
		| postfix_expression + rt(t_round_left) + rt(t_round_right)
		| postfix_expression + rt(t_round_left) + argument_expression_list + rt(t_round_right)
		| postfix_expression + rt(t_dot) + rt(t_identifier)
		| postfix_expression + rt(t_ptr_op) + rt(t_identifier)
		| postfix_expression + rt(t_inc_op)
		| postfix_expression + rt(t_dec_op)
		| rt(t_round_left) + type_name + rt(t_round_right) + rt(t_brace_left) + initializer_list + rt(t_brace_right)
		| rt(t_round_left) + type_name + rt(t_round_right) + rt(t_brace_left) + initializer_list + rt(t_comma) + rt(t_brace_right)
		;

	primary_expression
		= rt(t_identifier)
		| constant
		| string
		| rt(t_round_left) + expression + rt(t_round_right)
		| generic_selection
		;

	constant
		= rt(t_i_constant)		/* includes character_constant */
		| rt(t_f_constant)
		| rt(t_enumeration_constant)	/* after it has been defined as such */
		;

	string
		= rt(t_string_literal)
		| rt(t_func_name)
		;

	generic_selection
		= rt(t_generic) + rt(t_round_left) + assignment_expression + rt(t_comma) + generic_assoc_list + rt(t_round_right)
		;

	assignment_expression
		= conditional_expression
		| unary_expression + assignment_operator + assignment_expression
		;

	assignment_operator
		= rt(t_assign)
		| rt(t_mul_assign)
		| rt(t_div_assign)
		| rt(t_mod_assign)
		| rt(t_add_assign)
		| rt(t_sub_assign)
		| rt(t_left_assign)
		| rt(t_right_assign)
		| rt(t_and_assign)
		| rt(t_xor_assign)
		| rt(t_or_assign)
		;

	generic_assoc_list
		= generic_association
		| generic_assoc_list + rt(t_comma) + generic_association
		;

	generic_association
		= type_name + rt(t_colon) + assignment_expression
		| rt(t_default) + rt(t_colon) + assignment_expression
		;

	argument_expression_list
		= assignment_expression
		| argument_expression_list + rt(t_comma) + assignment_expression
		;

	initializer_list
		= designation + initializer
		| initializer
		| initializer_list + rt(t_comma) + designation + initializer
		| initializer_list + rt(t_comma) + initializer
		;


	designation
		= designator_list + rt(t_assign)
		;

	designator_list
		= designator
		| designator_list + designator
		;

	designator
		= rt(t_square_left) + constant_expression + rt(t_square_right)
		| rt(t_dot) + rt(t_identifier)
		;

	initializer
		= rt(t_brace_left) + initializer_list + rt(t_brace_right)
		| rt(t_brace_left) + initializer_list + rt(t_comma) + rt(t_brace_right)
		| assignment_expression
		;

	unary_operator
		= rt(t_ampersand)
		| rt(t_star)
		| rt(t_plus)
		| rt(t_minus)
		| rt(t_tilda)
		| rt(t_exmark)
		;

	expression
		= assignment_expression
		| expression + rt(t_comma) + assignment_expression
		;

	enum_specifier
		= rt(t_enum) + rt(t_brace_left) + enumerator_list + rt(t_brace_right)
		| rt(t_enum) + rt(t_brace_left) + enumerator_list + rt(t_comma) + rt(t_brace_right)
		| rt(t_enum) + rt(t_identifier) + rt(t_brace_left) + enumerator_list + rt(t_brace_right)
		| rt(t_enum) + rt(t_identifier) + rt(t_brace_left) + enumerator_list + rt(t_comma) + rt(t_brace_right)
		| rt(t_enum) + rt(t_identifier)
		;

	enumerator_list
		= enumerator
		| enumerator_list + rt(t_comma) + enumerator
		;

	enumerator	/* identifiers must be flagged as ENUMERATION_CONSTANT */
		= enumeration_constant + rt(t_assign) + constant_expression
		| enumeration_constant
		;

	enumeration_constant		/* before it has been defined as such */
		= rt(t_identifier) 
		[function_t<c_token>([&enums](parse_tree<c_token>& data) mutable {
			enums.insert(data.last().at(0).data()); 
			return true;
		})]
		;

	type_qualifier
		= rt(t_const)
		| rt(t_restrict)
		| rt(t_volatile)
		| rt(t_atomic)
		;

	function_specifier
		= rt(t_inline)
		| rt(t_noreturn)
		;

	alignment_specifier
		= rt(t_alignas) + rt(t_round_left) + type_name + rt(t_round_right)
		| rt(t_alignas) + rt(t_round_left) + constant_expression + rt(t_round_right)
		;

	declarator
		= pointer + direct_declarator
		| direct_declarator
		;

	direct_declarator
		= rt(t_identifier)
		/*[function_t<c_token>([&is_typedef, &pre_typedefs](parse_tree<c_token>& data) mutable {
			if (is_typedef)
				pre_typedefs.push_back(data.last().at(0).data());
			return true;
		})]*/
		| rt(t_round_left) + declarator + rt(t_round_right)
		| direct_declarator + rt(t_square_left) + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + rt(t_star) + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + rt(t_static) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + rt(t_static) + assignment_expression + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + type_qualifier_list + rt(t_star) + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + type_qualifier_list + rt(t_static) + assignment_expression + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + type_qualifier_list + rt(t_square_right)
		| direct_declarator + rt(t_square_left) + assignment_expression + rt(t_square_right)
		| direct_declarator + rt(t_round_left) + parameter_type_list + rt(t_round_right)
		| direct_declarator + rt(t_round_left) + rt(t_round_right)
		| direct_declarator + rt(t_round_left) + identifier_list + rt(t_round_right)
		;

	type_qualifier_list
		= type_qualifier
		| type_qualifier_list + type_qualifier
		;

	parameter_type_list
		= parameter_list + rt(t_comma) + rt(t_ellipsis)
		| parameter_list
		;

	parameter_list
		= parameter_declaration
		| parameter_list + rt(t_comma) + parameter_declaration
		;

	parameter_declaration
		= declaration_specifiers + declarator
		| declaration_specifiers + abstract_declarator
		| declaration_specifiers
		;

	abstract_declarator
		= pointer + direct_abstract_declarator
		| pointer
		| direct_abstract_declarator
		;

	direct_abstract_declarator
		= rt(t_round_left) + abstract_declarator + rt(t_round_right)
		| rt(t_square_left) + rt(t_square_right)
		| rt(t_square_left) + rt(t_star) + rt(t_square_right)
		| rt(t_square_left) + rt(t_static) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| rt(t_square_left) + rt(t_static) + assignment_expression + rt(t_square_right)
		| rt(t_square_left) + type_qualifier_list + rt(t_static) + assignment_expression + rt(t_square_right)
		| rt(t_square_left) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| rt(t_square_left) + type_qualifier_list + rt(t_square_right)
		| rt(t_square_left) + assignment_expression + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + rt(t_star) + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + rt(t_static) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + rt(t_static) + assignment_expression + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + type_qualifier_list + assignment_expression + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + type_qualifier_list + rt(t_static) + assignment_expression + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + type_qualifier_list + rt(t_square_right)
		| direct_abstract_declarator + rt(t_square_left) + assignment_expression + rt(t_square_right)
		| rt(t_round_left) + rt(t_round_right)
		| rt(t_round_left) + parameter_type_list + rt(t_round_right)
		| direct_abstract_declarator + rt(t_round_left) + rt(t_round_right)
		| direct_abstract_declarator + rt(t_round_left) + parameter_type_list + rt(t_round_right)
		;


	pointer
		= rt(t_star) + type_qualifier_list + pointer
		| rt(t_star) + type_qualifier_list
		| rt(t_star) + pointer
		| rt(t_star)
		;

	identifier_list
		= rt(t_identifier)
		| identifier_list + rt(t_comma) + rt(t_identifier)
		;

	declaration_list
		= declaration
		| declaration_list + declaration
		;

	compound_statement
		= rt(t_brace_left) + rt(t_brace_right)
		| rt(t_brace_left) + block_item_list + rt(t_brace_right)
		;

	block_item_list
		= block_item
		| block_item_list + block_item
		;

	block_item
		= declaration
		| statement
		;

	statement
		= labeled_statement
		| compound_statement
		| expression_statement
		| selection_statement
		| iteration_statement
		| jump_statement
		;

	labeled_statement
		= rt(t_identifier) + rt(t_colon) + statement
		| rt(t_case) + constant_expression + rt(t_colon) + statement
		| rt(t_default) + rt(t_colon) + statement
		;

	expression_statement
		= rt(t_semicolon)
		| expression + rt(t_semicolon)
		;

	selection_statement
		= rt(t_if) + rt(t_round_left) + expression + rt(t_round_right) + statement + rt(t_else) + statement
		| rt(t_if) + rt(t_round_left) + expression + rt(t_round_right) + statement
		| rt(t_switch) + rt(t_round_left) + expression + rt(t_round_right) + statement
		;

	iteration_statement
		= rt(t_while) + rt(t_round_left) + expression + rt(t_round_right) + statement
		| rt(t_do) + statement + rt(t_while) + rt(t_round_left) + expression + rt(t_round_right) + rt(t_semicolon)
		| rt(t_for) + rt(t_round_left) + expression_statement + expression_statement + rt(t_round_right) + statement
		| rt(t_for) + rt(t_round_left) + expression_statement + expression_statement + expression + rt(t_round_right) + statement
		| rt(t_for) + rt(t_round_left) + declaration + expression_statement + rt(t_round_right) + statement
		| rt(t_for) + rt(t_round_left) + declaration + expression_statement + expression + rt(t_round_right) + statement
		;

	jump_statement
		= rt(t_goto) + rt(t_identifier) + rt(t_semicolon)
		| rt(t_continue) + rt(t_semicolon)
		| rt(t_break) + rt(t_semicolon)
		| rt(t_return) + rt(t_semicolon)
		| rt(t_return) + expression + rt(t_semicolon)
		;

	declaration
		= declaration_specifiers + rt(t_semicolon)
		| declaration_specifiers + init_declarator_list + rt(t_semicolon)
		[function_t<c_token>([&typedefs](parse_tree<c_token>& data) mutable {
		parse_tree<c_token>& current = data.last();
		if (current.at(0).begin()->id() == t_typedef) {
			for (size_t i = 0; i < current.at(1).size(); ++i)
				for(auto iter = current.at(1).at(i).begin(); iter != current.at(1).at(i).end(); ++iter) {
					if (iter->id() == t_identifier) {
						typedefs.insert(iter->data());
						break;
					}
			}
		}
		/*
			if (is_typedef) {
				for (auto x : pre_typedefs)
					
				pre_typedefs.clear();
				is_typedef = false;
			}*/
			return true;
		})]
		| static_assert_declaration
		;

	init_declarator_list
		= init_declarator
		| init_declarator_list + rt(t_comma) + init_declarator
		;

	init_declarator
		= declarator + rt(t_assign) + initializer
		| declarator
		;

	static_assert_declaration
		= rt(t_static_assert) + rt(t_brace_left) + constant_expression + rt(t_comma) + rt(t_string_literal) + rt(t_brace_right) + rt(t_semicolon);
		;


	/*
	pr_primary_expression,
	pr_constant,
	pr_string,
	pr_expression,
	pr_generic_selection,
	pr_assignment_expression,
	generic_assoc_list;


	pr_primary_expression
	= rt(t_identifier)
	| pr_constant
	| pr_string
	| rt(t_round_left) + pr_expression + rt(t_round_right)
	| pr_generic_selection
	;

	pr_constant
	= rt(t_i_constant)
	| rt(t_f_constant)
	//TO-DO: enumeration constant
	;

	//TO-DO: enumeration_constant

	pr_string
	= rt(t_string_literal)
	| rt(t_func_name)
	;

	pr_generic_selection
	= rt(t_generic) + rt(t_round_left) + pr_assignment_expression + rt(t_comma) + pr_generic_assoc_list + rt(t_round_right)
	;
	*/
	//pr_generic_assoc_list

	//lex.parse("parse_rule<T> E0(\"E0\"), E(\"E\"), T(\"T\"), n(\"n\"), SE(\"SE\"), SS(\"SS\");E0 = E;");

	/*
	std::string code("static LIST_HEAD(async_global_pending);	/* pending from all registered doms /\n\
static ASYNC_DOMAIN(async_dfl_domain);\n\
static DEFINE_SPINLOCK(async_lock);\n\
\n\
struct async_entry {\n\
	struct list_head	domain_list;\n\
	struct list_head	global_list;\n\
	struct work_struct	work;\n\
	void			*data;\n\
	struct async_domain	*domain;\n\
};\n\
\n\
static DECLARE_WAIT_QUEUE_HEAD(async_done);\n\
\n\
\n\
static int lowest_in_progress(struct async_domain *domain)\n\
{\n\
	struct list_head *pending;\n\
	unsigned long flags;\n\
\n\
	spin_lock_irqsave(&async_lock, flags);\n\
\n\
	if (domain)\n\
		pending = &domain->pending;\n\
	pending = &async_global_pending;\n\
\n\
	if (!list_empty(pending))\n\
		ret = list_first_entry(pending, async_entry,\n\
				       domain_list)->cookie;\n\
\n\
	spin_unlock_irqrestore(&async_lock, flags);\n\
	return ret;\n\
}"
		/*
		"\
int main(int argc, char* argv[]) {\n\
	printf(\"Hello world!\");\n\
	return 0;\n\
}\
"*///);
	
	//lex.parse(code);
	/*translation_unit.print(std::cout);
	external_declaration.print(std::cout);
	function_definition.print(std::cout);
	declaration_specifiers.print(std::cout);
	storage_class_specifier.print(std::cout);
	type_specifier.print(std::cout);
	atomic_type_specifier.print(std::cout);
	type_name.print(std::cout);
	struct_or_union_specifier.print(std::cout);
	struct_or_union.print(std::cout);
	struct_declaration_list.print(std::cout);
	struct_declaration.print(std::cout);
	specifier_qualifier_list.print(std::cout);
	struct_declarator_list.print(std::cout);
	struct_declarator.print(std::cout);
	constant_expression.print(std::cout);
	conditional_expression.print(std::cout);
	logical_or_expression.print(std::cout);
	logical_and_expression.print(std::cout);
	inclusive_or_expression.print(std::cout);
	exclusive_or_expression.print(std::cout);
	and_expression.print(std::cout);
	equality_expression.print(std::cout);
	relational_expression.print(std::cout);
	shift_expression.print(std::cout);
	additive_expression.print(std::cout);
	multiplicative_expression.print(std::cout);
	cast_expression.print(std::cout);
	unary_expression.print(std::cout);
	postfix_expression.print(std::cout);
	primary_expression.print(std::cout);
	constant.print(std::cout);
	string.print(std::cout);
	generic_selection.print(std::cout);
	assignment_expression.print(std::cout);
	assignment_operator.print(std::cout);
	generic_assoc_list.print(std::cout);
	generic_association.print(std::cout);
	argument_expression_list.print(std::cout);
	initializer_list.print(std::cout);
	designation.print(std::cout);
	designator_list.print(std::cout);
	designator.print(std::cout);
	initializer.print(std::cout);
	unary_operator.print(std::cout);
	expression.print(std::cout);
	enum_specifier.print(std::cout);
	enumerator_list.print(std::cout);
	enumerator.print(std::cout);
	enumeration_constant.print(std::cout);
	type_qualifier.print(std::cout);
	function_specifier.print(std::cout);
	alignment_specifier.print(std::cout);
	declarator.print(std::cout);
	direct_declarator.print(std::cout);
	type_qualifier_list.print(std::cout);
	parameter_type_list.print(std::cout);
	parameter_list.print(std::cout);
	parameter_declaration.print(std::cout);
	abstract_declarator.print(std::cout);
	direct_abstract_declarator.print(std::cout);
	pointer.print(std::cout);
	identifier_list.print(std::cout);
	declaration_list.print(std::cout);
	compound_statement.print(std::cout);
	block_item_list.print(std::cout);
	block_item.print(std::cout);
	statement.print(std::cout);
	labeled_statement.print(std::cout);
	expression_statement.print(std::cout);
	selection_statement.print(std::cout);
	iteration_statement.print(std::cout);
	jump_statement.print(std::cout);
	declaration.print(std::cout);
	init_declarator_list.print(std::cout);
	init_declarator.print(std::cout);
	static_assert_declaration;
	*/
	//parse_rule<tokens> start = designator;
	//states.to_dot(std::cout);
	lr_states<c_token> states(program);
	//states.to_dot(std::cout);
	lr_table<c_token> table(states);
	lr<c_token, std::string> parser(table);

	std::ifstream file_in(argv[1]);
	std::ofstream file_out(argv[2]);
	if (!file_in.is_open()) {
		std::cout << "Can't open " << argv[1] << std::endl; //TO-DO: shell load
		return 1;
	}
	if (!file_out.is_open()) {
		std::cout << "Can't open " << argv[2] << std::endl; //TO-DO: shell load
		return 1;
	}
	lex.set_stream(file_in);
	while (!lex.eof()) {
		token t = lex.next(); //TO-DO: template lexer
		parser.next((c_token)t.index(), t.data());
	}
	
	auto& tree = parser.get_tree();

	tree.to_dot(file_out);

	//std::vector<token>& tok = lex.get_tokens();
	//std::cout << "State\tStack\tInput\tEvent" << std::endl;
	//for (token& t : tok)
		
	
	/*lr_table<tokens> table(states);
	*/
	/*
	states.to_dot(std::cout);
	lr_table<tokens> table(states);
	lr<tokens> parser(table);
	//parser.parse(
	//
	*/
	/*tree_vector t;
	parse_rule<char> E0("E0"), E("E"), T("T"), n("n"), SE("SE"), SS("SS");
	E0 = E;
	E =	E + rt('+') + T [std::bind(&tree_vector::sum, &t, std::placeholders::_1)] |
	T;
	T = n |  rt('(') + E + ')';
	n = rt('0') [std::bind(&tree_vector::add_n, &t, std::placeholders::_1)] |
	rt('1') [std::bind(&tree_vector::add_n, &t, std::placeholders::_1)] |
	rt('2') [std::bind(&tree_vector::add_n, &t, std::placeholders::_1)] |
	rt('3') [std::bind(&tree_vector::add_n, &t, std::placeholders::_1)] |
	rt('4') [std::bind(&tree_vector::add_n, &t, std::placeholders::_1)] |
	rt('5') [std::bind(&tree_vector::add_n, &t, std::placeholders::_1)] |
	rt('6') [std::bind(&tree_vector::add_n, &t, std::placeholders::_1)] |
	rt('7') [std::bind(&tree_vector::add_n, &t, std::placeholders::_1)] |
	rt('8') [std::bind(&tree_vector::add_n, &t, std::placeholders::_1)] |
	rt('9') [std::bind(&tree_vector::add_n, &t, std::placeholders::_1)];
	/*E0.print(std::cout);
	E.print(std::cout);
	T.print(std::cout);
	n.print(std::cout);*/
	/*lr_states<char> states(E0);
	states.to_dot(std::cout);
	lr_table<char> table(states);
	lr<char> parser(table);
	std::string line;
	getline(std::cin, line);
	std::cout << "State\tStack\tInput\tEvent" << std::endl;
	std::cout << (parser.parse(line) ? "All done" : "Not finished?") << std::endl;
	t.print();*/
	return 0;
}




















