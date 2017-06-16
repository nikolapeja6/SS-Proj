#include "content_table.h"

#include <sstream>
#include <iomanip>


ContentTable::ContentTable(){}

void ContentTable::set_section_name(string name){
	section_name = name;
}


ContentTable::ContentTable(string name) : section_name(name){}

void ContentTable::append(vector<unsigned char> new_data){
	data.insert(data.end(), new_data.begin(), new_data.end());
}

void ContentTable::append(list<unsigned char> new_data){
	data.insert(data.end(), new_data.begin(), new_data.end());
}

ostream& operator<<(ostream& out, const ContentTable& t){
	return out << t.str();
}

void ContentTable::empty(){
	section_name = "";
	data.clear();
}

string ContentTable::str()const{
	stringstream out;

	out << section_name << endl;

	for (int i = 0; i < data.size(); ){
		for (int j = 0; i < data.size() && j < 16; i++, j++)
			out << setfill('0') << setw(2) << hex << data[i] << " ";
		
		if (i < data.size())
			out << endl;
	}

	return out.str();

}