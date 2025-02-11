#include "loader.h"
#include "content_table.h"
#include "reloc_table.h"
#include "sym_table.h"
#include "my_util.h"
#include "emulator.h"

#include <fstream>
#include <unordered_map>

using namespace std;


unsigned load(string path_to_obj, Memory& mem){

	mlog.std("started load");

	// READING OBJ FILE

	ifstream in;
	in.open(path_to_obj);

	if (!in.good()){
		// Error
		mlog.error(" *** assemble error - " + path_to_obj + " is not good. Exit with code 1");
		exit(1);
	}


	SymbolTable symbol_table;

	unordered_map<string, pair<RelocationTable, ContentTable>> sections;

	unsigned start;

	try{

		in >> symbol_table;

		
		if (!symbol_table.has_symbol("START")){
			string error = "No 'START' symbol found.";
			mlog.error(error);
			throw error;
		}


		try{
			while (true){


				RelocationTable reloc_table;
				ContentTable content;

				in >> reloc_table >> content;

				mlog.std(reloc_table.str());
				mlog.std(content.str());

				sections.insert(make_pair(reloc_table.get_section_name(), make_pair(reloc_table, content)));

			}
		}
		catch (string s){
			if (s != "Error reading RelocationTable. First line does not match fpormat '#rel.<section_name>'. Line is '#end'")
				throw;
			else
				mlog.error("^^^OK, handeled");
		}

		in.close();

	}
	catch (string s){
		string error = "Error occured while loading the obj file. " + s;
		mlog.error(error);
		cout << error << endl;
		exit(2);
	}

	mlog.std("finished reading obj file. Read "+to_string(sections.size())+"sections.");




	// CHECING FOR OVERLAPPING OF ABSOLUTE SETIONS
	// AND LOADING THEM WITH RELOCATION AND DATA CORRECTION

	vector<pair<unsigned, unsigned>> occupance;

	// IVT
	//occupance.push_back(make_pair(0, 4*32+2)); // 32 Entryies of 4B, and 2 reg (IN OUT) of 1B

	try{

		symbol_table.virtual_load_of_abs_sections(occupance, Memory::MAX_MEM_SIZE);

		symbol_table.virtual_load_of_rel_setions(occupance, Memory::MAX_MEM_SIZE);

		mlog.std("occupance");
		for (auto p : occupance)
			mlog.std(to_string(p.first) + " - " + to_string(p.second));
		mlog.std("");

		for (auto p : sections){

			mlog.std("relocating section '" + p.first+"'");

			for (RelocationTable::Entry e : p.second.first.entries){
				
				int new_offset_of_label;
				int index_of_label = e.index;
				int index_of_section = symbol_table.entries[index_of_label-1].section;
				new_offset_of_label = symbol_table.entries[index_of_section-1].value;

				int access_offset;
				access_offset = e.offset;



				uint8_t values[] = { 0, 0, 0, 0 };

				for (int i = 0; i <4; i++)
					values[i] = p.second.second.data[access_offset + i];

				int32_t val = (values[3] << 24) | (values[2] << 16) | (values[1] << 8) | values[0];
				
				val += new_offset_of_label;

				if (e.type == 'R'){
					string section_name = p.second.first.section_name;
					int pc_offset = symbol_table.get_entry(section_name).value;
					val -= pc_offset;
				}

				values[0] = val&UINT8_MAX;
				values[1] = (val >> 8)&UINT8_MAX;
				values[2] = (val >> 16)&UINT8_MAX;
				values[3] = (val >> 24)&UINT8_MAX;

				for (int i = 0; i < 4; i++)
					p.second.second.data[access_offset + i] = values[i];

			}


			unsigned base = symbol_table.get_address_of_section(p.first);

			// LOADING
			for (int i = 0; i < p.second.second.data.size(); i++)
				mem[base + i] =  p.second.second.data[i];

			mlog.std("finishd " + p.first);
		
		}


		mlog.error(symbol_table.str());

		start = symbol_table.get_value_of_symbol("START");
		if (symbol_table.get_section_of_symbol("START") != -1){
			int index = symbol_table.get_section_of_symbol("START");
			start += symbol_table.get_address_of_section(index);
		}

	}
	catch (string s){
		string error = "Error occured while relocatng and loading. " + s;
		mlog.error(error);
		cout << error << endl;
		exit(2);
	}



	mlog.std("finished loading");

	return start;

}