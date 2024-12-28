#ifndef COND_H
#define COND_H

#include "val.h"
#include "tag.h"

class Cond
{
	public:
		Cond(Tag* t, Val* val);
		~Cond();
	
	private:
		Tag* tag_;
		Val* val_;
};

#endif
