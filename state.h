#ifndef STATE_H
#define STATE_H

#include "types.h"
#include <sys/user.h>     /* struct user */

class Memstate
{
	public:
		Memstate(addr_t from, addr_t to);
		Memstate(const Memstate& memstate);
		~Memstate();
		void restore() const;
		bool operator==(const Memstate& rhs) const;
	private:
		addr_t from_;
		addr_t to_;
		unsigned char* data_;
};

class State
{
	public:
		State(const struct user_regs_struct& regs, const Memstate& heapstate,
				const Memstate& stackstate);
		~State();
		void restore() const;
		bool operator==(const State& rhs) const;
	private:
		struct user_regs_struct regs_;
		const Memstate heapstate_;
		const Memstate stackstate_;

};

#endif
