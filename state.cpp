#include "state.h"
#include "target.h"
#include <stdlib.h>
#include <assert.h>

Memstate::Memstate(addr_t from, addr_t to) :
    from_(from), to_(to), data_(NULL)
{
    if(from_<to_) {
        data_ = new unsigned char[to_-from_];
        T::arget().readTarget(from_, (void*)data_, to_-from_);
    }
}
Memstate::Memstate(const Memstate& memstate) :
    from_(memstate.from_), to_(memstate.to_), data_(NULL)
{
    if(from_<to_) {
        data_ = new unsigned char[to_-from_];
        memcpy(data_, memstate.data_, to_-from_);
    }
}
Memstate::~Memstate()
{
    if(data_)
        delete data_;
}

void Memstate::restore() const
{
    if(data_) {
        T::arget().writeTarget(from_, (void*)&data_, to_-from_);
	//fprintf(stderr, "Restoring state from %lx to %lx\n", from_, to_);
    }
}

State::State(const struct user_regs_struct& regs, const Memstate& heapstate,
             const Memstate& stackstate) :
    regs_(regs), heapstate_(heapstate), stackstate_(stackstate)
{
}
State::~State()
{
}
void State::restore() const
{
    stackstate_.restore();
    struct user_regs_struct r = regs_;
    T::arget().safe_ptrace(PTRACE_SETREGS, 0, &r);
    T::arget().safe_ptrace(PTRACE_GETREGS, 0, &r);
    if(regs_.rip != r.rip)
    	fprintf(stderr, "New Rip %lx\n", r.rip);
    //heapstate_.restore();
}
bool State::operator==(const State& rhs) const
{
	char *ptrReg = (char*)&regs_;
	char *rhsPtrReg = (char*)&rhs.regs_;
	for(size_t i=0; i<sizeof(struct user_regs_struct); ++i)
		if(ptrReg[i]!=rhsPtrReg[i])
			return false;
	return stackstate_==rhs.stackstate_;
}
bool Memstate::operator==(const Memstate& rhs) const
{
	int len = from_ - to_;
	for(int i=0;i<len; ++i)
		if(data_[i]!=rhs.data_[i])
			return false;
	return true;
}
