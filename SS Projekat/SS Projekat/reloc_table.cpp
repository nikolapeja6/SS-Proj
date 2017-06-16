#include "reloc_table.h"
#include <sstream>

RelocationTable::RelocationTable(){}

RelocationTable::Entry::Entry(char _type, unsigned _index, int _offset) :type(_type), index(_index), offset(_offset){}

void RelocationTable::set_section_name(string name){
	section_name = name;
}


string RelocationTable::Entry::str(){
	stringstream out;

	out << type << " " << index << " " << offset;

	return out.str();
}

void RelocationTable::empty(){
	section_name = "";
	entries.clear();
}


ostream& operator<<(ostream& out, const RelocationTable::Entry& e){
	return out << e.type << " " << e.index << " " << e.offset;
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
