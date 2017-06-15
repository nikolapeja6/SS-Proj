#ifndef _content_table_h_
#define _content_table_h_

#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <list>

using namespace std;

class ContentTable{

	string section_name;
	vector<uint8_t> data;

public:
	

	ContentTable();
	ContentTable(string name);

	void append(vector<uint8_t>);
	void append(list<uint8_t>);

	void set_section_name(string name);

	void empty();

	friend ostream& operator<<(ostream&, const ContentTable&);

	string str()const;

};


#endif