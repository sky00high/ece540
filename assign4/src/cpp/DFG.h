#ifndef DFG_H
#define DFG_H
#include "BitVector.h"
#include <map>
#include <stdio.h>
#include "cfg.h"
extern "C"{
	#include <simple.h>
}
using namespace std;
class DFG{
	//this cfg for this procedure
	CFG *cfg;
	int varNum;
	map<int,int> varMap;
	simple_instr *inlist;
	

	//part1 helpers
	simple_instr *findInstr(int index);
	void checkReg(simple_reg *);
	void findVar();
	int findVarIndex(int indext);

	//part2
	map<int,BitVector>defSet;
	void genDefBlock(int);
	void iniDefSet();


	//part3
	map<int,BitVector>useSet; 
	void checkRegUse(simple_instr*, simple_instr*,BitVector &useVector,simple_reg *lereg);
	void genUseBlock(int);
	void iniUseSet();

	//part4
	map<int,BitVector> liveIn;
	map<int,BitVector> liveOut;
	void iniLiveIn();
	void iniLiveOut();


	public:
		DFG(CFG *cfg, simple_instr *inlist);
		//part1
		void generateVarList();
		void printVarList();

		//part2
		void genDefSet();
		void printDefSet();

		//part3
		void genUseSet();
		void printUseSet();

		//prt4
		void genLiveOut();
		void printLiveOut();
		inline int getVarNum(){
			return this->varNum;
		}
		
	

};



#endif 
