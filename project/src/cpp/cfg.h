
#ifndef CFG_H
#define CFG_H
#include <stdio.h>
extern "C" {
	#include <simple.h>
}

#include <vector>
#include <stack>
#include <map>
#include <iostream>
#include <set>

typedef struct  _edge{
	int start;
	int end;
	simple_instr *startInstr; //the instrucion that after this it will goes to loop's beginging
	simple_instr *endInstr; //end of the edge , actually it is the start of the loop

	//use end block number to arrange the edge allow us to inspect loop with 
	//same staring position

	bool operator<(const struct _edge& rhs) const{
		if( end < rhs.end) return true;
		else if(end == rhs.end) return start < rhs.start;
		else return false;
	}

	bool operator==(const struct _edge& rhs) const{
		if(start == rhs.start &&
			end == rhs.end) return true;
		return false;
	}
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
	void insertPreHeader(stack<simple_instr*>);
	void insertOnStack(int,stack<int>&,set<int>&);
	set<int> findLoop(int,int);
	
	//for project
	int loopNum;
	map<int,set<int> > loopSet;
	bool loopGenerated;
	


	public:
		simple_instr *findInstr(int);
		void findREdge();
		int getTotalBlockNum(){return totalBlockNum;}
		void printBlock();
		CFG(simple_instr *inlist);
		void findIDom();
		void printIDom();

		void genLoopHeader();
		void printLoop();

		int getTotalLoopNum();
		//for a3
		simple_instr* getBlockStartInstr(int);
		simple_instr* getBlockEndInstr(int);
		
		map<int,int> getBlockSucc(int BlockIndex);
		//for a4
		map<int,int> getBlockPred(int BlockIndex);

		//for project
		int getInstrNum(simple_instr *instr);
		void genLoopSet();
		map<int,set<int> > getLoopSet() ;
		int getLoopNum() const{return loopNum;}
		int findIndexInstr(simple_instr *instr);
		simple_instr *findInstrIndex(int index);
		int findBBInstr(simple_instr *instr);
		

};

#endif




	


