#include "emulator.h"
#include "my_util.h"

#include <sstream>
#include <iomanip>
#include <chrono>
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#endif

#ifdef __linux
#include <ncurses.h>
#endif



using namespace std;


void timer_body(Context*);

Context::Context() :done(false), timer_thread(timer_body, this), /*reader_thread(reader_body, this), */mode(1),
REG({ { 0x00, &R[0] }, { 0x01, &R[1] }, { 0x02, &R[2] }, { 0x03, &R[3] }, { 0x04, &R[4] }, { 0x05, &R[5] }, { 0x06, &R[6] }, { 0x07, &R[7] },
{ 0x08, &R[8] }, { 0x09, &R[9] }, { 0x0a, &R[10] }, { 0x0b, &R[11] }, { 0x0c, &R[12] }, { 0x0d, &R[13] }, { 0x0e, &R[14] }, { 0x0f, &R[15] },
{ 0x10, &SP }, { 0x11, &PC }
}){
	for (int i = 0; i < 32; i++)
		interrupts[i] = 0;

	mem.new_read = true;


#ifdef __linux
/*	initscr();

	cbreak();
	noecho();
	nodelay(stdscr, TRUE);

	scrollok(stdscr, TRUE);
*/
#endif

}


Context::~Context(){
	done = true;

	timer_thread.join();
	/*
	reader_thread.detach();
	reader_thread.~thread();
	*/

	#ifdef __linux
 //endwin(); 
#endif
}



unordered_map<string, unsigned char>  Context::address_codes{
	{ "immed", 0x4 },
	{ "regdir", 0x0 },
	{ "memdir", 0x6 },
	{ "regind", 0x2 },
	{ "regindoff", 0x7 }
};

unordered_map<string, uint8_t> Context::type_codes{
	{ "UB", 0x03 },
	{ "SB", 0x07 },
	{ "UW", 0x01 },
	{ "SW", 0x05 },
	{ "", 0x00 },
	{ "B", 0x03 },
	{ "W", 0x01 }
};


void timer_body(Context* c){

	while (!c->done){

		auto begin = std::chrono::system_clock::now();

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		c->interrupts[Context::TIMER_ENTRY] = true;
	}
}



/*


void reader_body(Context* c){

	//cout << "reader started" << endl;

	int i = 0;

	while (!c->done){

		//cout << "reader " << (c->mem.new_read ? "true" : "false") << endl;

		while (!c->mem.new_read);
		
		if (c->done)
			break;

		// FETCH NEW CHAR
		char data = getchar();

		mlog.error(to_string(data));
		i++;
		//cout << "reader '" + data << endl;

		c->mem.INPUT_BUFFER = data;
		mlog.error(to_string(c->mem.INPUT_BUFFER));
		c->new_read = false;
		c->interrupts[5] = true;

		std::this_thread::sleep_for(std::chrono::milliseconds(Context::DELTA));

	}

	mlog.std("reader ended");
	
}
*/





Memory::Memory():new_read(true), INPUT_BUFFER(0){
	for (int i = 0; i < MAX_MEM_SIZE; i++)
		mem[i] = UINT8_MAX;
}

uint8_t Memory::get_byte(unsigned address){
	
	if (address >= MAX_MEM_SIZE){
		string error = "Error accessing memory - address " + to_string(address) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}

	if (address == INPUT_ADDRESS){
		uint8_t ret = INPUT_BUFFER;
		new_read = true;
		mlog.error("X");
		return ret;
	}

	return mem[address];
}

uint16_t Memory::get_word(unsigned address){

	if (address+1 >= MAX_MEM_SIZE){
		string error = "Error accessing memory for word - address " + to_string(address+1) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}


	uint16_t ret = mem[address] | (uint16_t(mem[address + 1]) << 8);
	return ret;
}

uint32_t Memory::get_dword(unsigned address){

	if (address + 3 >= MAX_MEM_SIZE){
		string error = "Error accessing memory for word - address " + to_string(address + 3) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}


	uint32_t ret = 0;
	for (int i = 0; i < 4; i++){
		ret |= (uint32_t(mem[address + i])<<(i*8));
	}

	if (address == 800){
		ret++;
		ret--;

	}

	return ret;
}

