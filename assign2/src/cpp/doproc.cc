#include <stdio.h>
extern "C" { 
#include <simple.h>
}

#include  "cfg.h"
// data structures you should consider using are vector and hash_map from the STL
// refer to the following link as a starting point if you are not familiar with them: 
// http://www.sgi.com/tech/stl/Vector.html
// http://www.sgi.com/tech/stl/hash_map.html 

simple_instr* do_procedure (simple_instr *inlist, char *proc_name)
{
    // build flow control graph 
	 CFG *thisCFG = new CFG(inlist);
	 thisCFG->findIDom();
	 thisCFG->genLoopHeader();

	 delete thisCFG;
	 thisCFG = new CFG(inlist);
	 thisCFG->findIDom();
	 cout<<"loopcfg "<<proc_name<<" "<<thisCFG->getTotalBlockNum()<<endl;
	 thisCFG->printBlock();
	 thisCFG->findREdge();
	 cout<<"natural_loops "<<proc_name<<" "<<thisCFG->getTotalLoopNum()<<endl;
	 thisCFG->printLoop();
	 delete thisCFG;


    // find immediate dominators    

    return inlist;
}
