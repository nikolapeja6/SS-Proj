#include "log.h"

Log mlog;

Log::Log(){
	error_file.open("error.txt");
	log_file.open("log.txt");
}

Log::~Log(){
	error_file.close();
	log_file.close();
}

//	for logging standard messages
void Log::std(const string& str){
	log_file << str << endl;
}

// for logging error messages
void Log::error(const string& str){
	error_file << str << endl;
	
	log_file << str << endl;
	
}