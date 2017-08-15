#ifndef _sym_table_h_
#define _sym_table_h_

#include <string>
#include <vector>
#include <list>

#include "emulator.h"



using namespace std;

class SymbolTable{

	struct Entry{

		// type of the entry, 'SEG' or 'SYM'
		string type;

		// index of the entry
		int index;

		// name of the symbol/section
		string name;

		// index of the section that contatins the symbol
		// 0 for external, -1 for absolute
		int section;

		// value of the symbol, or the start address of the section
		int value;

		// size of the  section, unused for symbols
		unsigned size;

		// flags - G for global or L for local symbols, sections are local
		string flags;

		
		Entry();
		Entry(string t, int i, string n, int sec, int v);

		string str();

	};

public:

	friend ostream& operator<<(ostream&, const Entry&);


	vector<Entry> entries;

	void add_section_entry(string name, int address);

	void add_symbol_entry(string name, int value, int section);

	void set_section_size(string name, unsigned size);
	void set_section_size(int index, unsigned size);

	void set_symbol_to_global(string name);

	int get_index_of_section(string name);
	int get_index_of_symbol(string name);

	int get_section_of_symbol(string name);

	int get_value_of_symbol(string name);
	int get_address_of_section(string name);

	void add_external_symbol(string name);

	void set_symbol_to_absolute(string name);
	void set_section_to_absolute(string name);

	int get_section_of_section(string name);

	int get_address_of_section(int index);
	

	list<pair<string, int>> get_symbols();

	friend ostream& operator<<(ostream&, const SymbolTable&);

	string str();

	bool has_symbol(string name);

	friend istream& operator>> (istream&, SymbolTable&);

	friend istream& operator>> (istream&, Entry&);

	friend unsigned load(string path_to_obj, Memory& mem );


	static bool overlap(unsigned beginning, unsigned ending, vector<pair<unsigned, unsigned>>& occupance);
	static pair<unsigned, unsigned> get_overlap(unsigned beginning, unsigned ending, vector<pair<unsigned, unsigned>>& occupance);


private:

	Entry& get_entry(string name);
	bool has_entry(string name);


	void add_entry(string type, string name, int value, int section=-2);

	void virtual_load_of_abs_sections(vector<pair<unsigned, unsigned>>&, const unsigned MAX_SIZE);
	void virtual_load_of_rel_setions(vector<pair<unsigned, unsigned>>&, const unsigned MAX_SIZE);


};

#endif