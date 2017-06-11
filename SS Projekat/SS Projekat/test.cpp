#include "line.h"
#include "my_util.h"
#include "asm.h"


void test1(){
	mlog.std("Log test");

	mlog.error("Error");

	mlog.std("End of test");
}


void test2(){
	string lines[]{	".data",
					"a : DD a",
					"ORG 0x20",
					"	.text",
					"\tLOAD R1 , a; ucitava sadrzaj memorijske lokacije a u registar R1"
					"LOAD R2, #a; ucitava adresu lokacije a u registar R2"
					"LOAD R3, $a; ucitava sadrzaj lokacije a u registar R3 koristeci PC relativno adresiranje",
					"x : JZ R0, x; apsolutni skok na lokaciju x",
					"JZ R0, $x; PC relativni skok na lokaciju x",
					".end"
				};

	mlog.std("Test for clean_lines\n");

	for (int i = 0; i < 8; i++)
		mlog.std(clean_line(lines[i]));
}

#include <regex>
#include <iostream>

using namespace std;

void test3(){
	regex r("[0-9]*([a-z]+)[0-9]*");
	string text = "0123abc0123";
	smatch matches;

	if (std::regex_search(text, matches, r)) {
		std::cout << "Match found\n";

		for (size_t i = 0; i < matches.size(); ++i) {
			std::cout << i << ": '" << matches[i].str() << "'\n";
		}

	}
	else
		cout << "No match" << endl;
}

// INSTRUCTION and LABEL lines
void test4(){
	const regex r("^\\s*((\\w[\\w\\d]*)\\s*:)?\\s*(((\\w{2,})(\\s+([\\$\\#]?[\\w\\d]+))?(\\s*,\\s*([\\$\\#]?[\\w\\d]+))?(\\s*,\\s*([\\$\\#]?[\\w\\d]+))?)|()\\.\\w+)(\\s*;(.*))?\\s*$");
	//string text = "ADD reg0, reg1, reg2";
	//string text = "PUSH reg";
	//string text = " a : LOAD reg1, $op;aaa";
	//string text = "JZ reg1, op";
	//string text = "RET";
	//string text = "     label :    .data     ; sadadsad";
	string text = ".data";
	//string text = "a: DD a";
	smatch matches;

	if (std::regex_search(text, matches, r)) {
		std::cout << "Match found\n";

		for (size_t i = 0; i < matches.size(); ++i) {
			std::cout << i << ": '" << matches[i].str() << "'\n";
		}

	}
	else
		cout << "No match" << endl;
}





void test5(){
	assemble("example.txt");
}

void test6(){
	Line line1("");
	cout << line1.is_empty() << endl;
	Line line2(" \t");
	cout << line2.is_empty() << endl;

	Line line3(" a ");
	cout << line3.is_empty() << endl;
}

void test7(){
	Line lines[]{" sadsadasdsa ", " sdadsdsa  asdsad  ;sadasddsadsa", " sa dsa;asdsad", " sad sad ;     saddsadadsasad   saddadad     "};
	for (int i = 0; i < 4; i++)
		cout << '*'<<lines[i].get_comment()<<'*' << endl;
}

void test8(){
	Line lines[]{" sadsadasdsa ", " sdadsdsa:  asdsad  ;sadasddsadsa", "   \t sa     : dsa     asdsad", " _12sad_12312:     saddsadadsasad   saddadad     "};
	for (int i = 0; i < 4; i++)
		cout << '*' << lines[i].get_label() << '*' << endl;
}


void test9(){
	regex core("^\\s*(\\w+\\s*:\\s*)?([^;:]*)\\s*(;\\s*(.*))?$");
	string lines[]{	".data",
		"a : DD a",
		"ORG 0x20",
		"	.text",
		"\tLOAD R1 , a; ucitava sadrzaj memorijske lokacije a u registar R1",
		"LOAD R2, #a; ucitava adresu lokacije a u registar R2",
		"LOAD R3, $a; ucitava sadrzaj lokacije a u registar R3 koristeci PC relativno adresiranje",
		"x : JZ R0, x; apsolutni skok na lokaciju x",
		"JZ R0, $x; PC relativni skok na lokaciju x",
		".end"
	};

	smatch match;

	for (int i = 0; i < 10; i++){
		regex_match(lines[i], match, core);
	cout <<'*'<< match[2]<<'*' << endl;
	}

}




