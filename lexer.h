#pragma once
#include <regex>
#include <functional>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <istream>
#include <tuple>
#include <stdexcept>


typedef std::function<size_t(std::string&)> lexem_function_t; //TO-DO: change name

class lexem_regular {
	std::regex _regex;
	lexem_function_t _function;
public:
	lexem_regular(const std::regex& regex, lexem_function_t func) : _regex(regex), _function(func) {}
	std::regex& regex() {
		return _regex;
	}
	lexem_function_t& function() {
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
	std::unordered_set<char> _flags; //TO-DO: change name
	std::vector<std::pair<std::string, std::string>> _commentary;
	std::vector<std::string> _dividers;
	std::unordered_map<std::string, lexem_function_t> _keywords;
	std::vector<lexem_regular> _regulars;
	std::istream* _stream;
	bool _escape;
	char _flag;
	std::string _line;
	std::string _buffer;

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
					std::streamoff pos = _stream->tellg();
					size_t j;
					for (j = 1; j < first.length(); ++j) {
						char x = _stream->get();
						if (!(*_stream))
							break;
						if (x != first[j])
							break;
					}

					if (j != first.length()) {
						_stream->seekg(pos);
						continue;
					}

					f = true;

					std::string& second = _commentary[i].second;
					for (size_t j = 0; j < second.length(); ++j) {
						char x = _stream->get();
						if (!(*_stream))
							throw std::runtime_error("Comment not closed");
						if (x != second[j])
							j = -1;
					}

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

					std::streamoff pos = _stream->tellg();
					size_t j;
					for (j = 1; j < _dividers[i].length(); ++j) {
						char x = _stream->get();
						if (!(*_stream))
							break;
						if (x != _dividers[i][j])
							break;
					}

					if (j != _dividers[i].length()) {
						_stream->seekg(pos);
						continue;
					}

					f = true;

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
	}


public:
	//TO-DO: change order of operands
	lexer(std::string empty_dividers, std::string flags, std::vector<std::pair<std::string, std::string>> commentary, bool escape, std::vector<std::string>&& dividers) : _commentary(commentary), _dividers(dividers), _escape(escape), _flag(0) {
		for (auto x : flags)
			_flags.insert(x);
		for (auto x : empty_dividers)
			_empty_dividers.insert(x);
	}
	void set_stream(std::istream& stream) {
		_stream = &stream;
	}
	void add_keyword(std::string keyword, lexem_function_t function) {
		_keywords[keyword] = function;
	}
	void add_regular(const std::regex& regular, lexem_function_t function) {
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
	bool eof() {
		return !_stream->operator bool();
	}
};