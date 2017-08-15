#include "reloc_table.h"
#include "my_util.h"

#include <sstream>
#include <regex>

RelocationTable::RelocationTable(){}

RelocationTable::Entry::Entry(){}
RelocationTable::Entry::Entry(char _type, unsigned _index, int _offset) :type(_type), index(_index), offset(_offset){}

void RelocationTable::set_section_name(string name){
	section_name = name;
}


string RelocationTable::Entry::str(){
	stringstream out;

	out << offset << " " << type << " " << index;

	return out.str();
}

void RelocationTable::empty(){
	section_name = "";
	entries.clear();
}


ostream& operator<<(ostream& out, const RelocationTable::Entry& e){
	return out << e.offset << " " << e.type << " " << e.index;
}

RelocationTable::RelocationTable(string name) :section_name(name){}

ostream& operator<< (ostream& out, const RelocationTable& r){
	out << "#rel" << r.section_name << endl;
	for (RelocationTable::Entry e : r.entries)
		out << e << endl;

	return out;
}

string RelocationTable::str(){
	stringstream out;

	out << "#rel" << section_name << endl;
	for (RelocationTable::Entry e : entries)
		out << e << endl;

	return out.str();
}

void RelocationTable::add_absolute_entry(unsigned index, int offset){
	add_entry('A', index, offset);
}

void RelocationTable::add_relative_entry(unsigned index, int offset){
	add_entry('R', index, offset);
}


void RelocationTable::add_entry(char type, unsigned index, int offset){
	entries.push_back(Entry(type, index, offset));
}

string RelocationTable::get_section_name(){
	return section_name;
}


istream& operator>>(istream& in, RelocationTable& table){

	string line;
	getline(in, line);

	regex first_line("^\\#rel(\\..+)$");
	smatch match;

	try{
		
		if (!regex_match(line, match, first_line))
			throw "First line does not match fpormat '#rel.<section_name>'. Line is '" + line + "'";
		
		string name = match[1];
		table.set_section_name(name);

		while (in.peek() != '.' && in.peek() != EOF){
			RelocationTable::Entry e;
			in >> e;
			getline(in, line);
			table.entries.push_back(e);
		}

		mlog.std("next char after relocation table is '" +to_string(in.peek()));

		return in;

	}
	catch (string s){
		string error = "Error reading RelocationTable. " + s;
		mlog.error(error);
		throw error;
	}
	
}
istream& operator>>(istream& in, RelocationTable::Entry& entry){


	// type, A for absolute, R for relative
	char type;

	unsigned index;
	int offset;


	in >> offset;
	in >> type;
	if (type != 'A' && type != 'R')
		throw "Error reaing entry. Type was not 'A' or 'R', but '" + type + '\'';

	in >> index;

	entry.index = index;
	entry.offset = offset;
	entry.type = type;

	return in;

}