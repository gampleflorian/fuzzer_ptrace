#ifndef ACCESS_H
#define ACCESS_H

#include "types.h"
#include "val.h"
#include <exception>

enum ACCESSTYPE{READ, WRITE, IDK};

class InvalidMem : public std::exception
{
	public:
		virtual const char* what() const throw()
		{
			return "Invalid Memory";
		}
};

class Access
{
	public:
		Access(addr_t rip, addr_t loc, int len=0);
		~Access();

		addr_t rip() const {return rip_;}
		addr_t loc() const {return loc_;}
		int len() const {return len_;}
		const Val* val() const {return val_;}
		ACCESSTYPE atype() const;
		int reg() const {return reg_;}

	private:
		addr_t rip_;
		addr_t loc_;
		int len_;
		Val* val_;
		ACCESSTYPE atype_;
		int reg_;
};

#endif
