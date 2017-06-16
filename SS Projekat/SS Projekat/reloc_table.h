#ifndef _reloc_table_h_
#define _reloc_table_h_

#include <iostream>
#include <vector>

using namespace std;

class RelocationTable{
	
	struct Entry{

		// type, A for absolute, R for relative
		char type;

		// index in the table of symbols of the symbol that influences the relocation
		unsigned index;

		// offset in the section for the destination
		int offset;

		Entry(char _type, unsigned _index, int _offset);

		string str();
	};

	vector<Entry> entries;
	string section_name;

public:

	RelocationTable();

	RelocationTable(string name);

	friend ostream& operator<<(ostream& out, const Entry&);

	friend ostream& operator<< (ostream& out, const RelocationTable&);

	string str();

	void add_absolute_entry(unsigned index, int offest);

	void add_relative_entry(unsigned index, int offset);

	void set_section_name(string name);

	void empty();


	string get_section_name();

private:

	void add_entry(char type, unsigned index, int offset);

};


#endif