#include "content_table.h"
#include "my_util.h"

#include <sstream>
#include <iomanip>
#include <regex>


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

void ContentTable::append(unsigned char new_data){
	data.push_back(new_data);
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
			out << "0x"<<setfill('0') << setw(2) << hex << (unsigned)data[i] << " ";
		
		if (i < data.size())
			out << endl;
	}

	return out.str();

}

istream& operator>>(istream& in, ContentTable& table){

	regex first_line("^(\\.\\w+(\\.\\d+)?)\\s*$");

	string line;
	smatch match;
	getline(in, line);

	if (!regex_match(line, match, first_line))
		throw "First line is not in format '.<section_name>'. Line is '" + line + "'";

	string name = match[1];
	table.set_section_name(name);

	try{

		while (in.peek() != EOF && in.peek() != '#'){

			getline(in, line);
			istringstream l(line);

			unsigned data;
			while (l>>hex>>data)
				table.append((unsigned char)(data & UCHAR_MAX));
	
		}

	
		mlog.std("next_char after content table = '" +to_string(in.peek())+"'");

		in >> dec;

		return in;

	}
	catch(string s){
		string error = "Error reading content of section. " + s;
		mlog.error(error);
		throw error;
	}
}