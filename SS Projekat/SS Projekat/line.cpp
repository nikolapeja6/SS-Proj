#include "line.h"
#include "my_util.h"
#include <stack>

using namespace std;


regex Line::blank("^\\s*$");
regex Line::comment("^[^;]*;\\s*(.*)$");
regex Line::label("^\\s*(\\w+)\\s*:[^:]*$");
regex Line::core("^\\s*(?:\\w+\\s*:\\s*)?([^;:]*)\\s*(?:;\\s*(.*))?$");

//regex Line::directive("^\\s*(\\.\\w+|ORG)(\\s+(\\w+))?(\\s*,\\s*(\\w+))*\\s*$");
//string old_old_define_data = "^\\s*(DB|DW|DD)\\s+([\\?\\s\\w\\+\\-\\*/\\(\\)]+)(?:\\s*,\\s*([\\?\\s\\w\\+\\-\\*/\\(\\)]+))*\\s*$";
//string old_define_data = "^\\s*(DB|DW|DD)\\s+([^,]+)(?:\\s*,\\s*([^,]+))*\\s*$";

regex Line::define_data("^\\s*(DB|DW|DD)\\s+(.*)\\s*$");
regex Line::number("^\\s*(-)?((?:0x|0X|0b|0B)?)([a-fA-F\\d]+|'\\w'|\\?)\\s*$");

regex Line::define_symbolic_constant("^\\s*(\\w+)\\s+DEF\\s+(.+)\\s*$");

regex Line::org("^\\s*ORG\\s+(.*[^\\s])\\s*$");

regex Line::section("^\\s*((\\.(?:text|data|rodata|bss))(\\.\\d+)?)\\s*$");

regex Line::global("^\\s*\\.global\\s+([\\w\\s,]+)\\s*$");

regex Line::end("^\\s*\\.end(\\s+.*|$)");



string Line::flow_instruction_names = "INT|JMP|CALL|RET|JZ|JNZ|JGZ|JGEZ|JLZ|JLEZ";
string Line::mem_instruction_names = "LOAD(?:UB|SB|UW|SW)?|STORE(?:B|W)?";
string Line::stack_instruction_names = "PUSH|POP";
string Line::arlog_instruction_names = "ADD|SUB|MUL|DIV|MOD|AND|OR|XOR|NOT|ASL|ASR";

string Line::instruction_names =	Line::flow_instruction_names + "|" +
									Line::mem_instruction_names + "|" +
									Line::stack_instruction_names + "|" +
									Line::arlog_instruction_names;


regex Line::instruction("^\\s*("+Line::instruction_names+")\\s+(.*)\\s*$");

string Line::reg = "R(?:0?1|0?2|0?3|0?4|0?5|0?6|0?7|0?8|0?9|10|11|12|13|14|15)|PC|SP";
regex Line::immed("^#(.*)$");
regex Line::regdir("^("+Line::reg+")$");
regex Line::regind("^[\\s*"+Line::reg+"\\s*]$");
regex Line::regindoff("^[\\s*" + Line::reg + "\\s*\\+\\s*(.*)\\s*]$");
regex Line::pcrel("^\\$\\s*(\\w+)\\s*$");

Line::Line(string l) :line(l){}

// returns the original line
string Line::get_line(){
	return line;
}

// checks if line is empty
bool Line::is_empty(){
	return regex_match(line, blank);
}

// checks if  line has comment
bool Line::has_comment(){
	return regex_match(line, comment);
}

// checks if line has label
bool Line::has_label(){
	return regex_match(line, label);
}

// checks if line has an instruction
bool Line::has_instruction(){

	string core = get_core();

	return regex_match(core, Line::instruction);
}

// check if line has directive
bool Line::has_directive(){
	try{
		get_directive_name();
		return true;
	}
	catch (string s){
		return false;
	}
}



// returns the comment of the line, "" if there is none
string Line::get_comment(){
	smatch match;
	regex_match(line, match, comment);
	return match[1];
}

// returns the label of the line, "" if there is none
string Line::get_label(){
	smatch match;
	regex_match(line, match, label);
	return match[1];
}

// returs the whole instruction, or "" if there is none
string Line::get_instruction(){
	string core = get_core();

	if (regex_match(core, Line::instruction))
		return core;

	string error = "get_instrution called for a line that is not an instruction. Line is '" + get_line() + "'";
	mlog.error(error);
	throw error;
}



string Line::get_core(){
	smatch match;
	regex_match(line, match, Line::core);
	string core = match[1];

	return core;
}



