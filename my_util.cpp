#include "my_util.h"

bool is_white(const char);

// cleans the line from all extra white characters (spaces, tabs) and comments
string clean_line(const string& str){
	string ret = "";

	for (int i = 0; i < str.length();){

		// test for white
		if (is_white(str[i])){
			
			// start of white => add a single space as separator (if needed - not first) and go to end of white
			if (ret != "")
				ret += " ";

			while (i < str.length() && is_white(str[i]))
				i++;
			continue;
		}

		// test for start of comment
		if (str[i] == ';')
			// end of instruction
			break;

		ret += str[i++];
	}

	return ret;
}

// tests if the given character is a white character (space, tab)
inline bool is_white(const char c){
	if (c == ' ' || c == '\t')
		return true;
	else
		return false;
}