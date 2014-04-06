
#ifndef EXPRESSION_H
#define EXPRESSION_H 
#include "BitVector.h"
#include <assert.h>
#include <map>
#include <stdio.h>
#include "cfg.h"
extern "C"{
	#include <simple.h>
}

class Expression{
	public:
	int op1;
	int op2;
	simple_op opcode;
	simple_instr *instr;
	int instrIndex;
	bool operator==(const Expression &rhs) const;
	bool operator<(const Expression &rhs) const;
	Expression(); 
	Expression(simple_instr *instr);
	Expression(simple_instr *instr,bool);
};
#endif