// returns the whole directive
string Line::get_directive(){

	smatch match;
	string core = get_core();

	if (regex_match(core, match, Line::define_data))
		return core;

	if (regex_match(core, match, Line::define_symbolic_constant))
		return core;

	if (regex_match(core, match, Line::org))
		return core;

	if (regex_match(core, match, Line::section))
		return core;

	if (regex_match(core, match, Line::global))
		return core;

	if (regex_match(core, match, Line::end))
		return core;

	string error = "Line does not contain a directive, but get_directive was called. Line is '"+get_line()+"'";
	mlog.error(error);
	throw error;
}


// returns instruction name
string Line::get_instruction_name(){

	smatch match;
	string core = get_core();

	if (regex_match(core, match, Line::instruction))
		return match[1];

	string error = "Line does not contain an instruction, but get_directive was called. Line is '" + get_line() + "'";
	mlog.error(error);
	throw error;

}


// returns the name of the directive
string Line::get_directive_name(){
	smatch match;
	string core = get_core();

	if (regex_match(core, match, Line::define_data))
		return match[1];

	if (regex_match(core, match, Line::define_symbolic_constant))
		return "DEF";

	if (regex_match(core, match, Line::org))
		return "ORG";

	if (regex_match(core, match, Line::section))
		return match[2];

	if (regex_match(core, match, Line::global))
		return ".global";

	if (regex_match(core, match, Line::end))
		return ".end";

	string error = "Line does not contain a directive, but get_directive_name was called";
	mlog.error(error);
	throw error;
}


// returns the directive name with different arguments as separate elements in the list
list<string> Line::get_directive_arguments(){
	smatch match;
	string core = get_core();

	list<string> l;

	if (regex_match(core, match, Line::define_data)){
		regex reg_arg("(?:\\s*([^,]+)\\s*)");

		string arguments = match[2];

		std::regex_iterator<std::string::const_iterator> rit(arguments.begin(), arguments.end(), reg_arg);
		std::regex_iterator<std::string::const_iterator> rend;

		transform(rit, rend, back_inserter(l),
			[](const regex_iterator<std::string::const_iterator>::value_type& it){ return it[1]; });

		return l;

	}

	if (regex_match(core, match, Line::define_symbolic_constant)){
		string label = match[1];
		string expression = match[2];

		l.push_back(label);
		l.push_back(expression);

		return l;
	}

	if (regex_match(core, match, Line::org)){
		string argument = match[1];

		l.push_back(argument);

		return l;
	}

	if (regex_match(core, match, Line::section)){
		
		string argument = match[3];

		l.push_back(argument);

		return l;
	}

	if (regex_match(core, match, Line::global)){
		regex reg_arg("(?:\\s*([^,]+)\\s*)");

		string arguments = match[1];

		std::regex_iterator<std::string::const_iterator> rit(arguments.begin(), arguments.end(), reg_arg);
		std::regex_iterator<std::string::const_iterator> rend;

		transform(rit, rend, back_inserter(l),
			[](const regex_iterator<std::string::const_iterator>::value_type& it){ 
			string arg = it[1];
			//mlog.std("arg = '" + arg+"'");
			regex no_blanks("^\\s*(\\w+)\\s*$");
			smatch m;
			regex_match(arg, m, no_blanks);
			//mlog.std("no blanks = '" + m[1].str() + "'\n");
			return m[1].str();
		});

		return l;

	}

	if (regex_match(core, match, Line::end)){
		mlog.error("get_directive_arguments is called for END directive");

		return l;
	}

	string error = "Line does not contain a directive, but get_directive_arguments was called. Line is '" + get_line() + "'";
	mlog.error(error);
	throw error;
}

list<string> Line::get_split_directive(){
	try{
		list<string> l = get_directive_arguments();
		l.push_front(get_directive_name());

		return l;
	}
	catch (string s){
		string error = "get_spit_directive encountered and error. " + s;
		mlog.error(error);
		throw error;
	}

	
}


list<string> Line::get_instruction_arguments(){
	
	smatch match;
	string core = get_core();

	if (!regex_match(core, match, instruction)){
		string error = "Line does not contain an instruction, but get_instruction_arguments was called. Line is '" + get_line() + "'";
		mlog.error(error);
		throw error;
	}

	list<string> l;

	regex reg_arg("(?:\\s*([^,]+)\\s*)");

	string arguments = match[2];

	std::regex_iterator<std::string::const_iterator> rit(arguments.begin(), arguments.end(), reg_arg);
	std::regex_iterator<std::string::const_iterator> rend;

	transform(rit, rend, back_inserter(l),
		[](const regex_iterator<std::string::const_iterator>::value_type& it){
		string arg = it[1];
		regex no_blanks("^\\s*(.+)\\s*$");
		smatch m;
		regex_match(arg, m, no_blanks);
		return m[1].str();
	});
		

	return l;

}


