#ifndef _log_h_
#define _log_h_

#include <string>
#include <fstream>

using namespace std;

// class used for logging
class Log{

	// files for logging error and standard messages
	ofstream error_file;
	ofstream log_file;

public:

	Log();
	~Log();

	// log for errors
	void error(const string& str);

	// log for standard messages
	void std(const string& str);
};


#endif