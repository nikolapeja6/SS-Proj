#include "emulator.h"
#include "my_util.h"

#include <sstream>
#include <iomanip>
#include <chrono>
#include <iostream>

using namespace std;


Context::Context() :done(false), timer_thread(timer_body, this), reader_thread(reader_body, this), new_read(true),
REG({ { 0x00, &R[0] }, { 0x01, &R[1] }, { 0x02, &R[2] }, { 0x03, &R[3] }, { 0x04, &R[4] }, { 0x05, &R[5] }, { 0x06, &R[6] }, { 0x07, &R[7] },
{ 0x08, &R[8] }, { 0x09, &R[9] }, { 0x0a, &R[10] }, { 0x0b, &R[11] }, { 0x0c, &R[12] }, { 0x0d, &R[13] }, { 0x0e, &R[14] }, { 0x0f, &R[15] },
{ 0x10, &SP }, { 0x11, &PC }
}){}

Context::~Context(){
	done = true;

	timer_thread.join();
	reader_thread.detach();
	reader_thread.~thread();
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

		std::this_thread::sleep_for(std::chrono::milliseconds(Context::DELTA));
		c->interrupts[Context::TIMER_ENTRY] = true;
	}
}

void reader_body(Context* c){

	while (!c->done){
		while (!c->done && !c->new_read);
		
		if (c->done)
			break;
		char data = getchar();

		c->mem[Context::INPUT_ADDRESS] = data;
		c->new_read = false;

	}
	
}





Memory::Memory(){
	for (int i = 0; i < MAX_MEM_SIZE; i++)
		mem[i] = UINT8_MAX;
}

uint8_t Memory::get_byte(unsigned address)const{
	
	if (address >= MAX_MEM_SIZE){
		string error = "Error accessing memory - address " + to_string(address) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}

	return mem[address];
}

uint16_t Memory::get_word(unsigned address)const{

	if (address+1 >= MAX_MEM_SIZE){
		string error = "Error accessing memory for word - address " + to_string(address+1) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}


	uint16_t ret = mem[address] | (uint16_t(mem[address + 1]) << 8);
	return ret;
}

uint32_t Memory::get_dword(unsigned address)const{

	if (address + 3 >= MAX_MEM_SIZE){
		string error = "Error accessing memory for word - address " + to_string(address + 3) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
	}


	uint32_t ret = 0;
	for (int i = 0; i < 4; i++){
		ret |= (uint32_t(mem[address + i])<<(i*8));
	}

	return ret;
}

void Memory::set_byte(unsigned address, uint8_t byte){
	
	if (address >= MAX_MEM_SIZE){
		string error = "Error accessing memory - address " + to_string(address) + " is out of bounds. MAX_MEM_SIZE is " + to_string(MAX_MEM_SIZE);
		mlog.error(error);
		throw error;
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

	while (true){

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


			// REGIND
			if (Context::address_codes("regind") == addrmode){
				
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
			if (Context::address_codes("regindoff") == addrmode){

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
			if (Context::address_codes("memdir") == addrmode){

				int32_t address = mem.get_dword(PC + 4);

				arg0 = address;

				PC += 8;

				if (opcode == 0x02)
					JMP(arg0);
				else
					CALL(arg0);

				break;

			}



			error = "Instruction format error. JMP, CALL can have only regdind, regindoff and memdir, but has somethin else.";
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
				if (type == Context::type_codes["B"]){
					uint8_t data = *REG[reg0] & UINT8_MAX;
					mem.set_byte(arg1, data);
				}
				else if (type == Context::type_codes["W"]){
					uint16_t data = *REG[reg0] & UINT16_MAX;
					mem.set_word(arg1, data);
				}
				else mem.set_dword(arg1, *REG[reg0]);
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


		// END OF INSTRUCTION EXECUTION


		// CHECK FOR INTERUPTS
		

		}


	

	mlog.std("execute finished");


}



void Context::INT(uint32_t){

}

void Context::JMP(uint32_t address){
	PC = address;
}

void Context::CALL(uint32_t function){
	SP += 4;
	mem.set_dword(SP, PC);
	PC = function;
}

void Context::RET(){
	uint32_t val;
	val = mem.get_dword(SP);
	SP -= 4;
	PC = val;
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
	if (value >= 0)
		PC = address;
}

void Context::JLZ(int32_t value, uint32_t address){
	if (value < 0)
		PC = address;
}

void Context::JLEZ(int32_t value, uint32_t address){
	if (value <= 0)
		PC = address;
}

void Context::PUSH(uint32_t value){
	SP += 4;
	mem.set_dword(SP, value);
}

uint32_t Context::POP(){
	uint32_t value;
	value = mem.get_dword(SP);
	SP -= 4;
	return value;
}

void Context::ADD(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = *REG[reg1] + *REG[reg2];
}

void Context::SUB(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = *REG[reg1] - *REG[reg2];
}

void Context::MUL(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = *REG[reg1] * *REG[reg2];
}

void Context::DIV(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = *REG[reg1] / *REG[reg2];
}

void Context::MOD(uint8_t reg0, uint8_t reg1, uint8_t reg2){
	*REG[reg0] = *REG[reg1] % *REG[reg2];
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

