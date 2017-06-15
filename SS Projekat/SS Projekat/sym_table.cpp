#include "sym_table.h"
#include "my_util.h"
#include <sstream>
#include <climits>

using namespace std;

SymbolTable::Entry::Entry(string t, int i, string n, int sec, int v) :type(t), index(i), name(n), section(sec), value(v), flags("L"){}



ostream& operator<<(ostream& out, const SymbolTable::Entry& e){

	out << e.type << " " << e.index << " ";
	out << e.name << " " << e.section << " 0x" << hex << e.value << " ";
	if (e.type == "SEG")
		out << "0x" << hex << e.size << " ";

	out<< dec << e.flags;

 return out;
}


string SymbolTable::Entry::str(){
	stringstream out;

	out << type << " " << index << " ";
	out << name << " " << section << " 0x" << hex << value << " ";
	if (type == "SEG")
		out << "0x" << hex << size << " ";

	out << dec << flags;

	return out.str();
}

string SymbolTable::str(){
	stringstream out;

	out << "#TabelaSimbola" << endl;
	for (SymbolTable::Entry e : entries)
		out << e << endl;

	return out.str();
}


SymbolTable::Entry& SymbolTable::get_entry(string name){
	for (Entry& e : entries)
		if (e.name == name)
			return e;
	string error = "get_entry called in SymbolTable for entry '" + name + "', but it doesnt exist.";
	mlog.error(error);
	throw error;
}


bool SymbolTable::has_entry(string name){
	for (Entry e : entries)
		if (e.name == name)
			return true;

	return false;
}

void SymbolTable::add_entry(string type, string name, int value, int section){
	if (has_entry(name)){
		string error = "Entry with name '" + name + " already exists.";
		mlog.error(error);
		throw error;
	}

	int index = entries.size()+1;
	if (type == "SEG")
		section = index;
	entries.push_back(Entry(type, index, name, section ,value));
}

void SymbolTable::add_section_entry(string name, int address){
	try{
		add_entry("SEG", name, address);
	}
	catch (string s){
		string error = "add_section_entry encountered an error. "+s;
		mlog.error(error);
		throw error;
	}
	
}

void SymbolTable::add_symbol_entry(string name, int value, int section){
	try{
		add_entry("SYM", name, value, section);
	}
	catch (string s){
		string error = "add_symbol_entry encountered an error. " + s;
		mlog.error(error);
		throw error;
	}
}

void SymbolTable::set_section_size(string name, unsigned size){
	if (!has_entry(name)){
		string error = "set_section_size encountered an error. Entry with name '" + name + " does not exist exist.";
		mlog.error(error);
		throw error;
	}

	Entry& e = get_entry(name);

	if (e.type != "SEG"){
		string error = "set_section_size encountered an error. Entry with name '" + name + " is not a section entry.";
		mlog.error(error);
		throw error;
	}

	e.size = size;

	mlog.std("size of section '" + e.name + "' is set to " + to_string(size));
}

void SymbolTable::set_section_size(int index, unsigned size){
	if (index <1 || index >= entries.size()){
		string error = "set_section_size encountered an error. Entry with index '" + to_string(index) + " does not exist exist.";
		mlog.error(error);
		throw error;
	}

	Entry& e = entries[index-1];

	if (e.type != "SEG"){
		string error = "set_section_size encountered an error. Entry with index '" + to_string(index) + " is not a section entry.";
		mlog.error(error);
		throw error;
	}

	e.size = size;

	mlog.std("size of section '" + e.name + "' is set to " + to_string(size));

}

void SymbolTable::set_symbol_to_global(string name){
	if (!has_entry(name)){
		string error = "set_symbol_to_global encountered an error. Entry with name '" + name + " does not exist exist.";
		mlog.error(error);
		throw error;
	}

	Entry e = get_entry(name);

	if (e.type != "SYM"){
		string error = "set_section_size encountered an error. Entry with name '" + name + " is not a symbol entry.";
		mlog.error(error);
		throw error;
	}

	e.flags = "G";
}

