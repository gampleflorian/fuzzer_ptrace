#include "fuzzer.h"
#include "state.h"
#include "target.h"

#include <algorithm>    // std::swap
#include <stdio.h>
#include <unistd.h>
#include <QxtLogger>

Fuzzer::Fuzzer(int argc, char** argv)
{
	tracer_ = new Tracer(argc, argv);
	// initial run
	qtxLog->info("starting initial trace");
	//while(tracer_->trace());
	while(1) {
		short r = tracer_->trace();
		usleep(10000);
		if(!r) {
			break;
		}
	}
	qtxLog->info("initial trace finished");
	T::arget().reset();
	T::arget().runTo(tracer_->last());
}

Fuzzer::~Fuzzer()
{
	delete tracer_;
}

void Fuzzer::fuzz(addr_t from, addr_t to, const std::vector<const VRange*>& vrange)
{
	if(from>to) std::swap(from, to);
	fprintf(stdout, "\nStarting Fuzz from %lx to %lx\n", from, to);
	// reset target
	T::arget().reset();
//	T::arget().runTo(tracer_->last());
//	T::arget().reset();
	T::arget().runTo(from);
	// save state
	struct user_regs_struct regs;
	T::arget().safe_ptrace(PTRACE_GETREGS, 0, &regs);
	State state(regs, Memstate(tracer_->heap_min(), tracer_->heap_max()), 
			Memstate(T::arget().sstart(), T::arget().sstart()+2048));
			//Memstate(tracer_->stack_min(), tracer_->stack_max()));
	//T::arget().runTo(to);

	// iterate over all values for all tags
	for( auto value : vrange ) {
		do {
			Val test(value->tag()->loc(),value->tag()->len());
			fprintf(stderr, "val: %s\n", test.str());
			value->set();
			fprintf(stderr, "Starting target with %lx: %s ... \n\n", 
					value->tag()->loc(), value->str());
			T::arget().runTo(to);
			fprintf(stderr, "\nDone\n");
			state.restore();
		}while (value->next());
	}
}