// returns the instruction name with different arguments as separate elements in the list
list<string> Line::get_split_instruction(){
	list<string> l = get_instruction_arguments();
	if (l.size() > 0)
		l.push_front(get_instruction_name());

	return l;
}





list<int32_t> Line::get_define_data_values(){
	list<int32_t> l;

	string name = get_directive_name();
	
	if (name != "DB" && name != "DW" && name != "DD"){
		string error = "Error - get_define_data_values called for line that is not a define data directive. Line is '" + get_line() + "'";
		mlog.error(error);
		throw error;
	}

	try{

		for (string s : get_directive_arguments()){
			regex dup("^\\s*(.+)\\s+(?:DUP|dup)\\s+(.*)\\s*$");
			smatch m;
			if (regex_match(s, m, dup)){
				string s1 = m[1];
				string s2 = m[2];

				int val1 = evaluate_expression(s1);
				int val2 = evaluate_expression(s2);

				if (val1 <= 0)
					mlog.error("dup was called with number of repetitions less than 1. The number of repetitions is " + to_string(val1) + ". Whole expression is " + s);

				while (val1-- > 0)
					l.push_back(val2);
			}
			else{
				int val = evaluate_expression(s);
				l.push_back(val);
			}
		}
	}
	catch (string exception){
		string error = "Invalid define data. Line is '" + line + "'. Error is '" + exception + "'";
		mlog.error(error);
		throw error;
	}

	return l;
}

int Line::get_define_data_size(){

	string name = get_directive_name();

	if (name != "DB" && name != "DW" && name != "DD"){
		string error = "Error - get_define_data_size called for line that is not a define data directive. Line is '" + get_line() + "'";
		mlog.error(error);
		throw error;
	}

	int num = get_define_data_values().size();

	if (name == "DB")
		return num;
	if (name == "DW")
		return 2 * num;
	return 4 * num;
}


pair<string, int> Line::get_symbolic_constant(){
	try{
		if (!(get_directive_name() == "DEF"))
			throw "Line does not contain definition of symbolic constant. Line is '" + get_line() + "'";

		list<string> l = get_directive_arguments();

		if (l.size() != 2){
			string elements = "";
			for (string s : l)
				elements += s + "||";
			throw "Number of arguments in defining symbolic constant is not 2. Line is '" + get_line() + "'. Arguments are ||" + elements;
		}

		string name = l.front();
		string expression = l.back();

		int value = evaluate_expression(expression);

		return make_pair(name, value);

	}
	catch (string s){
		string error = "get_symbolic_constant encountered and error. " + s;
		mlog.error(error);
		throw error;
	}
}



int Line::get_org_value(){
	try{
		list<string> l = get_directive_arguments();

		if (l.size() != 1){
			string elements = "";
			for (string s : l)
				elements += s + "||";
			throw "Number of arguments in ORG is not 1. Line is '" + get_line() + "'. Arguments are ||" + elements;
		}
		string expression = l.front();
		mlog.std("*" + expression + "*");

		int value = evaluate_expression(expression);

		return value;
	}
	catch (string s){
		string error = "Error occured in get_org_value. Line is '" + get_line() + "'. " + s;
		mlog.error(error);
		throw error;
	}
}


int Line::get_section_number(){
	try{

		list<string> l = get_directive_arguments();

		if (l.size() != 1){
			string elements = "";
			for (string s : l)
				elements += s + "||";
			throw "Number of arguments in SECTION is not 1. Line is '" + get_line() + "'. Arguments are ||" + elements;
		}
		string expression = l.front();
		
		regex reg_num("^\\.(\\d+)\\s*$");
		smatch m;

		regex_match(expression, m, reg_num);

		string number = m[1];

		int value = evaluate_expression(number);

		return value;
	
	}
	catch (string s){
		string error = "Error occured in get_section_number(). Line is '" + get_line() + "'. Error - " + s;
		mlog.error(error);
		throw error;
	}
}



int Line::get_instruction_size(){

	if (!has_instruction()){
		string error = "get_size_of_instruction called for a line that is not an instruction. Line is '" + get_line() + "'";
		mlog.error(error);
		throw error;
	}

	for (string argument : get_instruction_arguments()){
		if (regex_match(argument, Line::regdir))
			continue;
		if (regex_match(argument, Line::regind))
			continue;

		mlog.std("get_size_of_instruction - argument '" + argument + "' is not REGDIR or REGIND, so size is 8B");
		return 8;
	}

	mlog.std("get_size_of_instruction - every argument is REGDIR or REGIND, so size is 4B. Instruction is '"+get_instruction()+"'");
	return 4;
}






