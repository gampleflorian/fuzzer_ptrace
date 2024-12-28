#include "branch.h"

Branch::Branch(addr_t from, addr_t to) :
	from_(from), to_(to)
{
}
Branch::~Branch()
{
	std::set<Cond*>::iterator it = conds_.begin();
	for( ; it!=conds_.end(); ++it)
		delete *it;
}

int Branch::addCond(Cond* c)
{
	std::pair<std::set<Cond*>::iterator, bool> ret = conds_.insert(c);
	if(!ret.second) {
		delete c;
		return 0;
	}
	return 1;
}
