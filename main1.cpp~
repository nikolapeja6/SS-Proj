#include "asm.h"

#include <iostream>

using namespace std;

int main1(int argc, char* argv[]){

	if (argc != 2){
		cout << "Error number of argumetns is not 1" << endl;
		return 1;
	}

	cout << "starting assembling" << endl;

	try{
		string out_file;
		string input_file = argv[1];
		out_file = assemble(input_file);

		cout << "Successfully finished assembling. Output file is '" + out_file + "'" << endl;
	}
	catch (string s){
		cout << "Error occured during assembling - " << s << endl;
		return 2;
	}

	return 0;
}