#include "my_util.h"
#include "line.h"
#include "sym_table.h"
#include "reloc_table.h"
#include "content_table.h"
#include <sstream>



// assemble - assembles the source code file into an object code file
// string path_to_source - path to the source file
// returns - path to the object file that was created
string assemble(const string& path_to_source){

	mlog.std("assemble started with " + path_to_source);

	
	// opening file with the source
	ifstream fin;
	fin.open(path_to_source);

	// checking to see if it exists
	if (!fin.good()){
		// Error
		mlog.error(" *** assemble error - " + path_to_source + " is not good. Exit with code 1");
		exit(1);
	}


	string l;

	SymbolTable symbol_table;
	int pc = 0;
	bool ORG = false;
	bool next_ORG = false;
	int section_index = -1;
	string section_type = "";

	list<pair<string, int>> symbols;

	list<string> global;

	// FIRST PASS

	while (getline(fin, l)){
		Line line(l);

		int move = 0;


		if (line.is_empty())
			continue;

		if (!line.has_section()){
			if (line.has_label()){
				string name = line.get_label();
				symbol_table.add_symbol_entry(name, pc, section_index);
				symbols.push_back(make_pair(name, pc));

				if (ORG)
					symbol_table.set_symbol_to_absolute(name);
			}
		}

		if (line.has_directive()){
			string directive_name = line.get_directive_name();

			if (directive_name == "ORG"){
				if (section_index != -1)
					symbol_table.set_section_size(section_index, pc);
				next_ORG = true;

				line.replace_symbols(symbols);

				pc = line.get_org_value(); // CONST
			}
			else if (line.has_define_data()){

				line.replace_symbols(symbols);

				move = line.get_define_data_size(); // CONST
			}
			else if (directive_name == "DEF"){

				line.replace_symbols(symbols);

				pair<string, int> sym_const = line.get_symbolic_constant(); // CONST
				//symbolic_constants.push_back(sym_const);
				symbols.push_back(sym_const);
			}
			else if (line.has_section()){

				if (section_index != -1 && !next_ORG)
					symbol_table.set_section_size(section_index, pc);

				string name = line.get_directive();
				if (!next_ORG)
					pc = 0;
				symbol_table.add_section_entry(name, pc);
				

				ORG = next_ORG;
				next_ORG = false;

				if (ORG)
					symbol_table.set_section_to_absolute(name);

				section_index = symbol_table.get_index_of_section(name);
				section_type = line.get_directive_name();
				
			}
			else if (line.get_directive_name() == ".global"){
				
				global.splice(global.end(), line.get_directive_arguments());
			}
		}

		
		if (line.has_instruction()){
			move = line.get_instruction_size();
		}

		if (line.has_section()){
			if (line.has_label()){
				string name = line.get_label();
				symbol_table.add_symbol_entry(name, pc, section_index);
				symbols.push_back(make_pair(name, pc));

				if (ORG)
					symbol_table.set_symbol_to_absolute(name);
			}
		}

		if (line.has_directive() && line.get_directive_name() == ".end"){
			if (section_index != -1)
				symbol_table.set_section_size(section_index, pc);
			break;
		}


		mlog.std("line is '" + line.get_line() + "; pc = " + to_string(pc) + "; new pc = " + to_string(pc + move));
		pc += move;

	}

	mlog.std("Table after first pass - " + symbol_table.str());

	fin.close();

	for (string s : global){
		if (symbol_table.has_symbol(s))
			symbol_table.set_symbol_to_global(s);
		else
			symbol_table.add_external_symbol(s);
	}


	mlog.std("Table after global - " + symbol_table.str());


	/*
	
	fin.open(path_to_source);

	// checking to see if it exists
	if (!fin.good()){
		// Error
		mlog.error(" *** assemble error - " + path_to_source + " is not good. Exit with code 1");
		exit(1);
	}



	pc = 0;
	ORG = false;
	next_ORG = false;
	bool first_section = true;

	RelocationTable reloc;
	ContentTable con;

	stringstream output;
	output << symbol_table << endl;

	// SECOND PASS

	while (getline(fin, l)){
		Line line(l);

		int move = 0;

		if (line.is_empty())
			continue;

		

		if (line.has_directive()){
			string directive_name = line.get_directive_name();

			if (directive_name == "ORG"){
				// ORG

				next_ORG = true;

				line.replace_symbols(symbols);
				pc = line.get_org_value(); // CONST
			}
			else if (line.has_define_data()){

				// DEFINE DATA

				line.replace_symbols(symbols);

				move = line.get_define_data_size(); // CONST

				vector<uint8_t> data = line.get_encoded_define_data_values();

				con.append(data);
			}
			else if (line.has_section()){

				// SECTION

				string name = line.get_directive();
				if (!first_section)
					output << reloc << endl << con << endl;
				else
					first_section = false;

				if (!next_ORG)
					pc = 0;
				ORG = next_ORG;
				next_ORG = false;

				con.empty();
				reloc.empty();

				con.set_section_name(name);
				reloc.set_section_name(name);




			}
		
		}


		if (line.has_instruction()){
			move = line.get_instruction_size();
		}

		if (line.has_directive() && line.get_directive_name() == ".end"){
			if (section_index != -1)
				symbol_table.set_section_size(section_index, pc);
			break;
		}


		mlog.std("line is '" + line.get_line() + "; pc = " + to_string(pc) + "; new pc = " + to_string(pc + move));
		pc += move;

		

	}
	

	*/


	mlog.std("assemble finished");

	return "";
}