#ifndef _content_table_h_
#define _content_table_h_

#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <list>

using namespace std;

#include "emulator.h"


class ContentTable{

	string section_name;
	vector<unsigned  char> data;

public:
	

	ContentTable();
	ContentTable(string name);

	void append(vector<unsigned char>);
	void append(list<unsigned char>);
	void append(unsigned char);

	void set_section_name(string name);

	void empty();

	friend ostream& operator<<(ostream&, const ContentTable&);
	friend istream& operator>>(istream&, ContentTable&);

	string str()const;

	friend unsigned load(string path_to_obj, Memory& mem);


};


#endif