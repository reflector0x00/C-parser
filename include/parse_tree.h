#pragma once
#include <stack>
#include <string>
#include <vector>
#include <queue>

template <typename T>
class parse_tree;

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
	std::string _data; //TO-DO: merge _data and _node_name
	std::vector<parse_tree<T>*> _childrens;
	bool compare(const parse_tree<T>& first, const parse_tree<T>& second) const {
		if (first._leaf != second._leaf)
			return false;
		if (first._childrens.size() != second._childrens.size())
			return false;
		if (first._leaf)
			return first._data == second._data;
		return first._node_name == second._node_name;
	}

public:
	parse_tree() : _node_name("root"), _root(nullptr), _leaf(false) {}
	parse_tree(std::istream& o) : _node_name("root"), _root(nullptr), _leaf(false) {
		std::string str;
		o >> str;
		if (str != "digraph")
			throw std::runtime_error("Can't parse digraph from stream: digraph keyword expected"); //TO-DO: unique exceptions
		o >> str;
		o >> str;
		if(str != "{")
			throw std::runtime_error("Can't parse digraph from stream: figure brace expected"); //TO-DO: unique exceptions
		
		std::unordered_map<std::string, size_t> identifiers;
		std::vector<parse_tree<T>*> nodes;
		std::unordered_map<std::string, std::list<std::string>> links;

	
		o >> str;
		while (str != "}") {
			std::string name = str;
			std::string operation;
			o >> operation;
			if (operation == "[") {
				o >> str;
				if(str != "label")
					throw std::runtime_error("Can't parse digraph from stream: \"label\" expected"); //TO-DO: unique exceptions
				o >> str;
				if (str != "=")
					throw std::runtime_error("Can't parse digraph from stream: assign after \"label\" expected"); //TO-DO: unique exceptions
				
				while (o.peek() == ' ')
					o.get();
				
				if(o.get() != '"')
					throw std::runtime_error("Can't parse digraph from stream: quote sign expected"); //TO-DO: unique exceptions
				
				std::string node_name;
				for (char x = o.get(); x != '"'; x = o.get()) {
					if (x == '\\')
						x = o.get();
					node_name.push_back(x);
				}

				o >> str;
				if (str != "color")
					throw std::runtime_error("Can't parse digraph from stream: \"color\" expected"); //TO-DO: unique exceptions
				o >> str;
				if (str != "=")
					throw std::runtime_error("Can't parse digraph from stream: assign after \"color\" expected"); //TO-DO: unique exceptions
				std::string color_name;
				o >> color_name;

				bool leaf;
				if (color_name == "red") 
					leaf = false;
				else if (color_name == "blue") 
					leaf = true;
				else 
					throw std::runtime_error("Can't parse digraph from stream: unknown color - blue or red expected"); //TO-DO: unique exceptions
				
				o >> str;
				if(str != "]")
					throw std::runtime_error("Can't parse digraph from stream: square brace expected"); //TO-DO: unique exceptions
				
				parse_tree* temp;
				if (leaf)
					temp = new parse_tree<T>(T(), node_name, nullptr);
				else
					temp = new parse_tree<T>(nullptr, node_name);

				identifiers[name] = nodes.size();
				nodes.push_back(temp);
			}
			else if (operation == "->") {
				std::string second_name;
				o >> second_name;
				links[name].push_back(second_name);
			}
			else 
				throw std::runtime_error("Can't parse digraph from stream: unknown operator \"" + operation + "\"");

			o >> str;
		}
		
		for (auto& iter : links) {
			parse_tree<T>* first = nodes[identifiers[iter.first]];
			for (auto x : iter.second) {
				parse_tree<T>* second = nodes[identifiers[x]];
				first->_childrens.push_back(second);
				second->_root = first;
			}
		}

		_childrens.push_back(*nodes.begin());
		done();

	}
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
				o << temp << "\" color = blue ] " << std::endl;
			}
			else
				o << next->_node_name << "\" color = red ] " << std::endl;

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
	parse_tree<T>& at(size_t i) const {
		return *(_childrens[i]);
	}
	std::string data() {
		return _data;
	}
	T id() {
		return _id;
	}
	size_t size() const {
		return _childrens.size();
	}
	bool leaf() const {
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

	bool operator ==(const parse_tree<T>& o) const {
		if (size() != o.size())
			return false;
		if (!compare(*this, o))
			return false;


		std::stack<size_t> indexes;
		std::stack<const parse_tree<T>*> pointers_first;
		std::stack<const parse_tree<T>*> pointers_second;

		pointers_first.push(this);
		pointers_second.push(&o);
		indexes.push(0);

		while (!indexes.empty()) {
			const parse_tree<T>* first = pointers_first.top();
			const parse_tree<T>* second = pointers_second.top();
			size_t& ind = indexes.top();

			if (ind >= first->size()) {
				pointers_first.pop();
				pointers_second.pop();
				indexes.pop();
				if (!indexes.empty())
					++indexes.top();
				continue;
			}


			const parse_tree<T>& child_first = first->at(ind);
			const parse_tree<T>& child_second = second->at(ind);
			if (!compare(child_first, child_second))
				return false;

			if (!child_first.leaf()) {
				pointers_first.push(&child_first);
				pointers_second.push(&child_second);
				indexes.push(0);
			}
			else
				++ind;
		}
		return true;

	}
	/* TO-DO: to find out, why it not works
	std::vector<parse_tree<T>*::iterator begin() {
		return _childrens.begin();
	}
	std::vector<parse_tree<T>*>::iterator end() {
		return _childrens.end();
	}
	*/
	leaf_iterator<T> leaf_begin() {
		return leaf_iterator<T>(this);
	}
	leaf_iterator<T> leaf_end() {
		return leaf_iterator<T>();
	}
	parse_tree<T>& operator =(parse_tree<T>&& o) {
		_node_name = o._node_name;
		_root = o._root;
		_leaf = o._leaf;
		_id = o._id;
		_data = o._data;
		std::swap(_childrens, o._childrens);
		return *this;
	}
	~parse_tree() {
		for (auto x : _childrens)
			delete x;
	}
};
