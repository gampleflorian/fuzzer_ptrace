#include "cond.h"

Cond::Cond(Tag* t, Val* val) :
	tag_(t), val_(val)
{
}

Cond::~Cond()
{
	delete val_;
}
