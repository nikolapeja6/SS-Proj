#ifndef _line_h_
#define _line_h_

#include <string>
#include <list>
#include <regex>
#include <cstdint>
#include <unordered_map>

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

	vector<int32_t> get_define_data_values();

	int get_define_data_size();


	pair<string, int> get_symbolic_constant();

	int get_org_value();

	int get_section_number();

	bool has_define_data();
	bool has_section();

	void replace_symbols(list<pair<string, int>> );



	vector<uint8_t> get_encoded_instruction(list<pair<string, int>>, uint32_t pc );
	vector<string> get_labels_in_instruction();


	vector<uint8_t> get_encoded_define_data_values();



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
	static regex ld_st_extension;

	static unordered_map<string, uint8_t>  instruction_codes;
	static unordered_map<string, uint8_t>  address_codes;
	static unordered_map<string, uint8_t>  type_codes;
	static unordered_map<string, uint8_t>  register_codes;


	static int IPR(string s);
	static int SPR(string s);
	static int R(string s);


public:

	static int evaluate_expression(string expression);

	string static replace_symbols(string str, list<pair<string, int>> symbols);

	static int get_number(string str);

};


#endif