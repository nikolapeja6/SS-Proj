#include "emulator.h"
#include "loader.h"


#include <iostream>
#include <ncurses.h>

using namespace std;


int main(int argc, char* argv[]){



	if (argc != 2){
		cout << "Error number of argumetns is not 1" << endl;
		return 1;
	}
#ifdef __linux
initscr();

	cbreak();
	noecho();
	nodelay(stdscr, TRUE);

	scrollok(stdscr, TRUE);
#endif


	Context con;

#ifdef __linux
printw("start loading\n");
#else
	cout << "starting loading" << endl;
#endif
	try{

		
		string input_file = argv[1];
		uint32_t START = load(input_file, con.mem);
#ifdef __linux
printw("finished loading");
refresh();
printw("started execution");
refresh();
#else
		cout << "finished loading" << endl;
		cout << "string execution" << endl;
#endif
		con.PC = START;
		con.execute();

#ifdef __linux
printw("Successfully executed the obj file");
//endwin();
#else
		cout << "Successfully executed the obj file" << endl;
#endif
	}
	catch (string s){
#ifdef __linux
endwin();
#endif
		cout << "Error occured - " << s << endl;
		return 2;
	}
	

	return 0;
}
