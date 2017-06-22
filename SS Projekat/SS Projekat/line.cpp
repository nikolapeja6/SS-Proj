#include "line.h"
#include "my_util.h"
#include <stack>

using namespace std;

#define OPCODE_OFF 24
#define ADDRMODE_OFF 21
#define REG0_OFF 16
#define REG1_OFF 11
#define REG2_OFF 6
#define TYPE_OFF 3


regex Line::blank("^\\s*$");
regex Line::comment("^[^;]*;\\s*(.*)$");
regex Line::label("^\\s*(\\w+)\\s*:[^:]*$");
regex Line::core("^\\s*(?:\\w+\\s*:\\s*)?([^;:]*)\\s*(?:;\\s*(.*))?\\s*$");

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


regex Line::instruction("^\\s*("+Line::instruction_names+")(?:\\s+(.*))?\\s*$");

regex Line::ld_st_extension("^\\s*(?:(?:LOAD|STORE)(UB|SB|UW|SW|B|W)?)\\s+.*$");

string Line::reg = "R(?:0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15)|PC|SP";
regex Line::immed("^#(.*)$");
regex Line::regdir("^\\s*("+Line::reg+")\\s*$");
regex Line::regind("^\s*\\[\\s*("+Line::reg+")\\s*\\]\\s*$");
regex Line::regindoff("^\\s*\\[\\s*(" + Line::reg + ")\\s*\\+\\s*(.*)\\s*\\]\\s*$");
regex Line::pcrel("^\\s*\\$\\s*(\\w+)\\s*$");


unordered_map<string, unsigned char>  Line::instruction_codes{

	{ "INT", 0x00 },
	{ "RET", 0x01 },
	{ "JMP", 0x02 },
	{ "CALL", 0x03 },
	{ "JZ", 0x04 },
	{ "JNZ", 0x05 },
	{ "JGZ", 0x06 },
	{ "JGEZ", 0x07 },
	{ "JLZ", 0x08 },
	{ "JLEZ", 0x09 },

	{ "LOAD", 0x10 },
	{ "STORE", 0x11 },

	{ "PUSH", 0x20 },
	{ "POP", 0x21 },

	{ "ADD", 0x30 },
	{ "SUB", 0x31 },
	{ "MUL", 0x32 },
	{ "DIV", 0x33 },
	{ "MOD", 0x34 },
	{ "AND", 0x35 },
	{ "OR", 0x36 },
	{ "XOR", 0x37 },
	{ "NOT", 0x38 },
	{ "ASL", 0x39 },
	{ "ASR", 0x3A }

};

unordered_map<string, unsigned char>  Line::address_codes{
	{ "immed", 0x4 },
	{ "regdir", 0x0 },
	{ "memdir", 0x6 },
	{ "regind", 0x2 },
	{ "regindoff", 0x7 }
};

unordered_map<string, unsigned char>  Line::register_codes{
	{ "R0", 0x00 },
	{ "R1", 0x01 },
	{ "R2", 0x02 },
	{ "R3", 0x03 },
	{ "R4", 0x04 },
	{ "R5", 0x05 },
	{ "R6", 0x06 },
	{ "R7", 0x07 },
	{ "R8", 0x08 },
	{ "R9", 0x09 },
	{ "R10", 0x0A },
	{ "R11", 0x0B },
	{ "R12", 0x0C },
	{ "R13", 0x0D },
	{ "R14", 0x0E },
	{ "R15", 0x0F },

	{ "SP", 0x10 },

	{ "PC", 0x11 },

};


unordered_map<string, unsigned char> Line::type_codes{
	{ "UB", 0x03},
	{ "SB", 0x07},
	{ "UW", 0x01},
	{ "SW", 0x05},
	{ "", 0x00},
	{ "B", 0x03},
	{ "W", 0x01}
};


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
		mlog.error("^^^OK, not an errror");
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

	if (regex_match(core, match, Line::section)){
		return match[1];
	}

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

	if (regex_match(core, match, Line::instruction)){

		regex ld_st("^(LOAD|STORE).*$");
		string name = match[1];
		if (regex_match(name, match, ld_st))
			return match[1];
		else
			return name;
	}

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





