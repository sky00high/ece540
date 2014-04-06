#include <stdio.h>
extern "C" { 
#include <simple.h>
}

#include  "cfg.h"
#include "DFG.h"
// data structures you should consider using are vector and hash_map from the STL
// refer to the following link as a starting point if you are not familiar with them: 
// http://www.sgi.com/tech/stl/Vector.html
// http://www.sgi.com/tech/stl/hash_map.html 

simple_instr* do_procedure (simple_instr *inlist, char *proc_name)
{
    // build flow control graph 
	 CFG *thisCFG = new CFG(inlist);
	 thisCFG->findIDom();
	// thisCFG->genLoopHeader();
	 //delete thisCFG;
	// thisCFG = new CFG(inlist);
//	 thisCFG->findIDom();
//	 thisCFG->findREdge();

	 DFG *thisDFG = new DFG(thisCFG,inlist);
	 thisDFG->generateVarList();
	 cout<<"variables "<<proc_name<<" "<<thisDFG->getVarNum()<<endl;
	 thisDFG->printVarList();
	 cout<<endl;
	 thisDFG->genDefSet();
	 cout<<"def_lv_sets "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisDFG->printDefSet();
	 cout<<endl;
	 thisDFG->genUseSet();
	 cout<<"use_lv_sets "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisDFG->printUseSet();
	 cout<<endl;
	 thisDFG->genLiveOut();
	 cout<<"live_variables "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisDFG->printLiveOut();
	 cout<<endl;
	 delete thisCFG;
	 delete thisDFG;


    // find immediate dominators    

    return inlist;
}
