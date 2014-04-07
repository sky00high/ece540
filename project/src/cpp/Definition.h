#ifndef DEFINITION_H
#define DEFINITION_H

#include "BitVector.h"
#include <assert.h>
#include <map>
#include <stdio.h>
#include "cfg.h"
extern "C"{
	#include <simple.h>
}

class Definition{
	public:
		int instrIndex;
		simple_instr *instr;
		bool operator==(const Definition &rhs) const;
		bool operator<(const Definition &rhs) const;

		Definition();
		Definition(simple_instr *instr, int instrIndex);
};

#endif
