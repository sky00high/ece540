#ifndef CFG_H
#define CFG_H
#include <stdio.h>
extern "C" {
	#include <simple.h>
}

#include <vector>
#include <map>
#include <iostream>
#include <set>

typedef _int2{
	int start;
	int end;
	simple_instr *startInstr;
	simple_instr *endInstr;
}edge;
using namespace std;
class BasicBlock {
	friend ostream &operator<<(ostream &, const BasicBlock &);

	

	public:
		int instrNum;
		int blockNum;
		int startNum;
		int endNum;
		set <int> dom;
		set <int> iDom; 
		map<int,int> preb;
		map<int,int> succ;
		BasicBlock();
		BasicBlock(int instrNum, int start, int end);
		
		void addPreb(int blockIndex);
		void addSucc(int blockIndex);
		int operator== (const BasicBlock &rhs) const;
		int operator< (const BasicBlock &rhs) const;
};
		
		
	
class CFG {
	map <int, BasicBlock> cfgMap;
	simple_instr *inlist;

	set <edge> rEdge;
	int totalInstrNum;
	int totalBlockNum;
	void addBlock(int,int,int);
	void iniBasicBlock(simple_instr *, map<int,int> &);
	int buildLeadVector(simple_instr *, map<int,int> &);
	void buildCFG(simple_instr *inlist);
	int findBasicBlockInstr(int instrIndex);
	void findDom();
	bool edgeExist(int start, int end);
	void findREdge();
	void insertPreHeader(<stack<simple_instr*>,int);
	simple_instr *findInstr(int);
	


	public:
		int getTotalBlockNum(){return totalBlockNum;}
		void printBlock();
		CFG(simple_instr *inlist);
		void findIDom();
		void printIDom();

		void CFG::genLoopHeader();


		
		

};

#endif




	