int SymbolTable::get_index_of_section(string name){
	if (!has_entry(name)){
		string error = "get_index_of_section encountered an error. Entry with name '" + name + " does not exist exist.";
		mlog.error(error);
		throw error;
	}

	Entry e = get_entry(name);

	if (e.type != "SEG"){
		string error = "get_index_of_section encountered an error. Entry with name '" + name + " is not a section entry.";
		mlog.error(error);
		throw error;
	}

	return e.index;
}
int SymbolTable::get_index_of_symbol(string name){
	if (!has_entry(name)){
		string error = "get_index_of_symbol encountered an error. Entry with name '" + name + " does not exist exist.";
		mlog.error(error);
		throw error;
	}

	Entry e = get_entry(name);

	if (e.type != "SYM"){
		string error = "get_index_of_symbol encountered an error. Entry with name '" + name + " is not a symbol entry.";
		mlog.error(error);
		throw error;
	}

	return e.index;
}

int SymbolTable::get_section_of_symbol(string name){
	if (!has_entry(name)){
		string error = "get_section_of_symbol encountered an error. Entry with name '" + name + " does not exist exist.";
		mlog.error(error);
		throw error;
	}

	Entry e = get_entry(name);

	if (e.type != "SYM"){
		string error = "get_section_of_symbol encountered an error. Entry with name '" + name + " is not a symbol entry.";
		mlog.error(error);
		throw error;
	}

	return e.section;
}

int SymbolTable::get_value_of_symbol(string name){
	if (!has_entry(name)){
		string error = "get_value_of_symbol encountered an error. Entry with name '" + name + " does not exist exist.";
		mlog.error(error);
		throw error;
	}

	Entry e = get_entry(name);

	if (e.type != "SYM"){
		string error = "get_value_of_symbol encountered an error. Entry with name '" + name + " is not a symbol entry.";
		mlog.error(error);
		throw error;
	}

	return e.value;
}
int SymbolTable::get_address_of_section(string name){
	if (!has_entry(name)){
		string error = "get_address_of_section encountered an error. Entry with name '" + name + " does not exist exist.";
		mlog.error(error);
		throw error;
	}

	Entry e = get_entry(name);

	if (e.type != "SEG"){
		string error = "get_address_of_section encountered an error. Entry with name '" + name + " is not a section entry.";
		mlog.error(error);
		throw error;
	}

	return e.value;
}

ostream& operator<<(ostream& out, const SymbolTable& t){
	out << "#TabelaSimbola" << endl;
	for (SymbolTable::Entry e : t.entries)
		out << e << endl;

	return out;
}

list<pair<string, int>> SymbolTable::get_symbols(){

	list<pair<string, int>> l;
	for (Entry e : entries)
		if (e.type == "SYM")
			l.push_back(make_pair(e.name, e.value));

	return l;
}

bool SymbolTable::has_symbol(string name){
	return has_entry(name);
}

void SymbolTable::add_external_symbol(string name){
	add_entry("SYM", name, INT_MAX, 0);
	set_symbol_to_global(name);
}


void SymbolTable::set_symbol_to_absolute(string name){
	try{
		Entry& e = get_entry(name);
		if (e.type != "SYM")
			throw "'" + name + "' is not a symbol";

		e.section = -1;

	}
	catch (string s){
		string error = "Error in set_symbol_to_absolute. " + s;
		mlog.error(error);
		throw error;
	}
}


void SymbolTable::set_section_to_absolute(string name){
	try{
		Entry& e = get_entry(name);
		if (e.type != "SEG")
			throw "'" + name + "' is not a section";

		e.section = -1;

	}
	catch (string s){
		string error = "Error in set_section_to_absolute. " + s;
		mlog.error(error);
		throw error;
	}
}