void Memory::set_byte(unsigned address, uint8_t byte){
	
	if (address >= MAX_MEM_SIZE){
		string error = "Error accessing memory - address " + to_string(address) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}

	if (address == INPUT_ADDRESS){
		string error = "Error - user program tries to write in INPUT REGISTER";
		mlog.error(error);
		throw error;
	}

	if (address == OUTPUT_ADDRESS){
		mlog.std("for out " + to_string(byte));
		cout << (char)byte;
	}

	if (address == 800){
		mlog.std("set_byte " + to_string(byte));
	}

	mem[address] = byte;

}

void Memory::set_word(unsigned address, uint16_t word){

	if (address + 1 >= MAX_MEM_SIZE){
		string error = "Error accessing memory for word - address " + to_string(address + 1) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}

	uint8_t data[2];

	for (int i = 0; i < 2; i++){
		data[i] = word & UINT8_MAX;
		word >>= 8;
	}

	if (address == 800){
		mlog.std("set_byte " + to_string(word));
	}

	for (int i = 0; i < 2; i++)
		mem[address + i] = data[i];

}

void Memory::set_dword(unsigned address, uint32_t dword){

	if (address + 4 >= MAX_MEM_SIZE){
		string error = "Error accessing memory for word - address " + to_string(address + 4) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}

	uint8_t data[4];

	for (int i = 0; i < 4; i++){
		data[i] = dword & UINT8_MAX;
		dword >>= 8;
	}
	if (address == 800 && !(dword <= 10 && dword >=0)  ){
		mlog.std("set_byte " + to_string(dword));
	}



	for (int i = 0; i < 4; i++)
		mem[address + i] = data[i];

}

string Memory::str()const{
	stringstream out;

	out << "\t";
	for (int i = 0; i < 10; i++)
		out<< setw(4)<<i << " ";
	out << endl << endl;

	for (int i = 0, k=0; i < MAX_MEM_SIZE;k++){
		out << k << "\t";
		for (int j = 0; j < 10 && i < MAX_MEM_SIZE; j++, i++)
			out << "0x" << hex << setw(2) <<setfill('0')<< (int)mem[i] << " ";

		out << endl << endl;
	}

	return out.str();
}

const uint8_t& Memory::operator[](unsigned address)const{

	if (address >= MAX_MEM_SIZE){
		string error = "Error accessing memory - address " + to_string(address) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}

	return mem[address];
}

uint8_t& Memory::operator[](unsigned address){
	
	if (address >= MAX_MEM_SIZE){
		string error = "Error accessing memory - address " + to_string(address) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}

	return mem[address];
}



