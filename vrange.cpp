#include "vrange.h"
#include "target.h"
#include <assert.h>
#include <math.h>

VRange::VRange(Tag* tag, Val* from_val, Val* to_val) :
	tag_(tag), from_(from_val), to_(to_val), current_(from_val)
{
	// if the variables are too long, we have to find new space
	len_ = fmax(from_->len(), to_->len());
	from_->len(len_);
	to_->len(len_);
	if(tag_->len() < len_)
		tag_->len(len_);
	orig_from_ = new Val(from_->val(), from_->len());
	inc_ = new IncSkip(to_->val(), to_->len());
	//unsigned char step = 1;
	//inc_ = new IncNum(&step, 1);
}
VRange::~VRange()
{
	delete inc_;
}
bool VRange::next() const
{
	if(current_->cmp(*to_)) return false;
	addr_t newLoc = inc_->inc(current_);
	if(newLoc == 1) return false;
	if(newLoc != 0) {
		// TODO update back references;
		tag_->loc(newLoc);
	}
	return true;
}
void VRange::set() const
{
	T::arget().writeTarget(tag_->loc(), (void*)current_->val(), tag_->len());
}
void VRange::from(Val* newVal)
{
	len_ = fmax(newVal->len(), len_);
	if(tag_->len() < len_)
		tag_->len(len_);
	from_->len(len_);
	to_->len(len_);
	from_->val(newVal->val(), newVal->len());

	if(inc_->type() == ILEN) {
		if(!from_->isTerminated()) {
			from_->terminate();
			to_->len(from_->len());
		}
		if(!to_->isTerminated()) {
			to_->terminate();
			from_->len(to_->len());
		}
	}

}
void VRange::to(Val* newVal)
{
	len_ = fmax(newVal->len(), len_);
	if(tag_->len() < len_)
		tag_->len(len_);
	from_->len(len_);
	to_->len(len_);
	to_->val(newVal->val(), newVal->len());
	if(inc_->type()==SKIP)
		inc(new IncSkip(
					to_->val(),
					to_->len()
			       ));

	if(inc_->type() == ILEN) {
		if(!from_->isTerminated()) {
			from_->terminate();
			to_->len(from_->len());
		}
		if(!to_->isTerminated()) {
			to_->terminate();
			from_->len(to_->len());
		}
	}
}

void VRange::inc(Inc* inc)
{
	if(inc_) delete inc_;
	if(inc_->type() == ILEN) {
		if(!from_->isTerminated()) {
			from_->terminate();
			to_->len(from_->len());
		}
		if(!to_->isTerminated()) {
			to_->terminate();
			from_->len(to_->len());
		}
	}
	inc_ = inc;
}
void VRange::resetFrom() const
{
	delete from_;
	from_ = orig_from_;
}
