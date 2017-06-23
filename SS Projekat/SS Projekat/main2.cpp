#include "emulator.h"
#include "loader.h"


#include <iostream>

using namespace std;


int main(int argc, char* argv[]){

	if (argc != 2){
		cout << "Error number of argumetns is not 1" << endl;
		return 1;
	}

	cout << "starting loading" << endl;

	try{

		Context con;
		string input_file = argv[1];
		uint32_t START = load(input_file, con.mem);

		cout << "finished loading" << endl;

		cout << "string execution" << endl;

		con.PC = START;
		con.execute();

		cout << "Successfully executed the obj file" << endl;
	}
	catch (string s){
		cout << "Error occured - " << s << endl;
		return 2;
	}

	return 0;
}