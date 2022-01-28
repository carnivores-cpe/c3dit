#include "stdafx.h"

#include <fstream>


namespace std {
	/*
	Tokenize a string into a vector of strings
	TODO: Test this
	*/
	bool tokenize(vector<string>& _tokens, string _string, const string& _delims)
	{
		if (_string.empty())
			return false;

		string tok = "";

		while (!_string.empty()) {
			size_t i = _string.find_first_of(_delims);

			if (i == string::npos) { break; }

			tok = _string.substr(i);
			_string.erase(0, i);

			if (tok.empty()) { continue; }

			_tokens.push_back(tok);
		}
		
		return _tokens.size() > 0;
	}
} // namespace std