int Line::evaluate_expression(string expression){

	if (regex_match(expression, Line::blank))
		throw "empty expression";			

	regex reg_item("\\s*([a-zA-Z0-9]+|\\+|-|\\*|\\(|\\)|/|'\\w'|\\?)\\s*");

	std::regex_iterator<std::string::const_iterator> rit(expression.begin(), expression.end(), reg_item);
	std::regex_iterator<std::string::const_iterator> rend;

	list<string> infix;

	transform(rit, rend, back_inserter(infix),
		[](const regex_iterator<std::string::const_iterator>::value_type& it){ return it[1]; });

	stack<string> s;
	
	list<string> postfix;


	// INFIX to POSTFIX
	int rank = 0;
	for (string next : infix){

		if (next == "+" || next == "*" || next == "-" || next == "/" || next == "(" || next == ")"){
			// operator
			while (!s.empty() && IPR(next) <= SPR(s.top())){
				string x = s.top();
				s.pop();
				postfix.push_back(x);
				rank += R(x);

				if (rank < 1){
					mlog.error("expression is invaid - " + expression);
					throw "expression is invaid - " + expression;
				}
			}

			if (next != ")")
				s.push(next);
			else
				s.pop();
		}
		else{
			// operand
			postfix.push_back(next);
			rank += 1;
		}

	}

	while (!s.empty()){
		string x = s.top();
		s.pop();
		postfix.push_back(x);
		rank += R(x);
	}

	if (rank != 1){
		mlog.error("expression is invaid - " + expression);
		throw "expression is invaid - " + expression;
	}


	stack<long> ss;


	// POSTFIX to VALUE
	for (string x : postfix){
		if (x == "+" || x == "-" || x == "*" || x == "/"){
			// operator
			long op2 = ss.top();
			ss.pop();
			long op1 = ss.top();
			ss.pop();

			long rez;
			if (x == "+")
				rez = op1 + op2;
			if (x == "-")
				rez = op1 - op2;
			if (x == "*")
				rez = op1 * op2;
			if (x == "/")
				rez = op1 / op2;
			ss.push(rez);
		}
		else{
			// opearnd
			ss.push(get_number(x));
		}
	}

	int ret = ss.top();
	ss.pop();

	if (!ss.empty()){
		mlog.error("Error: invalid expression, stack not empty - " + expression);
		throw "Error: invalid expression, stack not empty - " + expression;
	}

	return ret;

}

// return the number that is contained in the string, or throws a string error message
int Line::get_number(string str){
	smatch match;

	if (!regex_match(str, match, Line::number)){
		mlog.error("NaN - " + str);
		throw ("NaN - " + str);
	}

	string base = match[2];
	string value = match[1].str()+match[3].str();


	int ret;

	try{
		if (base == ""){
			if (value == "?")
				ret = 0;
			else if (value[0] == '\'')
				ret = value[1];
			else {
				regex num("^\\d+$");

				if (regex_match(value, num))
					ret = stoi(value, nullptr, 10);
				else
					throw "unknown @@@" + value+"@@@";
			}
				
		}
		else if (base == "0x" || base == "0X")
			ret = stoi(value, nullptr, 16);
		else if (base == "0b" || base == "0B"){
			regex not_bin("^-?[01]+$");
			if (!regex_match(value, not_bin))
				throw invalid_argument("");

			ret = 0;
			if (value[0] == '0' || value[0] == '1')
				ret |= (value[0] == '0' ? 0 : 1);
			for (int i = 1; i < value.length(); i++){
				ret <<= 1;
				ret |= (value[i] == '0' ? 0 : 1);
			}

			if (value[0] == '-')
				ret = -ret;
			

		}
		else {
			mlog.error("unknown format - " + str);
				throw "unknown format - " + str;
		}

	}
	catch (invalid_argument i){ 
		mlog.error("Ivalid argument format - " + str);
		throw "Ivalid argument format - " + str; 
	}

	mlog.std("num " + str + " is " + to_string(ret) + "; value is "+value);

	return ret;


}

int Line::IPR(string s){
	if (s == "+" || s == "-")
		return 2;
	if (s == "*" || s == "/")
		return 3;
	if (s == "(")
		return 6;
	if (s == ")")
		return 1;

	mlog.error("unknown operator in IPR : " + s);
	throw "unknown operator in IPR : " + s;
}

int Line::SPR(string s){
	if (s == "+" || s == "-")
		return 2;
	if (s == "*" || s == "/")
		return 3;
	if (s == "(")
		return 0;

	mlog.error("unknown operator in SPR : " + s);
	throw "unknown operator in SPR : " + s;
}

int Line::R(string s){
	if (s == "+" || s == "-" || s == "*" || s == "/")
		return -1;

	mlog.error("unknown operator in R : " + s);
	throw "unknown operator in R : " + s;
}





