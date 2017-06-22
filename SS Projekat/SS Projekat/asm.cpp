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
	int first_pc = 0;

	string error = "";
	int i = 0;

	bool end = false;

	list<pair<string, int>> symbols;

	list<string> global;

	// FIRST PASS

	while (getline(fin, l)){
		Line line(l);
		i++;

		try{
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
						symbol_table.set_section_size(section_index, pc - first_pc);
					next_ORG = true;

					line.replace_symbols(symbols);

					first_pc = pc = line.get_org_value(); // CONST
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
						symbol_table.set_section_size(section_index, pc - first_pc);

					string name = line.get_directive();
					if (!next_ORG)
						first_pc = pc = 0;
					symbol_table.add_section_entry(name, pc);


					ORG = next_ORG;
					next_ORG = false;

					if (ORG)
						symbol_table.set_section_to_absolute(name);

					section_index = symbol_table.get_index_of_section(name);
					section_type = line.get_directive();

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
				end = true;
				if (section_index != -1)
					symbol_table.set_section_size(section_index, pc - first_pc);
				break;
			}


			mlog.std("line is '" + line.get_line() + "; pc = " + to_string(pc) + "; new pc = " + to_string(pc + move));
			pc += move;

		}
		catch (string s){
			error += to_string(i)+":\t"+line.get_line()+"\n\t"+s+"\n";
		}

	}

	if (!end)
		symbol_table.set_section_size(section_index, pc-first_pc);


	fin.close();

	mlog.std("Table after first pass - " + symbol_table.str());

	if (error != ""){
		error = "Errors occured on the following lines while assembling (first pass):\n\n" + error;
		mlog.error(error);
		cout << error << endl;
		exit(2);
	}

	for (string s : global){
		if (symbol_table.has_symbol(s))
			symbol_table.set_symbol_to_global(s);
		else
			symbol_table.add_external_symbol(s);
	}


	mlog.std("Table after global - " + symbol_table.str());


	
	
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
	i = 0;

	RelocationTable reloc;
	ContentTable con;

	stringstream output;
	output << symbol_table;

	// SECOND PASS

	while (getline(fin, l)){
		Line line(l);
		i++;

		bool found_something = false;

		try{


			int move = 0;

			if (line.is_empty())
				continue;



			if (line.has_directive()){
				found_something = true;
				string directive_name = line.get_directive_name();
				if (line.has_define_data()){

					// DEFINE DATA

					vector<pair<string, int>> relocation_data;
					relocation_data = line.get_labels_in_define_data(symbols, pc);

					line.replace_symbols(symbols);

					move = line.get_define_data_size(); // CONST

					vector<unsigned char> data = line.get_encoded_define_data_values();
					con.append(data);

					for (auto p : relocation_data){

						if (symbol_table.get_section_of_symbol(p.first) == -1)
							continue;

						int index = symbol_table.get_index_of_symbol(p.first);
						reloc.add_absolute_entry(index, p.second);
					}
				}
				else if (line.has_section()){

					// SECTION

					string name = line.get_directive();
					if (!first_section)
						output << reloc << con << endl;
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
				found_something = true;
				move = line.get_instruction_size();

				bool pcrel = false;

				vector<unsigned char> data = line.get_encoded_instruction(symbols, pc, pcrel);
				con.append(data);

				vector<string> relocation_symbols;
				relocation_symbols = line.get_labels_in_instruction(symbols);



				for (string s : relocation_symbols){
					if (!symbol_table.has_symbol(s))
						continue;

					int index = symbol_table.get_index_of_symbol(s);

					if (symbol_table.get_section_of_symbol(s) == -1)
						continue;

					if (pcrel){

						string name = reloc.get_section_name();
						int current_section_index = symbol_table.get_section_of_section(name); 
						int label_section_index = symbol_table.get_section_of_symbol(s);

						mlog.std("------------current_section index = " + to_string(current_section_index) + "; label_section index = " + to_string(label_section_index));

						if (label_section_index != current_section_index)
							reloc.add_relative_entry(index, pc + 4);
					}
					else
						reloc.add_absolute_entry(index, pc + 4);
				}

					
			}

		
			if (line.has_directive() && line.get_directive_name() == ".end"){
				if (section_index != -1)
					output << reloc << con;
				break;
			}


			if (!found_something){
					string skiped = "Nothing found or undefined instruction - line skipped. Line is '" + line.get_line() + "'";
					mlog.error(error);
			}

			mlog.std("line is '" + line.get_line() + "; pc = " + to_string(pc) + "; new pc = " + to_string(pc + move));
			pc += move;

		}
		catch (string s){
			error += to_string(i) + ":\t" + line.get_line() + "\n\t" + s + "\n";
		}

	}

	if (!end)
		output << reloc << con;

	output << endl << "#end";


	fin.close();


	if (error != ""){
		error = "Errors occured on the following lines while assembling (second pass):\n\n" + error;
		mlog.error(error);
		cout << error << endl;
		exit(3);
	}
	

	mlog.std("\n\n\n***\n\n" + output.str());

	ofstream out;
	out.open("obj_" + path_to_source);
	out << output.str();
	out.close();
	


	mlog.std("assemble finished");

	return "obj_"+path_to_source;
}