#ifndef _asm_h_
#define _asm_h_

#include <string>
using namespace std;


// assemble - assembles the source code file into an object code file
// string path_to_source - path to the source file
// returns - path to the object file that was created
string assemble(const string& path_to_source);

#endif