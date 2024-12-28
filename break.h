#ifndef _H_BREAK
#define _H_BREAK

#include "types.h"
#include <vector>
#include <sys/ptrace.h>
#include <unistd.h>
#include <signal.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <map>

#define TRAP_INST   0xCC
#define TRAP_MASK   0xFFFFFFFFFFFFFF00

/*
 * class to (un)set and manage breakpoints
 */

class Break
{
public:
	Break(pid_t pid) :  pid_(pid) {};
	~Break();
	// set breakpoint at address
	int set(addr_t addr);
	// remove breakpoint restore original instructions and set rip to bp address
	int unset(addr_t addr);

private:
	std::map<addr_t, addr_t> bp_;
	pid_t pid_;
};
#endif
