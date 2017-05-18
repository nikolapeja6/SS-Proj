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





int main(){
	//test1();
	//test2();

	//test3();
	//test4();

	test5();
}