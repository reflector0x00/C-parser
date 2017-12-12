#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>

#include "parse_item.h"
#include "parse_state.h"



template <typename T>
class lalr_states {
	class kernel_hasher {
		parse_item_hasher<T> h;
	public:
		size_t operator()(const parse_state<T>* key) const {
			size_t r = 0;
			for (size_t i = 0; i < key->kernel(); ++i)
				r ^= h((*key)[i]);
			return r;
		}
	};

	class kernel_eq {
	public:
		bool operator()(const parse_state<T>* lhs, const parse_state<T>* rhs) const {
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

	std::vector<parse_state<T>*> _states;
	std::unordered_map<parse_rule<T>*, std::set<T>> _first;
	std::unordered_map<parse_state<T>*, size_t, kernel_hasher, kernel_eq> _kernels;

	void closure(parse_state<T>& vector) {
		std::queue<size_t> queue;
		std::unordered_set<size_t> queued;
		for (size_t i = 0; i < vector.size(); ++i) {
			queue.push(i);
			queued.insert(i);
		}

		while (!queue.empty()) {
			size_t i = queue.front();
			queue.pop();
			queued.erase(i);

			if ((!vector[i].chain()) || (vector[i].is_terminal()))
				continue;

			expression_chain<T>* beta = vector[i].chain()->next();
			if ((!beta) || (beta->is_terminal())) {

				parse_rule<T>& sub = vector[i].chain()->nterminal();
				if (beta)  //TO-DO: ustal_kostil
					for (size_t j = 0; j < sub.size(); ++j) {
						parse_item<T> temp(sub, sub[j]);
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
						parse_item<T> temp(sub, sub[j]);
						size_t ind;
						if (vector.find(temp, ind)) {
							if (vector[ind].assign(vector[i].preview()) && (queued.find(ind) == queued.end())) {
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
				parse_rule<T>& sub = vector[i].chain()->nterminal();
				for (size_t j = 0; j < sub.size(); ++j) {
					parse_item<T> temp(sub, sub[j]);
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
			}
		}
	}
	void _goto(parse_state<T>& result, parse_state<T>& current, T terminal) { //TO-DO: one goto
		for (size_t i = 0; i < current.size(); ++i) {
			if (!current[i].chain())
				continue;
			if (current[i].is_terminal() && (current[i].terminal() == terminal))
				result.push_back(current[i].next());
		}
		result.set_kernel();
	}
	void _goto(parse_state<T>& result, parse_state<T>& current, parse_rule<T>& nterminal) {
		for (size_t i = 0; i < current.size(); ++i) {
			if (!current[i].chain())
				continue;
			if ((!current[i].is_terminal()) && (current[i].nterminal() == nterminal))
				result.push_back(current[i].next());
		}
		result.set_kernel();
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
						for (expression_chain<T>* iter = left[i]->next(); iter; iter = iter->next())
							//И если элемент является нетерминалом и еще не помечен как обработанный(-ывающийся), то добавляем в очередь
							if ((!iter->is_terminal()) && (processed.find(&iter->nterminal()) == processed.end()))
								queue.push(&iter->nterminal());
					//И поднимаемся из рекурсии
					stack_symbols.pop_back();
					stack_indexes.pop();
				}

			}

		}
	}
public:
	lalr_states(parse_rule<T>& start) {
		build_first(start);
		parse_state<T>* first = new parse_state<T>;
		parse_item<T> first_item(start, start[0]);
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

		while (!queue.empty()) {
			size_t i = queue.front();
			queue.pop();
			queued.erase(i);

			for (auto& terminal : _states[i]->goto_terminals()) {
				parse_state<T> temp;
				_goto(temp, *_states[i], terminal.first);
				auto check = _kernels.find(&temp);
				if (check != _kernels.end()) {
					_states[i]->set_goto(terminal.first, check->second);

					bool changed = false;
					parse_state<T>& s = *check->first;
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
					}
				}
				else {
					closure(temp);
					_states[i]->set_goto(terminal.first, _states.size()); //TO-DO: set directly?
					_states.push_back(new parse_state<T>(std::move(temp)));
					size_t ind = _states.size() - 1;
					_kernels[_states[ind]] = ind;
					queue.push(ind);
					queued.insert(ind);
				}

			}

			for (auto& nterminal : _states[i]->goto_nterminals()) {


				parse_state<T> temp;
				_goto(temp, *_states[i], *nterminal.first);
				auto check = _kernels.find(&temp);
				if (check != _kernels.end()) {
					_states[i]->set_goto(nterminal.first, check->second);
					bool changed = false;
					parse_state<T>& s = *check->first;
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
					}
				}
				else {
					closure(temp);
					_states[i]->set_goto(nterminal.first, _states.size());
					_states.push_back(new parse_state<T>(std::move(temp)));
					size_t ind = _states.size() - 1;
					_kernels[_states[ind]] = ind;
					queue.push(ind);
					queued.insert(ind);
				}

			}
		}

	}
	void to_dot(std::ostream& o) {

		o << "digraph states {" << std::endl
			<< "\trankdir=LR;" << std::endl;
		for (size_t i = 0; i < _states.size(); ++i) {
			parse_state<T>& current = *_states[i];
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
					if (p != *preview.begin())
						o << "/";

					if (p)
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
	parse_state<T>& operator[](size_t i) {
		return *_states[i];
	}
};