vector<int32_t> Line::get_define_data_values(){
	vector<int32_t> l;

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



bool Line::has_define_data(){
	if (has_directive()){
		string name = get_directive_name();
		if (name == "DB" || name == "DW" || name == "DD")
			return true;
	}

	return false;
}
bool Line::has_section(){
	if (has_directive()){
		string name = get_directive_name();
		if (name == ".text" || name == ".bss" || name == ".rodata" || name ==".data")
			return true;
	}

	return false;
}


void Line::replace_symbols(list<pair<string, int>> symbols){
	
	if (is_empty())
		return;
	if (!(has_directive() || has_instruction()))
		return;

	string core = get_core();

	mlog.std("---replace_symbols called. Original core is '"+core+"'");

	string left = "^(.*\\W)?(?:";
	string right = ")(\\W.*)?$";

	for (auto p : symbols){
		regex replacer(left + p.first + right);
		smatch match;

		while (regex_match(core, match, replacer)){

		mlog.std("replcaing '" + p.first + "'. src = '" + core + "' => dst = '" + match[1].str() + to_string(p.second) + match[2].str() + "'");
		core = match[1].str() + to_string(p.second) + match[2].str();
		}
	}

	string new_line = "";
	if (has_label())
		new_line = get_label() + ": ";
	new_line += core;

	if (has_comment())
		new_line += " ; " + get_comment();

	mlog.std("line '"+line+"'");
	mlog.std("new line '" + new_line+"'");

	line = new_line;


	mlog.std("+++replace_symbol finished. New core is '"+core+"'");

}

string Line::replace_symbols(string str, list<pair<string, int>> symbols){

	string left = "^(.*\\W)?(?:";
	string right = ")(\\W.*)?$";

	for (auto p : symbols){
		regex replacer(left + p.first + right);
		smatch match;

		while (regex_match(str, match, replacer)){

			mlog.std("replcaing '" + p.first + "'. src = '" + str + "' => dst = '" + match[1].str() + to_string(p.second) + match[2].str() + "'");
			str = match[1].str() + to_string(p.second) + match[2].str();
		}
	}

	return str;
}



vector<unsigned char> Line::get_encoded_define_data_values(){
	try{
		vector<int32_t> v = get_define_data_values();
		string type = get_directive_name();

		vector<unsigned char> ret;
		ret.reserve(get_define_data_size());

		int n = 0;

		mlog.std("l.size = " + to_string(v.size()));

		for (auto i : v){
			if (type == "DD")
				n = 4;
			else if (type == "DW")
				n = 2;
			else if (type == "DB")
				n = 1;
			else{
				string error = "define_data is not DB, DW or DD, but '" + type + "'";
				mlog.error(error);
				throw error;
			}

			unsigned char x;
			uint32_t val = i;
			uint32_t mask = UINT8_MAX;
			for (int j = 0; j < n; j++){
				x = uint8_t(val&mask);
				mlog.std(to_string(j) + "B from " + to_string(i) + " is " + to_string(x));
				ret.push_back(x);
				val >>= 8;
			}
		}

		return ret;

	}
	catch (string s){
		string error = "Error in __define_data_values. " + s;
		mlog.error(error);
		throw error;
	}
}

#include <iostream>
#include <iomanip>


vector<unsigned char> Line::get_encoded_instruction(list<pair<string, int>> symbols, uint32_t pc, bool& pcrel){
	string core = get_core();

	uint32_t first = 0;
	uint32_t second = 0;
	bool use_second = false;

	// INSTRUCTION NAME and OPCODE
	string instruction_name = get_instruction_name();
	unsigned char instruction_code = Line::instruction_codes[instruction_name];
	mlog.std("instruction code = "+to_string(instruction_code));
	first |=  instruction_code<< 24;


	// ARGUMENTS and ADDRESS CODE
	list<string> arg = get_instruction_arguments();
	smatch match;
	string arg1, arg2, arg3;
	unsigned char code;
	
	string error;

	switch (instruction_code){


	case 0x00: // INT

		mlog.std("INT");

		if (arg.size() != 1){
			error = "Instruction format error. INT can have only 1 argument. Found " + to_string(arg.size()) + ". Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		if (!regex_match(arg.front(), match, Line::regdir)){
			error = "Instruction format error. INT can have only regdir, but isn't. Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		arg1 = match[1];
		code = Line::register_codes[arg1];
		first |= code << REG0_OFF;

		code = Line::address_codes["regdir"];
		first |= code << ADDRMODE_OFF;

		break;

	case 0x01: // RET
		mlog.std("RET");

		if (arg.size() != 0){
			error = "Instruction format error. RET can have only 0 arguments. Found " + to_string(arg.size()) + ". Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		break;


	case 0x02: // JMP
	case 0x03: // CALL

		mlog.std("JMP, CALL");

		if (arg.size() != 1){
			error = "Instruction format error. JMP, CALL can have only 1 argument. Found " + to_string(arg.size()) + ". Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		// REGIND
		if (regex_match(arg.front(), match, Line::regind)){

			arg1 = match[1];
			code = Line::register_codes[arg1];
			first |= code << REG0_OFF;

			code = Line::address_codes["regind"];
			first |= code << ADDRMODE_OFF;

			break;
		}

		// REGINDOFF
		if (regex_match(arg.front(), match, Line::regindoff)){

			arg1 = match[1];
			arg2 = match[2];

			code = Line::register_codes[arg1];
			first |= code << REG0_OFF;

			arg2 = replace_symbols(arg2, symbols);
			uint32_t val = evaluate_expression(arg2);
			second = val;
			use_second = true;

			code = Line::address_codes["regindoff"];
			first |= code << ADDRMODE_OFF;

			break;
		}

		// PCREL
		if (regex_match(arg.front(), match, Line::pcrel)){


			arg1 = match[1];
			arg1 = replace_symbols(arg1, symbols);

			second = evaluate_expression(arg1);
			second -= pc;
			use_second = true;

			code = Line::address_codes["regindoff"];
			first |= code << ADDRMODE_OFF;

			code = Line::register_codes["PC"];
			first |= code << REG0_OFF;

			pcrel = true;

			break;

		}

		// MEMDIR
		if (!regex_match(arg.front(), Line::immed) && !regex_match(arg.front(), Line::regdir)){

			arg1 = arg.front();
			arg1 = replace_symbols(arg1, symbols);

			mlog.std("---" + arg1);

			second = evaluate_expression(arg1);
			use_second = true;

			code = Line::address_codes["memdir"];
			first |= code << ADDRMODE_OFF;

			break;

		}



		error = "Instruction format error. JMP, CALL can have only regdind, regindoff and memdir, but has somethin else. Core is '" + core + "'";
		mlog.error(error);
		throw error;

		break;




	case 0x04: // JZ
	case 0x05: // JNZ
	case 0x06: // JGZ
	case 0x07: // JGEZ
	case 0x08: // JLZ
	case 0x09: // JLEZ

		mlog.std("JZ, JNZ, JGZ, JGEZ, JLZ, JLEZ ");

		if (arg.size() != 2){
			error = "Instruction format error. must have 2 argument. Found " + to_string(arg.size()) + ". Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		// REGDIR 1.
		if (!regex_match(arg.front(), match, Line::regdir)){
			error = "Instruction format error. JZ, JNZ, JGZ, JGEZ, JLZ, JLEZ must have a REGISTER as 1. argument. Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		arg1 = match[1];

		first |= Line::register_codes[arg1] << REG0_OFF;

		// NOT IMMED and NOT REGDIR 2.
		if (regex_match(arg.back(), Line::immed) || regex_match(arg.back(), Line::regdir)){
			error = "Instruction format error. JZ, JNZ, JGZ, JGEZ, JLZ, JLEZ must have MEMDIR, REGIND or REGINDOFF as 2. argument. Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		// REGIND
		if (regex_match(arg.back(), match, Line::regind)){

			arg2 = match[1];
			code = Line::register_codes[arg2];
			first |= code << REG1_OFF;

			code = Line::address_codes["regind"];
			first |= code << ADDRMODE_OFF;

			break;

		}


		// REGINDOFF
		if (regex_match(arg.back(), match, Line::regindoff)){

			arg1 = match[1];
			arg2 = match[2];

			code = Line::register_codes[arg1];
			first |= code << REG1_OFF;

			arg2 = replace_symbols(arg2, symbols);
			uint32_t val = evaluate_expression(arg2);
			second = val;
			use_second = true;

			code = Line::address_codes["regindoff"];
			first |= code << ADDRMODE_OFF;

			break;
		}

		// PCREL
		if (regex_match(arg.back(), match, Line::pcrel)){


			arg1 = match[1];
			arg1 = replace_symbols(arg1, symbols);

			second = evaluate_expression(arg1);
			second -= pc;
			use_second = true;

			code = Line::address_codes["regindoff"];
			first |= code << ADDRMODE_OFF;

			code = Line::register_codes["PC"];
			first |= code << REG1_OFF;

			pcrel = true;

			break;

		}

		// MEMDIR
		if (!regex_match(arg.back(), Line::immed) && !regex_match(arg.back(), Line::regdir)){

			arg1 = arg.back();
			arg1 = replace_symbols(arg1, symbols);

			mlog.std(arg1);

			second = evaluate_expression(arg1);
			use_second = true;

			code = Line::address_codes["memdir"];
			first |= code << ADDRMODE_OFF;

			break;

		}


		error = "Instruction format error. JZ, JNZ, JGZ, JGEZ, JLZ, JLEZ can have only regdind, regindoff and memdir, but has somethin else. Core is '" + core + "'";
		mlog.error(error);
		throw error;

		break;



	case 0x10: // LOAD
	case 0x11: // STORE

		mlog.std("LOAD, STORE");


		if (arg.size() != 2){
			error = "Instruction format error. STORE, LOAD must have 2 argument. Found " + to_string(arg.size()) + ". Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		// REGDIR 1.
		if (!regex_match(arg.front(), match, Line::regdir)){
			error = "Instruction format error. STORE, LOAD must have a REGISTER as 1. argument. Core is '" + core + "'. '" + arg.front() + "'";
			mlog.error(error);
			throw error;
		}

		arg1 = match[1];
		mlog.std("arg1 = '" + arg1+"'");

		first |= Line::register_codes[arg1] << REG0_OFF;



		if (!regex_match(core, match, Line::ld_st_extension)){
			error = "Instruction format error - STORE, LOAD wrong extension. Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		// TYPE
		arg2 = match[1];
		mlog.std("type fot ld_st is '" + arg2 + "'");
		first |= Line::type_codes[arg2] << TYPE_OFF;


	

		// REGIND 2.
		if (regex_match(arg.back(), match, Line::regind)){

			arg2 = match[1];
			code = Line::register_codes[arg2];
			first |= code << REG1_OFF;

			code = Line::address_codes["regind"];
			first |= code << ADDRMODE_OFF;

			break;

		}


		// REGINDOFF 2.
		if (regex_match(arg.back(), match, Line::regindoff)){

			arg1 = match[1];
			arg2 = match[2];

			code = Line::register_codes[arg1];
			first |= code << REG1_OFF;

			arg2 = replace_symbols(arg2, symbols);
			uint32_t val = evaluate_expression(arg2);
			second = val;
			use_second = true;

			code = Line::address_codes["regindoff"];
			first |= code << ADDRMODE_OFF;

			break;
		}

		// PCREL 2.
		if (regex_match(arg.back(), match, Line::pcrel)){


			arg1 = match[1];
			arg1 = replace_symbols(arg1, symbols);

			second = evaluate_expression(arg1);
			second -= pc;
			use_second = true;

			code = Line::address_codes["regindoff"];
			first |= code << ADDRMODE_OFF;

			code = Line::register_codes["PC"];
			first |= code << REG1_OFF;

			pcrel = true;

			break;

		}

		/*
		// REGDIR 2.
		if (regex_match(arg.back(), match, Line::regdir)){
			arg1 = match[1];
			first |= Line::register_codes[arg1] << REG1_OFF;

			break;
		}
		*/

		// REGDIR 2.
		if (regex_match(arg.back(), match, Line::regdir)){

			arg2 = match[1];
			code = Line::register_codes[arg2];
			first |= code << REG1_OFF;

			code = Line::address_codes["regdir"];
			first |= code << ADDRMODE_OFF;

			break;

		}

		// IMMED 2.
		if (regex_match(arg.back(), match, Line::immed)){
			if (instruction_code == 0x11){
				error = "Instruction format error. IMMED not allowed in STORE. Core is '" + core + "'";
				mlog.error(error);
				throw error;
			}

			first |= Line::address_codes["immed"] << ADDRMODE_OFF;

			arg2 = match[1];

			arg2 = replace_symbols(arg2, symbols);

			second = evaluate_expression(arg2);
			use_second = true;

			break;
		}

		// MEMDIR

		arg1 = arg.back();
		arg1 = replace_symbols(arg1, symbols);

		second = evaluate_expression(arg1);
		use_second = true;

		code = Line::address_codes["memdir"];
		first |= code << ADDRMODE_OFF;

		break;



	case 0x20: // PUSH
	case 0x21: // POP

		mlog.std("PUSH, POP");


		if (arg.size() != 1){
			error = "Instruction format error. PUSH, POP must have 1 argument. Found " + to_string(arg.size()) + ". Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		// REGDIR
		if (regex_match(arg.front(), match, Line::regdir)){
			arg1 = match[1];
			first |= Line::register_codes[arg1] << REG0_OFF;
			break;
		}

		error = "Instruction format error. PUSH, POP must have REGDIR. Core is '" + core + "'";
		mlog.error(error);
		throw error;

	case 0x30: // ADD
	case 0x31: // SUB
	case 0x32: // MUL
	case 0x33: // DIV
	case 0x34: // MOD
	case 0x35: // AND
	case 0x36: // OR
	case 0x37: // XOR
	case 0x38: // NOT
	case 0x39: // ASL
	case 0x3A: // ASR

		mlog.std("ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, NOT, ASL, ASR");


		if ((arg.size() != 3 && instruction_code != 0x38) ||(instruction_code == 0x38 && arg.size() !=2)){
			error = "Instruction format error. ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, NOT, ASL, ASR must have 3 arguments. Found " + to_string(arg.size()) + ". Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		arg1 = arg.front();
		arg.pop_front();
		arg2 = arg.front();
		if (instruction_code != 0x38)
			arg3 = arg.back();

		// NOT REGDIR 1.
		if (!regex_match(arg1, match, Line::regdir)){
			error = "Instruction format error. ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, NOT, ASL, ASR must have REGDIR arguments. Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		arg1 = match[1];
		code = Line::register_codes[arg1];
		first |= code << REG0_OFF;

		// NOT REGDIR 2.
		if (!regex_match(arg2, match, Line::regdir)){
			error = "Instruction format error. ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, NOT, ASL, ASR must have REGDIR arguments. Core is '" + core + "'";
			mlog.error(error);
			throw error;
		}

		arg2 = match[1];
		code = Line::register_codes[arg2];
		first |= code << REG1_OFF;

		// NOT REGDIR 3.
		if (instruction_code != 0x38){
			if (!regex_match(arg3, match, Line::regdir)){
				error = "Instruction format error. ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, NOT, ASL, ASR must have REGDIR arguments. Core is '" + core + "'";
				mlog.error(error);
				throw error;
			}

			arg3 = match[1];
			code = Line::register_codes[arg3];
			first |= code << REG2_OFF;
		}
	
		break;

	default:

		error = "UNKNOWN INSTRUCTION - core is '"+core+"', instruction name is '"+get_instruction_name()+"'";
		mlog.error(error);
		throw error;
	}

	mlog.std("core = " + core);
	mlog.std("first = " + to_string(first));
	mlog.std("second = " + to_string(second));

	vector<unsigned char> v;

	uint32_t mask = UCHAR_MAX;
	for (int i = 0; i < 4; i++){
		unsigned char x = first&mask;
		v.push_back(x);
		first >>= 8;
	}

	if (use_second)
	for (int i = 0; i < 4; i++){
		unsigned char x = mask&second;
		v.push_back(x);
		second >>= 8;
	}


	return v;

}



vector<string> Line::get_labels_in_instruction(list<pair<string, int>> symbols){
	vector<string> ret;

	if (!has_instruction()){
		string error = "get_labels_in_instruction called for a line that does not have an instruction.";
		mlog.error(error);
		throw error;
	}

	string left = "^(?:.*\\W)?";
	string right = "(?:\\W.*)?$";

	for (auto p : symbols){
		regex has(left + p.first + right);
		for (string arg : get_instruction_arguments())
			if (regex_match(arg, has))
				ret.push_back(p.first);
	}

	return ret;
}

vector<pair<string, int>> Line::get_labels_in_define_data(list<pair<string, int>> symbols, int pc){

	if (!has_define_data()){
		string error = "get_labels_in_define_data called for a line that does not have a define data directive.";
		mlog.error(error);
		throw error;
	}


	try{
		vector<pair<string, int>> ret;

		string name = get_directive_name();
		int off = name == "DB" ? 1 : name == "DW" ? 2 : 4;


		string left = "^(?:.*\\W)?";
		string right = "(?:\\W.*)?$";

		for (string arg : get_directive_arguments()){

			regex DUP("^\\s*(.+)\\s+DUP\\s+(.+)\\s*$");
			smatch match;

			string a;
			int rept;
			if (regex_match(arg, match, DUP)){
				a = match[2];
				string num_of_repetitions = match[1];
				rept = evaluate_expression(num_of_repetitions);
			}
			else{
				a = arg;
				rept = 1;
			}

			for (auto p : symbols){
				regex has(left + p.first + right);

				if (!regex_match(a, has))
					continue;

				for (int i = 0; i < rept; i++)
					ret.push_back(make_pair(p.first, pc + i*off));
			}

			pc += rept*off;
		}

		return ret;

	}
	catch (string s){
		string error = "Error in get_labels_in_define_data. Core is '" + get_core() + "'. " + s;
		mlog.error(error);
		throw error;
	}



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





