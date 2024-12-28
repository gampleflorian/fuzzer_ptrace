#include "tracer.h"
#include "target.h"
#include "distorm.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/mman.h>
#include <math.h>
#include <limits>
#include <QxtLogger>
#include <QString>

Tracer::Tracer(int argc, char** argv) :
	heap_min_(std::numeric_limits<addr_t>::max()),
	heap_max_(0),
	stack_min_(std::numeric_limits<addr_t>::max()),
	stack_max_(0)
{
	// prep target
	T::arget().init(argv);
	// find argv
	struct user_regs_struct regs;
	T::arget().safe_ptrace(PTRACE_GETREGS, 0, &regs);
	//Tag* tag_argv_base = new Tag(regs.rax, sizeof(addr_t), TT_PTR);
	//addTag(tag_argv_base);
	addr_t main = T::arget().elf()->get_func((char*)"main");
	for(int i=0; i<argc; ++i) {
		addr_t a = regs.rsi + i*sizeof(addr_t);
		addr_t aDst = 0;
		int len = strlen(argv[i])+1;
		Tag* src = new Tag(a, sizeof(addr_t), main, TT_PTR);
		T::arget().readTarget(a, (void*)&aDst, sizeof(addr_t));
		Tag* dst = new Tag(aDst, len, main, TT_STR);
		char tmp[16];
		T::arget().readTarget(aDst, (void*)&tmp, sizeof(addr_t));

		//tag_argv_base->addTraceF(0, src, PTR);
		//src->addTraceB(0, tag_argv_base, PTR);
		src->addTraceF(0, dst, PTR);
		dst->addTraceB(0, src, PTR);

		addTag(src);
		addTag(dst);
	}
	// protect stack so we get a segfault on access
	T::arget().protect_stack(PROT_NONE);

	qxtLog->info("Tracer created");
}

Tracer::~Tracer()
{
	std::set<Tag*>::iterator it =tags_.begin();
	for( ; it!=tags_.end(); ++it)
		delete *it;
	// dont double delete tags ...
	std::vector<Branch*>::iterator jt = branchs_.begin();
	for( ; jt!=branchs_.end(); ++jt)
		delete *jt;
}

int Tracer::trace()
{

	// continue target
	T::arget().safe_ptrace(PTRACE_CONT, 0, NULL);
	int status;
	waitpid(T::arget().pid(), &status, 0);
	if(WIFEXITED(status))
	{
		fprintf(stderr, "child exit status %d\n", 
				WEXITSTATUS(status));
		return 0;
	}

	// assume WIFSTOPPED
	//
	//
	struct user_regs_struct regs;
	T::arget().safe_ptrace(PTRACE_GETREGS, 0, &regs);
	siginfo_t si;
	T::arget().safe_ptrace(PTRACE_GETSIGINFO, 0 , &si); 

	qxtLog->debug("Tracing rip: ", QString::number(regs.rip, 16));
	if(regs.rip==0){
		fprintf(stderr, "DONE");
		T::arget().protect_stack(PROT_READ|PROT_WRITE);
		//T::arget().safe_ptrace(PTRACE_CONT, 0, NULL);
		return 0; // we are done
	}

	// un protect stack
	T::arget().protect_stack(PROT_READ|PROT_WRITE);

	// if the next instruction is not in our area we break on return
	if(!T::arget().inCode(regs.rip)) {

		qxtLog->debug("Is not in Code section");

		addr_t returnadr = T::arget().safe_ptrace(PTRACE_PEEKTEXT, 
				regs.rsp, NULL);

		qxtLog->debug("return address: ", QString::number(returnadr, 16));

		if(!T::arget().inCode(returnadr)) {
		//if(returnadr==0)  {
			qxtLog->critical("Return address is not in code");
			//fprintf(stderr, "DONE");
			//T::arget().protect_stack(PROT_READ|PROT_WRITE);
			return 0;
		}
		T::arget().bp()->set(returnadr);
		T::arget().safe_ptrace(PTRACE_CONT, 0, NULL);

		int status;
		waitpid(T::arget().pid(), &status, 0);
		T::arget().bp()->unset(returnadr);

		qxtLog->debug("Target returned to code section");

		T::arget().protect_stack(PROT_NONE);
		return 1;
	}


	last_ = regs.rip;
	switch(si.si_signo) {
		// breakpoint
		case SIGTRAP:
			qxtLog->debug("Got SIGTRAP");
			handle_cjmp();
			break;
		// mem access
		case SIGSEGV:
			qxtLog->debug("Got SIGSEGV");
			handle_segv((addr_t)si.si_addr, regs.rip);
			break;
		default:
			qxtLog->critical("Got unknown Signal: ", si.si_signo);
			//fprintf(stderr, "Unknown Signal %d\n", si.si_signo);
			return 0; // done
	}

	// reexecute instruction and reprotect stack
	T::arget().singlestep();
	T::arget().protect_stack(PROT_NONE);
	return 1;
}

