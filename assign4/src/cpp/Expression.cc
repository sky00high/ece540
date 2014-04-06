#include <assert.h>
#include <map>
#include <stdio.h>
#include "cfg.h"
#include "Expression.h"
extern "C"{
	#include <simple.h>
}


bool Expression::operator==(const Expression &rhs) const{
	if(this->opcode == rhs.opcode){
		if(this->op1 == rhs.op1 && this->op2 == rhs.op2){
			return true;
		}else if(this->op1 == rhs.op2 && this->op2 == rhs.op1){
			return true;
		}
	}
	return false;
}

bool Expression::operator<(const Expression &rhs) const{
	
	//I think I just need to find a consistent way to distinguish if
	//one expression is smaller than another one. The way below can 
	//mask the effect that differnt operand in switched location
	if(this->opcode != rhs.opcode){
		 return this->opcode < rhs.opcode;
	}
	//int lhsSmall = this->op1 > this->op2? this->op2: this->op1;
	//int lhsBig = this->op1 > this->op2? this->op1: this->op2;
	//int rhsSmall = rhs.op1>rhs.op2?rhs.op2:rhs.op1;
	//int rhsBig = rhs.op1>rhs.op2?rhs.op1:rhs.op2;

	if(this->op1 != rhs.op1) return this->op1 < rhs.op1;
	else return this->op2 < rhs.op2;
}
Expression::Expression(){
}
Expression::Expression(simple_instr *instr){
	if(instr->opcode != CVT_OP){
		this->op1 = instr->u.base.src1->num;
		this->op2 = instr->u.base.src2->num;
		this->opcode = instr->opcode;
		this->instr = NULL;
		this->instrIndex = -1;
	}else{
		this->op1 = instr->u.base.src1->num;
		this->op2 = 777;
		this->opcode = instr->opcode;
		this->instr = NULL;
		this->instrIndex = -1;
	}

}
Expression::Expression(simple_instr *instr, bool if_invert){
	if(if_invert){
		this->op2 = instr->u.base.src1->num;
		this->op1 = instr->u.base.src2->num;
	}else{
		this->op1 = instr->u.base.src1->num;
		this->op2 = instr->u.base.src2->num;
	}
	this->opcode = instr->opcode;
	this->instr = NULL;
	this->instrIndex = -1;
}


