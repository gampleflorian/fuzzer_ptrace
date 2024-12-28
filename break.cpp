#include "break.h"
#include <stdio.h>
#include <QxtLogger>
#include <QString>

Break::~Break()
{
	bp_.clear();
}

int Break::set(addr_t addr) {
	// save original instruction
	addr_t orig = ptrace(PTRACE_PEEKTEXT, pid_, addr);
	// insert trap
	ptrace(PTRACE_POKETEXT, pid_, addr, (orig & TRAP_MASK) | TRAP_INST);
	// add breakpoint to list
	bp_.insert(std::pair<addr_t, addr_t>(addr, orig));
	//printf("Set bp at %lx\n", addr);
	qxtLog->info("breakpoint set at ", QString::number(addr, 16));
	return bp_.size();
}

int Break::unset(addr_t addr) {
	std::map<addr_t, addr_t>::iterator i = bp_.find(addr);
	if(i==bp_.end()) {
		fprintf(stderr, "WARNING: no bp at %lx\n", addr);
		return -1;
	}
	// set rip back to bp address 
	struct user_regs_struct regs;
	ptrace(PTRACE_GETREGS, pid_, NULL, &regs);
	//if(addr != regs.rip+1) {
	//	fprintf(stderr, "WARNING: unsetting bp at %lx from %lx\n",
	//			addr, regs.rip);
	//	return 0;
	//}
	//printf("Usetting bp at %lx\n", addr);
	regs.rip = addr;
	ptrace(PTRACE_SETREGS, pid_, NULL, &regs);
	// rewrite original instructions
	ptrace(PTRACE_POKETEXT, pid_, i->first, i->second);
	// remove breakpoint from list
	bp_.erase(i);
	qxtLog->info("breakpoint unset at ", QString::number(addr, 16));
	return bp_.size();
}