int Tracer::handle_cjmp()
{
	// unset breakpoint
	struct user_regs_struct regs;
	T::arget().safe_ptrace(PTRACE_GETREGS, 0, &regs);
	addr_t rip = regs.rip-0x1; // -0x1 because 'int $0x3' has already been executed
	T::arget().bp()->unset(rip);

	// create branch instance
	const _DInst* ijmp = T::arget().getI(rip);
	addr_t to = 0;
	if (ijmp->flags & FLAG_RIP_RELATIVE)
		to = INSTRUCTION_GET_RIP_TARGET(ijmp);
	else
		to = INSTRUCTION_GET_TARGET(ijmp);
	Branch* b = new Branch(rip, to);
	if(!addBranch(b))
		return 0;

	// iterate over last accessed tags
	std::vector<Tag*>::reverse_iterator tag = last_tag_.rbegin();
	for(; tag!=last_tag_.rend(); ++tag) {
		const Access* ta = (*tag)->lastAccess();
		if(ta == NULL) continue;
		_OffsetType lastRip = ta->rip();
		for(_OffsetType off = 0; off+lastRip < rip; ) {
			// decode instruction
			const _DInst* inst = T::arget().getI(lastRip+off);
			// compared accessed flags
			if(inst->modifiedFlagsMask & ijmp->testedFlagsMask) {
				if(inst->ops[0].type == O_IMM || inst->ops[1].type == O_IMM) {
					regs.rip = rip;
					T::arget().safe_ptrace(PTRACE_SETREGS, 0, &regs);
					return 1;
				}
				size_t len = fmin((*tag)->lastAccess()->len(),inst->ops[0].size);
				Val* val = new Val((unsigned char*)&inst->imm.sqword, len);
				Cond* c = new Cond((*tag), val);
				// TODO
				// until ??
				// add condition to branch
				b->addCond(c);
			}
			off += inst->size;
		}
	}
	getchar();
	return 0;
}

int Tracer::handle_segv(addr_t loc, addr_t rip)
{
	std::multimap<addr_t, Tag*>::iterator tag = mem_tags_.find(loc);
	// handle tagged memory
	if(tag!=mem_tags_.end()) {
		tag->second->log(rip, loc);
		last_tag_.push_back(tag->second);
	} else { // handle untagged memory
		try {
			Access* a = new Access(rip, loc);
			Tag* t = new Tag(a);
			std::vector<Tag*>::reverse_iterator ltag;
			switch(a->atype())
			{
				case READ:
					addTag(t);
					break;
				case WRITE:
					if(addTag(t)) {
						for(ltag = last_tag_.rbegin(); ltag!=last_tag_.rend(); ++ltag) {
							if(t==*ltag) continue;
							const Access* ta = (*ltag)->lastAccess();
							if(ta == NULL) continue;
							if(ta->atype()==READ && // read<->write
									a->reg() == ta->reg() &&
									a->val()->cmp(*ta->val())) {
								t->type((*ltag)->type());
								t->addTraceB(rip, *ltag, CPY);
								(*ltag)->addTraceF(rip, t, CPY);
							}
						}
					}
					break;
				default:
					delete t;
			}
		} catch (...) {
			return 0;
		}
	}
	// add new tag
	return 0;
}

int Tracer::addTag(Tag* t)
{
	std::pair<std::set<Tag*>::iterator, bool> ret = tags_.insert(t);
	if(!ret.second) { // tag is already present
		delete t;
		return 0;
	}
	stack_min_ = fmin(stack_min_, t->loc());
	stack_max_ = fmax(stack_max_, t->loc() + t->len());
	for(int i=0; i<t->len(); ++i)
		mem_tags_.insert(std::pair<addr_t, Tag*>(t->loc()+i, t));
	if(t->lastAccess()!=NULL)
		last_tag_.push_back(t);
	return 1;
}

int Tracer::addBranch(Branch* b)
{
	std::pair<std::map<addr_t, Branch*>::iterator, bool> ret = 
		rip_branch_.insert(std::pair<addr_t, Branch*>(b->from(),b));
	if(!ret.second) { // tag is already present
		delete b;
		return 0;
	}
	branchs_.push_back(b);
	return 1;
}