void Context::execute(){
	
	mlog.std("execute started");

	SP = mem.get_dword(0);


	
	while (true){

		refresh();

		if (mode == 0)
		mlog.std(" start " + to_string(R[2]));

		// INSTRUCTION DECODING AND EXECUTION

		uint32_t instruction1;
		instruction1 = mem.get_dword(PC);

		uint8_t opcode = (instruction1 >> Context::OPCODE_OFF) & Context::OPCODE_MASK;
		uint8_t addrmode = (instruction1 >> Context::ADDRMODE_OFF) & Context::ADDRMODE_MASK;
		uint8_t reg0 = (instruction1 >> Context::REG0_OFF) & Context::REG0_MASK;
		uint8_t reg1 = (instruction1 >> Context::REG1_OFF) & Context::REG2_MASK;
		uint8_t reg2 = (instruction1 >> Context::REG2_OFF) & Context::REG1_MASK;
		uint8_t type = (instruction1 >> Context::TYPE_OFF) & Context::TYPE_MASK;

		string error;
		int32_t arg0;
		int32_t arg1;
		int32_t arg2;

		switch (opcode){

		case 0x00: // INT

			mlog.std("found INT");

			if (Context::address_codes["regrir"] != addrmode){
				error = "Instruction format error. INT can have only regdir, but isn't.";
				mlog.error(error);
				throw error;
			}

			arg0 = *REG[reg0];

			PC += 4;
			INT(arg0);

			break;


		case 0x01: // RET

			mlog.std("found RET");

			PC += 4;
			RET();

			break;


		case 0x02: // JMP
		case 0x03: // CALL

			mlog.std("found JMP, CALL");

			if (PC == 1064){
				mlog.std("a");
			}


			// REGIND
			if (Context::address_codes["regind"] == addrmode){
				
				int r = *REG[reg0];
				arg0 = r;

				PC += 4;

				if (opcode == 0x02)
					JMP(arg0);
				else
					CALL(arg0);

				break;

			}

			// REGINDOFF
			if (Context::address_codes["regindoff"] == addrmode){

				int r = *REG[reg0];

				int32_t offset = mem.get_dword(PC + 4);
				arg0 += r + offset;

				PC += 8;

				if (opcode == 0x02)
					JMP(arg0);
				else
					CALL(arg0);

				break;

			}

			// MEMDIR
			if (Context::address_codes["memdir"] == addrmode){

				int32_t address = mem.get_dword(PC + 4);

				arg0 = address;

				PC += 8;

				if (opcode == 0x02)
					JMP(arg0);
				else
					CALL(arg0);

				break;

			}



			error = "Instruction format error. JMP, CALL can have only regdind, regindoff and memdir, but has somethin else. PC = "+to_string(PC);
			mlog.error(error);
			throw error;

			break;


		case 0x04: // JZ
		case 0x05: // JNZ
		case 0x06: // JGZ
		case 0x07: // JGEZ
		case 0x08: // JLZ
		case 0x09: // JLEZ

			mlog.std("JZ, JNZ, JGZ, JGEZ, JLZ, JLEZ ");


			arg0 = *REG[reg0];


			// NOT IMMED and NOT REGDIR 2.
			if (addrmode == Context::address_codes["immed"] || addrmode == Context::address_codes["regdir"]){
				error = "Instruction format error. JZ, JNZ, JGZ, JGEZ, JLZ, JLEZ must have MEMDIR, REGIND or REGINDOFF as 2. argument.";
				mlog.error(error);
				throw error;
			}

			// REGIND
			if (addrmode == Context::address_codes["regind"]){

				int r = *REG[reg1];
				arg1 = r;

				PC += 4;

				switch (opcode){
				case 0x04: // JZ
					JZ(arg0, arg1);
						break;
				case 0x05: // JNZ
					JNZ(arg0, arg1);
					break;
				case 0x06: // JGZ
					JGZ(arg0, arg1);
					break;
				case 0x07: // JGEZ
					JGEZ(arg0, arg1);
					break;
				case 0x08: // JLZ
					JLZ(arg0, arg1);
					break;
				case 0x09: // JLEZ
					JLEZ(arg0, arg1);
					break;
				}

				break;

			}


			// REGINDOFF
			if (addrmode == Context::address_codes["regindoff"]){

				int32_t r = *REG[reg1];
				
				int32_t offset = mem.get_dword(PC + 4);
				arg1 = r + offset;

				PC += 8;

				switch (opcode){
				case 0x04: // JZ
					JZ(arg0, arg1);
					break;
				case 0x05: // JNZ
					JNZ(arg0, arg1);
					break;
				case 0x06: // JGZ
					JGZ(arg0, arg1);
					break;
				case 0x07: // JGEZ
					JGEZ(arg0, arg1);
					break;
				case 0x08: // JLZ
					JLZ(arg0, arg1);
					break;
				case 0x09: // JLEZ
					JLEZ(arg0, arg1);
					break;
				}

				break;
			}

		
			// MEMDIR
			if (addrmode == Context::address_codes["memdir"]){

				int32_t address = mem.get_dword(PC + 4);
				arg1 = address;

				PC += 8;

				switch (opcode){
				case 0x04: // JZ
					JZ(arg0, arg1);
					break;
				case 0x05: // JNZ
					JNZ(arg0, arg1);
					break;
				case 0x06: // JGZ
					JGZ(arg0, arg1);
					break;
				case 0x07: // JGEZ
					JGEZ(arg0, arg1);
					break;
				case 0x08: // JLZ
					JLZ(arg0, arg1);
					break;
				case 0x09: // JLEZ
					JLEZ(arg0, arg1);
					break;
				}

				break;
			}



			error = "Instruction format error. JZ, JNZ, JGZ, JGEZ, JLZ, JLEZ can have only regdind, regindoff and memdir, but has somethin else.";
			mlog.error(error);
			throw error;

			break;



		case 0x10: // LOAD
		case 0x11: // STORE

			mlog.std("LOAD, STORE");


			// REGDIR
			if (addrmode == Context::address_codes["regdir"]){

				if (opcode == 0x10){ // LOAD
					if (type == Context::type_codes["UB"]){
						uint8_t data = *REG[reg1] & UINT8_MAX;
						*REG[reg0] = data;
					}
					else  if (type == Context::type_codes["SB"]){
						int8_t data = *REG[reg1] & UINT8_MAX;
						*REG[reg0] = data;
					}
					else if (type == Context::type_codes["UW"]){
						uint16_t data = *REG[reg1] & UINT16_MAX;
						*REG[reg0] = data;
					}
					else if (type == Context::type_codes["SW"]){
						int16_t data = *REG[reg1] & UINT16_MAX;
						*REG[reg0] = data;
					}
					else *REG[reg0] = *REG[reg1];
				}
				else{ // STORE
					if (type == Context::type_codes["B"]){
						*REG[reg1] &= 0xffffff00;
						uint8_t data = *REG[reg0] & UINT8_MAX;
						*REG[reg1] |= data;
					}
					else if(type == Context::type_codes["W"]){
						*REG[reg1] &= 0xffff0000;
						uint8_t data = *REG[reg0] & UINT16_MAX;
						*REG[reg1] |= data;
					}
					else *REG[reg1] = *REG[reg0];
				}

				PC += 4;

				break;

			}

			// IMMED 
			if (addrmode == Context::address_codes["immed"]){
				if (opcode == 0x11){
					error = "Instruction format error. IMMED not allowed in STORE. ";
					mlog.error(error);
					throw error;
				}

				int32_t value = mem.get_dword(PC + 4);
				*REG[reg0] = value;

				PC += 8;

				break;
			}

			// REGIN
			if (addrmode == Context::address_codes["regind"]){
				arg1 = *REG[reg1];		
				PC += 4;
			}
			else
			// REGINDOFF 
			if (addrmode == Context::address_codes["regindoff"]){
				int32_t offset = mem.get_dword(PC + 4);
				arg1 = *REG[reg1] + offset;
				PC += 8;
			}
			// MEMDIR
			else{
				int32_t address = mem.get_dword(PC + 4);
				arg1 = address;
				PC += 8;
			}

			if (opcode == 0x10){ // LOAD
				if (type == Context::type_codes["UB"]){
					uint8_t data = mem.get_byte(arg1);
					*REG[reg0] = data;
				}
				else  if (type == Context::type_codes["SB"]){
					int8_t data = mem.get_byte(arg1);
					*REG[reg0] = data;
				}
				else if (type == Context::type_codes["UW"]){
					uint16_t data = mem.get_word(arg1);
					*REG[reg0] = data;
				}
				else if (type == Context::type_codes["SW"]){
					int16_t data = mem.get_word(arg1);
					*REG[reg0] = data;
				}
				else *REG[reg0] = mem.get_dword(arg1);
			}
			else{ // STORE
				if (arg1 >= 800 && arg1 <= 803){
					mlog.std( "STORE PC = " + to_string( PC));
				}

				if (type == Context::type_codes["B"]){
					uint8_t data = *REG[reg0] & UINT8_MAX;
					
					mem.set_byte(arg1, data);
				}
				else if (type == Context::type_codes["W"]){
					uint16_t data = *REG[reg0] & UINT16_MAX;
					mem.set_word(arg1, data);
				}
				else {
					if (opcode == 0x11 && addrmode == 6 && type == 0){
						mlog.std("reg0 = "+to_string(reg0)+" value = "+to_string(*REG[reg0]));
					}
					mem.set_dword(arg1, *REG[reg0]);
					
				}
			}

			break;


		case 0x20: // PUSH
			mlog.std("PUSH");

			if (addrmode != Context::address_codes["regdir"]){
				error = "Instruction format error. PUSH can only have REGDIR.";
				mlog.error(error);
				throw error;
			}

			arg0 = *REG[reg0];
			PUSH(arg0);

			PC += 4;

			break;


		case 0x21: // POP

			mlog.std("POP");

			if (addrmode != Context::address_codes["regdir"]){
				error = "Instruction format error. POP can only have REGDIR.";
				mlog.error(error);
				throw error;
			}

			arg0 = POP();
			*REG[reg0] = arg0;

			PC += 4;

			break;



		case 0x30: // ADD
			mlog.std("ADD");
			ADD(reg0, reg1, reg2);
			PC += 4;
			break;
		case 0x31: // SUB
			mlog.std("SUB");
			SUB(reg0, reg1, reg2);
			PC += 4;
			break;
		case 0x32: // MUL
			mlog.std("MUL");
			MUL(reg0, reg1, reg2);
			PC += 4; 
			break;
		case 0x33: // DIV
			mlog.std("DIV");
			DIV(reg0, reg1, reg2);
			PC += 4; 
			break;
		case 0x34: // MOD
			mlog.std("MOD");
			MOD(reg0, reg1, reg2);
			PC += 4; 
			break;
		case 0x35: // AND
			mlog.std("AND");
			AND(reg0, reg1, reg2);
			PC += 4; 
			break;
		case 0x36: // OR
			mlog.std("OR");
			OR(reg0, reg1, reg2);
			PC += 4; 
			break;
		case 0x37: // XOR
			mlog.std("XOR");
			XOR(reg0, reg1, reg2);
			PC += 4; 
			break;
		case 0x38: // NOT
			mlog.std("NOT");
			NOT(reg0, reg1);
			PC += 4; 
			break;
		case 0x39: // ASL
			mlog.std("ASL");
			ASL(reg0, reg1, reg2);
			PC += 4; 
			break;
		case 0x3A: // ASR
			mlog.std("ASR");
			ASR(reg0, reg1, reg2);
			PC += 4; 
			break;

		default:
			error = "Error - unknown instruction with opcode " + to_string(opcode);
			mlog.error(error);
			throw error;
		
		}

		//if (mode == 0)
		//mlog.std(" end " + to_string(R[2])+" opcode = "+to_string(opcode) + " type = "+to_string(type)+" addrmode "+to_string(addrmode)+ " ");
		if (opcode == 0x11)
			mlog.std(" opcode = "+to_string(opcode) + " type = "+to_string(type)+" addrmode "+to_string(addrmode)+ " mem[800] "+to_string(mem.get_dword(800))+" arg1 " + to_string(arg1));


		// END OF INSTRUCTION EXECUTION


		// CHECK FOR INTERUPTS

		if (interrupts[0] == true){
			done = false;
			break;
		}

		if (mode == 0)
			continue;


		// READER

		// WIN
#ifdef _WIN32
		if (kbhit() && mem.new_read){
			char data = _getch();
			//cin.get(data);
			mem.INPUT_BUFFER = data;
			//mlog.error(to_string(mem.INPUT_BUFFER));
			mem.new_read = false;
			interrupts[5] = true;
			putch(data);
			//cout << data;
		}
#endif

		// LINUX
#ifdef __linux
		
		mlog.std("linux");
		if(mem.new_read){

char ch = getch();
if(ch != ERR){
			mem.INPUT_BUFFER = ch;
			mem.new_read = false;
	interrupts[5] = true;
addch(ch);
}
			 
		}
		
#endif

		uint32_t address;
		int i;
		for (i = 1; i < 32; i++)
			if (interrupts[i]){
				address = mem.get_dword(i * 4);
				if (address != 0)
					break;
			}
		if (i >= 32)
			continue;

		

		mode = 0;
		save_context();
		interrupts[i] = false;

		mlog.error("int");

		PC = address;
		

		}


	

	mlog.std("execute finished");


}


