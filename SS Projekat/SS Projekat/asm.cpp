#include "my_util.h"

#include <regex>
#include <vector>


class TableOfSimbols{
	struct Entry{
		string type;
		unsigned num;
		string name;
		int section_num;
		unsigned start_address;
		unsigned size;
		int value;
		string flags;

		friend ostream& operator<<(ostream& out, const Entry& e){
			out << e.type << " " << e.num << " " << e.name << " " << e.section_num << " " << hex;
			if (e.type == "SEG")
				out << e.start_address << " " << e.size<< " ";
			else
				out << e.value << " ";

			out << dec << e.flags;

			return out;
		}
	};

	vector<Entry> entries;
	int index = 0;
	int last_section = -1;

public:

	void addSection(const string& name, const unsigned& start_address){
		Entry e;
		e.type = "SEG";  e.name = name; e.num = index; e.section_num = index;
		e.start_address = start_address; e.flags = "L";
		entries.push_back(e);
		index++;
		last_section++;
	}
	void updateSection(const string& name, const unsigned& size){
		for (int i = 0; i < entries.size(); i++){
			if (entries[i].name == name){
				entries[i].size = size;
				break;
			}
		}
	}
	
	void addSymbol(const string& name, const int& section_num){
		Entry e;
		e.type = "SYM"; e.name = name; e.num = index; e.section_num = section_num;
	}
	void updateSymbol(const string& name, const int& value, const string& flags){
		for (int i = 0; i < entries.size(); i++)
			if (entries[i].name == name){
				entries[i].value = value;
				entries[i].flags = flags;
					break; 
			}
	}

	// -1 = no entry, 0 - entry without value, 1 - entry with value
	int defined(const string& name) const{
		for (int i = 0; i < entries.size(); i++){
			if (entries[i].name == name)
				return 0;
				// check if defined
		}
		return -1;
	}



	friend ostream& operator<<(ostream& out, const TableOfSimbols& t){
		out << "#TabelaSimbola" << endl;
		for (Entry e : t.entries)
			out << e << endl;

		return out;
	}
};


// assemble - assembles the source code file into an object code file
// string path_to_source - path to the source file
// returns - path to the object file that was created
string assemble(const string& path_to_source){

	mlog.std("assemble started with " + path_to_source);

	// the group numbers in the regex
	const int label_num = 2;
	const int directive_num = 3;
	const int instruction_num = 5;
	const int op1_num = 7;
	const int op2_num = 9;
	const int op3_num = 11;
	const int comment_num = 14;

	// regex for extracting data from the line
	const regex pattern("^\\s*((\\w[\\w\\d]*)\\s*:)?\\s*(((\\w{2,})(\\s+([\\$\\#]?[\\w\\d]+))?(\\s*,\\s*([\\$\\#]?[\\w\\d]+))?(\\s*,\\s*([\\$\\#]?[\\w\\d]+))?)|()\\.\\w+)(\\s*;(.*))?\\s*$");
	const regex blank("^\\s*$");

	// opening file with the source
	ifstream fin;
	fin.open(path_to_source);

	// checking to see if it exists
	if (!fin.good()){
		// Error
		mlog.error(" *** assemble error - " + path_to_source + " is not good. Exit with code 1");
		exit(1);
	}


	string line;

	while (getline(fin, line)){

		smatch matches;

		// checking if the line is blank
		if (regex_search(line, matches, blank))
			continue;

		// checking to see if the line is valid
		if (!regex_search(line, matches, pattern)) 
			mlog.error("Line '" + line + "' does not match the pattern (nor is blank) => skip");

		// extracting the data
		string label = matches[label_num];
		string direcive = matches[directive_num];
		string instruction = matches[instruction_num];
		string op1 = matches[op1_num];
		string op2 = matches[op2_num];
		string op3 = matches[op3_num];
		string comment = matches[comment_num];


		mlog.std("l:"+label+"#i:"+instruction+"#o1:"+op1+"#op2:"+op2+"#op3:"+op3+"#c:"+comment+"#d:"+direcive);

	}

	return "";
}