void test10(){
	Line lines[]{	".data",
		"a : DD a",
		"ORG 0x20",
		"	.text",
		"\tLOAD R1 , a; ucitava sadrzaj memorijske lokacije a u registar R1",
		"LOAD R2, #a; ucitava adresu lokacije a u registar R2",
		"LOAD R3, $a; ucitava sadrzaj lokacije a u registar R3 koristeci PC relativno adresiranje",
		"x : JZ R0, x; apsolutni skok na lokaciju x",
		"JZ R0, $x; PC relativni skok na lokaciju x",
		".end"
	};

	smatch match;

	for (int i = 0; i < 10; i++){
		cout << lines[i].has_directive() << endl;
		cout << lines[i].get_directive() << endl;
		cout << lines[i].get_directive_name() << endl;
		for (auto j : lines[i].get_split_directive())
			cout << j << '*';
		cout << endl;
	}
}


void test11(){
	Line lines[]{
			"DB 5",
			
			"DB ?",
			
			"DB 3 DUP 5",
			
			"DB 3 DUP ?",
			
			"DB 3 DUP 5, a11 , 2 DUP ?"
		

	};

	smatch match;

	for (int i = 0; i < 5; i++){
		cout << lines[i].has_directive() << endl;
		cout << lines[i].get_directive() << endl;
		cout << lines[i].get_directive_name() << endl;
		for (auto j : lines[i].get_directive_arguments())
			cout << j << '*';
		cout << endl << '*' << endl;
	}
}

#include <regex>

void test12(){
	string lines[] = {
		"DB 5",

		"DB ?",

		"DB 3 DUP 5",

		"DB 3 DUP ?",

		"DB 3 DUP 5, a11 , 2 DUP ?"


	};

	regex r("(?:\\s*([^,]+)\\s*)");
	smatch match;

	regex core("^\\s*(?:DB|DW|DD)\\s+(.*)\\s*$");



	for (int j = 0; j < 5; j++){
		regex_match(lines[j], match, core);
		string line = match[1];

		std::regex_iterator<std::string::const_iterator> rit(line.begin(), line.end(), r);
		std::regex_iterator<std::string::const_iterator> rend;

		list<string> l;

		transform(rit, rend, back_inserter(l),
			[](const regex_iterator<std::string::const_iterator>::value_type& it){ return it[1]; });


		for (auto i : l)
			cout << i << endl;
		cout << '*' << endl << endl;
	}
}

void test13(){

	string num[]{
			"1",
			" 10 ",
			" 0xf",
			" 0b1001",
			" 0b1002",
			"0xabg", 
			"'a'"

	};

	for (int i = 0; i < 7; i++){
		try{
			cout << Line::get_number(num[i]) << endl;
		}
		catch (string s){
			cout << "error *** " << s << endl;
		}
	}

}


void test14(){
	string e[]{
		"1",
			"1+2",
			"3*4+8",
			"1+2-3*4/4",
			"(1+2)*(3-2)+(5-6)", // 4
			"'c'-'a'",
			"-1"

	};

	for (int i = 0; i < 7; i++)
		try{
		mlog.std("*");
		cout << Line::evaluate_expression(e[i]) << endl;
	}
	catch (string s){
		cout << s << endl;
	}
}


void test15(){
	Line lines[] = {
		"DB 5",

		"DD ?",

		"DB 3 DUP 5",

		"DW 3 DUP ?",

		"DB 3 DUP 5, 1 , 2 DUP ?"


	};

	for (int i = 0; i < 5; i++){
		cout << lines[i].get_line() << endl;
		cout << lines[i].get_define_data_size() << endl;
		for (int i : lines[i].get_define_data_values())
			cout << i << " : ";
		cout << endl << "----" << endl;
	}
	
}

