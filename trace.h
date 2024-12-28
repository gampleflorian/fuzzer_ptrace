#ifndef TRACE_H
#define TRACE_H

#include "types.h"

class Tag;

enum TTYPE{CPY, PTR};

struct Trace
{
	Trace(Tag* t, TTYPE tt) : tag(t), ttype(tt) {}
	Tag* tag;
	TTYPE ttype;
};

#endif
