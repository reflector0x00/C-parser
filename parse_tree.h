#pragma once
#include <stack>
#include <string>
#include <vector>
#include <queue>


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
	//TO-DO: (גûגוסעט to_dot מעעהוכüםמ) - א םאהמ?
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
				size_t pos;
				//TO-DO: procedure?
				for (size_t i = 0; (pos = temp.find('\\', i)) != std::string::npos; i = pos + 2)
					temp.insert(pos, "\\");
				for (size_t i = 0; (pos = temp.find('"', i)) != std::string::npos; i = pos + 2)
					temp.insert(pos, "\\");
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
