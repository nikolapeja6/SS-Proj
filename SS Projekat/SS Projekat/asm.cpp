#include "my_util.h"


// assemble - assembles the source code file into an object code file
// string path_to_source - path to the source file
// returns - path to the object file that was created
string assemble(const string& path_to_source){

	mlog.std("assemble started with " + path_to_source);

	
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

		

	}

	return "";
}