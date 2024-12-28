#ifndef _H_ELF
#define _H_ELF

#include "types.h"

#include <string>
#include <map>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <libelf.h>
#include <gelf.h>

#define ERR -1

struct classcomp {
	  bool operator() (const char* lhs, const char* rhs) const
	    {return strcmp(lhs,rhs)<0;}
};

class Elf {
	public:
		Elf(char* fname);
		~Elf();

		int init();

		addr_t get_func(char* name) const;
		const char* get_func(addr_t ip) const;

		void print_func();
		addr_t code_min() const { return code_min_;};
		addr_t code_max() const { return code_max_;};
		
	private:
		std::map<char*, addr_t, classcomp> func_;
		std::map<addr_t, char*, std::greater<addr_t>> funcA_;
		addr_t code_min_;
		addr_t code_max_;

};
#endif
