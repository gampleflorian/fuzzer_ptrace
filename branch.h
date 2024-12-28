#ifndef BRANCH_H
#define BRANCH_H

#include "types.h"
#include "cond.h"
#include <set>

class Branch
{
	public:
		Branch(addr_t from, addr_t to);
		~Branch();

		addr_t from() const {return from_;}
		addr_t to() const {return to_;}

		int addCond(Cond* c);

	private:
		addr_t from_;
		addr_t to_;
		std::set<Cond*> conds_;
};

#endif