void test16(){
	Line l("    \tsimbol DEF 0x2+0b10*3");

	try{
		cout << l.get_directive() << endl;
		cout << l.get_directive_name() << endl;
		for (auto s : l.get_directive_arguments())
			cout << s << " * ";
		cout << endl;
		pair<string, int> p = l.get_symbolic_constant();
		cout << p.first << " = " << p.second << endl;
	}
	catch (string s){
		cout << s << endl;
	}
}

void test17(){
	Line l("   \t ORG 0x20  ");

	try{
		cout << l.get_directive() << endl;
		cout << l.get_directive_name() << endl;
		for (auto s : l.get_directive_arguments())
			cout << s << " * ";
		cout << endl;
		int value = l.get_org_value();
		cout << "value = " << value << endl;
	}
	catch (string s){
		cout << s << endl;
	}
	
}

void test18(){
	Line l("   \t .bss.10  ");

	try{
		cout << l.get_directive() << endl;
		cout << l.get_directive_name() << endl;
		for (auto s : l.get_split_directive())
			cout << s << " * ";
		cout << endl;
		int value = l.get_section_number();
		cout << "section number = " << value << endl;
	}
	catch (string s){
		cout << s << endl;
	}
}


void test19(){
	Line l("   \t .global aaa, bbb,ccc ,   ddd  ");

	try{
		cout << l.get_directive() << endl;
		cout << l.get_directive_name() << endl;
		for (auto s : l.get_split_directive())
			cout << s << " * ";
		cout << endl;

	}
	catch (string s){
		cout << s << endl;
	}
}


void test20(){

	Line l("   \t  .end sadasd sadd ; asdasd");

	try{
		cout << l.get_directive() << endl;
		cout << l.get_directive_name() << endl;
		for (auto s : l.get_split_directive())
			cout << s << " * ";
		cout << endl;

	}
	catch (string s){
		cout << s << endl;
	}
}

void test21(){
	Line lines[]{	".data",
		"a : DD a",
		"ORG 0x20",
		"	.text",
		"\tLOAD R1 , a; ucitava sadrzaj memorijske lokacije a u registar R1",
		"LOAD R2, #a; ucitava adresu lokacije a u registar R2",
		"LOAD R3, $a; ucitava sadrzaj lokacije a u registar R3 koristeci PC relativno adresiranje",
		"x : JZ R0, x; apsolutni skok na lokaciju x",
		"JZ R0, $x; PC relativni skok na lokaciju x",
		".end"
	};


	for (int i = 0; i < 10; i++){
		cout << "line '" + lines[i].get_line()<<"'" << endl;
		cout << "label '" + lines[i].get_label() << "'" << endl;

		cout << lines[i].has_directive() << endl;
		if (lines[i].has_directive()){
			cout << lines[i].get_directive() << endl;
			cout << lines[i].get_directive_name() << endl;
			for (auto s : lines[i].get_directive_arguments())
				cout << s << " * ";
			cout << endl;
		}
		cout << "---" << endl;

	}
}


void test22(){
	Line lines[]{	".data",
		"a : DD a",
		"ORG 0x20",
		"	.text",
		"\tLOAD R1 , a; ucitava sadrzaj memorijske lokacije a u registar R1",
		"LOAD R2, #a; ucitava adresu lokacije a u registar R2",
		"LOAD R3, $a; ucitava sadrzaj lokacije a u registar R3 koristeci PC relativno adresiranje",
		"x : JZ R0, x; apsolutni skok na lokaciju x",
		"JZ R0, $x; PC relativni skok na lokaciju x",
		".end"
	};


	for (int i = 0; i < 10; i++){
		try{
			cout << lines[i].get_line() << endl;
			cout << lines[i].get_core() << endl;
			cout << lines[i].has_instruction() << endl;

			if (lines[i].has_instruction()){
				cout <<"'"<< lines[i].get_instruction_name()<<"'" << endl;
				for (auto s : lines[i].get_instruction_arguments())
					cout << s << " * ";
				cout << endl;
				cout << "size of instruction is " << lines[i].get_instruction_size() << endl;
			}

			cout << "---" << endl << endl;
		}
		catch (string s){
			cout << s << endl;
		}
	}
}






int main(){
	//test1();
	//test2();

	//test3();
	//test4();

	test22();

	
}