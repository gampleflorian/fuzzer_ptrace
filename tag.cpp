#include "tag.h"
#include "target.h"
#include <stdio.h>
#include <QxtLogger>
#include <QString>

Tag::Tag(addr_t loc, int len, addr_t rip, TAGTYPE type) :
	loc_(loc), len_(len), type_(type)
{
	init_val_ = new Val(loc_, len_);
	Access* a = new Access(rip, loc_, len_);
	addAccess(a);
	if(type == TT_UNKNOWN)
		type_ = guessType();
	//fprintf(stderr, "Created Tag at %lx (+%d) type %d\n", loc_, len_, type_);
	qxtLog->info("Created Tag at ", QString::number(loc_, 16), " (+", len_, ") type ", type_);
}

Tag::Tag(Access* a, TAGTYPE type) : type_(type),
	init_val_(a->val())
{
	addAccess(a);
	loc_ = a->loc();
	len_ = a->len();
	if(type == TT_UNKNOWN)
		type_ = guessType();
	//fprintf(stderr, "Created Tag at %lx (+%d) type %d\n", loc_, len_, type_);
	qxtLog->info("Created Tag at ", QString::number(loc_, 16), " (+", len_, ") type ", type_);
}

Tag::~Tag()
{
	delete init_val_;
	std::vector<Access*>::iterator it = access_.begin();
	for( ; it!=access_.end(); ++it)
		delete *it;

	std::map<addr_t, Trace*>::iterator jt = tforw_.begin();
	for( ; jt!=tforw_.end(); ++jt)
		delete jt->second;
	jt = tbackw_.begin();
	for( ; jt!=tbackw_.end(); ++jt)
		delete jt->second;
	// DO NOT double delete the access ptrs
}

const Access* Tag::lastAccess() const
{
	if(access_.empty())
		return NULL;
	return access_.back();
}
addr_t Tag::loc() const
{
	return loc_;
}
int Tag::len() const
{
	return len_;
}

int Tag::log(addr_t rip, addr_t loc)
{
	//const Access* last = lastAccess();
	Access* a = new Access(rip, loc);
	addAccess(a);
	return 1;
}

int Tag::addAccess(Access* a)
{
	access_.push_back(a);
	rip_access_.insert(std::pair<addr_t, Access*>(
				a->rip(), a));
	return 0;

}

int Tag::addTraceF(addr_t rip, Tag* t, TTYPE type)
{
	Trace* tr = new Trace(t, type);
	std::pair<std::map<addr_t, Trace*>::iterator, bool> ret =
		tforw_.insert(std::pair<addr_t, Trace*>(
					rip, tr));
	if(!ret.second) {
		delete tr;
		return 0;
	}
	return 1;
}
int Tag::addTraceB(addr_t rip, Tag* t, TTYPE type)
{
	Trace* tr = new Trace(t, type);
	std::pair<std::map<addr_t, Trace*>::iterator, bool> ret =
		tbackw_.insert(std::pair<addr_t, Trace*>(
					rip, tr));
	if(!ret.second) {
		delete tr;
		return 0;
	}
	return 1;
}

TAGTYPE Tag::guessType() // TODO: make better ...
{
	int n = sizeof(addr_t);
	const unsigned char* val = init_val_->val();
	if(len_ == n) {
		if(T::arget().inStack(*((addr_t*)val))) { // TODO: change criteria
			return TT_PTR;
		} 
	}else {
		for(int i=0; i<n; ++i) {
			if(val[i] < 32 && val[i] != 0) {// non displayable character
				return TT_NUM;
			}
		}
		return TT_STR;
	}
	return TT_UNKNOWN;
}


void Tag::loc(addr_t newLoc)
{
	fprintf(stderr, "Changed location %lx -> %lx\n",
			loc_, newLoc);
	loc_ = newLoc;
	for(auto back : tbackw_) {
		if(back.second->ttype==PTR) {
			T::arget().writeTarget(back.second->tag->loc(), &newLoc, sizeof(addr_t));
			fprintf(stderr, "Updated Tag at %lx\n", back.second->tag->loc());
		}
	}
	//}
}
void Tag::len(int newLen)
{
	len_ = newLen;
	loc(T::arget().findSpace(len_));
}

const Val* Tag::val(addr_t rip) const 
{
	auto it = rip_access_.lower_bound(rip);
	if(it!=rip_access_.end())
		return it->second->val();
	return init_val_;
}

const Trace* Tag::tforw(addr_t rip) const
{
	auto it = tforw_.lower_bound(rip);
	if(it!=tforw_.end())
		return it->second;
	return NULL;
}
const Trace* Tag::tbackw(addr_t rip) const
{
	auto it = tbackw_.lower_bound(rip);
	if(it!=tbackw_.end())
		return it->second;
	return NULL;
}
