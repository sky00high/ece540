#include <stdio.h>
extern "C" { 
#include <simple.h>
}

#include  "cfg.h"
#include "DFG.h"
#include "ae.h"
// data structures you should consider using are vector and hash_map from the STL
// refer to the following link as a starting point if you are not familiar with them: 
// http://www.sgi.com/tech/stl/Vector.html
// http://www.sgi.com/tech/stl/hash_map.html 

simple_instr* do_procedure (simple_instr *inlist, char *proc_name)
{
    // build flow control graph 
	 CFG *thisCFG = new CFG(inlist);
	 thisCFG->findIDom();
	 //thisCFG->printBlock();
	// thisCFG->genLoopHeader();
	 //delete thisCFG;
	// thisCFG = new CFG(inlist);
//	 thisCFG->findIDom();
//	 thisCFG->findREdge();

	 AE *thisAE = new AE(thisCFG,inlist);
	 thisAE->genExprList();
	 cout<<"expressions "<<proc_name<<" "<<thisAE->getExprNum()<<endl;
	 thisAE->printExprList();
	 cout<<endl;

	 thisAE->genEvalSet();
	 cout<<"eval_ae_sets "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisAE->printEvalSet();
	 cout<<endl;

	 thisAE->genKillSet();
	 cout<<"kill_ae_sets "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisAE->printKillSet();
	 cout<<endl;

	 thisAE->genAeOutSet();
	 cout<<"available_exprs "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisAE->printAeOutSet();
	 cout<<endl;
	 

	 /*
	 cout<<endl;
	 thisAE->genUseSet();
	 cout<<"use_lv_sets "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisAE->printUseSet();
	 cout<<endl;
	 thisAE->genLiveOut();
	 cout<<"live_variables "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisAE->printLiveOut();
	 cout<<endl;
	 */
	 delete thisCFG;
	 delete thisAE;


    // find immediate dominators    

    return inlist;
}
