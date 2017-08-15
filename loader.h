#ifndef _loader_h_
#define _loder_h_

#include <string>

#include "emulator.h"


using namespace std;

unsigned load(string path_to_obj, Memory& mem);

#endif