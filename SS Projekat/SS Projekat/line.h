#ifndef _line_h_
#define _line_h_

#include <string>
#include <list>
#include <regex>
#include <cstdint>

using namespace std;

class Line{

	// original line
	string line;


public:

	Line(string l);

	// returns the original line
	string get_line();


	// checks if line is empty
	bool is_empty();

	// checks if  line has comment
	bool has_comment();

	// checks if line has label
	bool has_label();

	// checks if line has an instruction
	bool has_instruction();

	// check if line has directive
	bool has_directive();



	// returns the comment of the line, "" if there is none
	string get_comment();

	// returns the label of the line, "" if there is none
	string get_label();

	// returs the whole instruction, or "" if there is none
	string get_instruction();

	// returns the whole directive, or "" if there is none
	string get_directive();

	string get_core();

	int get_instruction_size();


	// returns instruction name, or "" if there is none
	string get_instruction_name();

	// returns the name of the directive, or "" if there is none
	string get_directive_name();

	// returns the directive name with different arguments as separate elements in the list
	list<string> get_split_directive();

	// returns the instruction name with different arguments as separate elements in the list
	list<string> get_split_instruction();

	list<string> get_directive_arguments();

	list<string> get_instruction_arguments();

	list<int32_t> get_define_data_values();

	int get_define_data_size();


	pair<string, int> get_symbolic_constant();

	int get_org_value();

	int Line::get_section_number();



private:

	static regex blank;
	static regex comment;
	static regex label;
	static regex core;
	//static regex directive;

	static regex define_data;
	static regex number;

	static regex define_symbolic_constant;

	static regex org;

	static regex section;

	static regex global;

	static regex end;


	static string flow_instruction_names;
	static string mem_instruction_names;
	static string stack_instruction_names;
	static string arlog_instruction_names;

	static string instruction_names;



	static string reg;

	static regex immed;
	static regex regdir;
	static regex regind;
	static regex regindoff;
	static regex pcrel;

	static regex instruction;



	static int IPR(string s);
	static int SPR(string s);
	static int R(string s);


public:

	static int evaluate_expression(string expression);

	static int get_number(string str);

};


#endif