void Context::save_context(){

	for (int i = 0; i < 16; i++){
		SP += 4;
		mem.set_dword(SP, R[i]);
	}

	SP += 4;

	if (SP >= 800 && SP <= 803){
		mlog.std(" save context PC = " + to_string(PC));
	}

	mem.set_dword(SP, PC);
}

void Context::restore_context(){
	
	uint32_t value;

	for (int i = 15; i >= 0; i--){
		value = mem.get_dword(SP);
		R[i] = value;
		SP -= 4;
	}
}



void Context::INT(uint32_t entry){
	mlog.error("INT");
	if (entry >= 32){
		string error = "Error - INT caled for entry greather than 31 - called for " + to_string(entry);
		mlog.error(error);
		throw error;
	}

	interrupts[entry] = true;
}

void Context::JMP(uint32_t address){
	mlog.error("JMP");
	PC = address;
}

void Context::CALL(uint32_t function){
	mlog.error("CALL");
	SP += 4;
	if (SP >= 800 && SP <= 803){
		mlog.std(" CALL PC = " + to_string(PC));
	}

	mem.set_dword(SP, PC);
	PC = function;
}

void Context::RET(){
	mlog.error("RET");
	uint32_t val;
	val = mem.get_dword(SP);
	SP -= 4;
	PC = val;
	if (mode == 0){
		restore_context();
		mode = 1;
	}
}

