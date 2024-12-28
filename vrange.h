#ifndef VRAGNE_H
#define VRAGNE_H

#include "tag.h"
#include "val.h"
#include "inc.h"

class VRange
{
	public:
		VRange(Tag* t, Val *from_val, Val *to_val);
		~VRange();

		bool next() const;
		void set() const;

		const Tag* tag() const {return tag_;}
		void inc(Inc* inc);
		const Inc* inc() {return inc_;}
		const Val* from() const {return from_;}
		const Val* to() const {return to_;}
		void from(Val* from);
		void to(Val* from);
		const char* str() const {return current_->str();}
		void resetFrom() const;

	private:
		Tag* tag_;
		mutable Val* from_;
		Val* orig_from_;
		Val* to_;
		Inc* inc_;
		int len_;
		mutable Val* current_;
};

#endif
