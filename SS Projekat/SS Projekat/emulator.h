#ifndef _emulator_h_
#define _emulator_h_

#include <cstdint>
#include <string>
#include <thread>
#include <condition_variable>
#include <unordered_map>

using namespace std;

class Memory{

public:
	static const unsigned MAX_MEM_SIZE = 2000;

private:

	uint8_t mem[Memory::MAX_MEM_SIZE];

public:

	Memory();

	uint8_t  get_byte(unsigned address)  const;
	uint16_t get_word(unsigned address)  const;
	uint32_t get_dword(unsigned address) const;

	void set_byte(unsigned address, uint8_t byte);
	void set_word(unsigned address, uint16_t word);
	void set_dword(unsigned address, uint32_t dword);

	const uint8_t& operator[] (unsigned address)const;
	uint8_t& operator[] (unsigned address);

	string str()const;




};

class Context{

	// REGISTERS
	int32_t R[16];
	int32_t PC;
	int32_t SP;

	// 0 - Kernel, 1 - User
	int mode;


	volatile bool interrupts[32];
	static const unsigned TIMER_ENTRY = 4;

	Memory mem;

public:

	Context();
	~Context();

	void execute();

private:

	volatile bool done;

	friend void reader_body(Context* );
	bool new_read;
	thread reader_thread;
	static const unsigned INPUT_ADDRESS = 36;

	thread timer_thread;
	friend void timer_body(Context* c);
	static const unsigned DELTA = 100; // time in millisecond for iteration of timer
	


	static const unsigned OPCODE_OFF = 24;
	static const unsigned ADDRMODE_OFF = 21;
	static const unsigned REG0_OFF = 16;
	static const unsigned REG1_OFF = 11;
	static const unsigned REG2_OFF = 6;
	static const unsigned TYPE_OFF = 3;

	static const unsigned OPCODE_MASK = 0xff;		// 1111 1111
	static const unsigned ADDRMODE_MASK = 0x07;		// 0000 0111
	static const unsigned REG0_MASK = 0x1f;			// 0001 1111	
	static const unsigned REG1_MASK = 0x1f;			// 0001 1111
	static const unsigned REG2_MASK = 0x1f;			// 0001 1111
	static const unsigned TYPE_MASK = 0x07;			// 0000 0111


	static unordered_map<string, unsigned char>  address_codes;
	static unordered_map<string, uint8_t> type_codes;

	unordered_map<uint8_t, int32_t*> REG;

	void INT(uint32_t);
	void JMP(uint32_t);
	void CALL(uint32_t);
	void RET();
	void JZ(int32_t, uint32_t);
	void JNZ(int32_t, uint32_t);
	void JGZ(int32_t, uint32_t);
	void JGEZ(int32_t, uint32_t);
	void JLZ(int32_t, uint32_t);
	void JLEZ(int32_t, uint32_t);
	void PUSH(uint32_t);
	uint32_t POP();
	void ADD(uint8_t reg0, uint8_t reg1, uint8_t reg2);
	void SUB(uint8_t reg0, uint8_t reg1, uint8_t reg2);
	void MUL(uint8_t reg0, uint8_t reg1, uint8_t reg2);
	void DIV(uint8_t reg0, uint8_t reg1, uint8_t reg2);
	void MOD(uint8_t reg0, uint8_t reg1, uint8_t reg2);
	void AND(uint8_t reg0, uint8_t reg1, uint8_t reg2);
	void OR(uint8_t reg0, uint8_t reg1, uint8_t reg2);
	void XOR(uint8_t reg0, uint8_t reg1, uint8_t reg2);
	void ASL(uint8_t reg0, uint8_t reg1, uint8_t reg2);
	void ASR(uint8_t reg0, uint8_t reg1, uint8_t reg2);
	void NOT(uint8_t reg0, uint8_t reg1);



	friend void test104();
	friend void test103();
	friend void test102();
};



#endif