void Context::JZ(int32_t value, uint32_t address){
	if (value == 0)
		PC = address;
}
void Context::JNZ(int32_t value, uint32_t address){
	if (value != 0)
		PC = address;
}

void Context::JGZ(int32_t value, uint32_t address){
	if (value > 0)
		PC = address;
}

void Context::JGEZ(int32_t value, uint32_t address){
	//mlog.error("JGEZ");
	if (value >= 0)
		PC = address;
}

void Context::JLZ(int32_t value, uint32_t address){

	if (value < 0){
		mlog.std("JLZ done");
		PC = address;
	}
	else{
		//cout << hex << value << endl;
	}
}

void Context::JLEZ(int32_t value, uint32_t address){
	if (value <= 0)
		PC = address;
}

void Context::PUSH(uint32_t value){
	SP += 4;

	if (SP >= 800 && SP <= 803){
		mlog.std("PUSH PC = " + to_string(PC));
	}
	mem.set_dword(SP, value);
}

uint32_t Context::POP(){
	uint32_t value;
	value = mem.get_dword(SP);
	SP -= 4;
	return value;
}

void Context::ADD(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = (int)*REG[reg1] + (int)*REG[reg2];
}

void Context::SUB(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	
	*REG[reg0] = (int)*REG[reg1] - (int)*REG[reg2];
	mlog.std(to_string(*REG[reg1]) + " - " + to_string(*REG[reg2]) + " = " + to_string(*REG[reg0]));
	//cout << "S"<<hex << *REG[reg0] << endl;
}

void Context::MUL(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = (int)*REG[reg1] * (int)*REG[reg2];
}

void Context::DIV(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	if (reg2 != 0)
		*REG[reg0] = (int)*REG[reg1] / (int)*REG[reg2];
	else{
		interrupts[3] = true;
		string error = "DIVISION BY 0";
		mlog.error(error);
	}
}

void Context::MOD(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = (int)*REG[reg1] % (int)*REG[reg2];
}

void Context::AND(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = *REG[reg1] & *REG[reg2];
}

void Context::OR(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = *REG[reg1] | *REG[reg2];
}

void Context::XOR(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = *REG[reg1] ^ *REG[reg2];
}

void Context::ASL(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = *REG[reg1] << *REG[reg2];
}

void Context::ASR(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = uint32_t(*REG[reg1]) >> *REG[reg2];
}

void Context::NOT(uint8_t reg0, uint8_t reg1){
	*REG[reg0] = ~*REG[reg1];
}

