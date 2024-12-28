#include "val.h"
#include "target.h"
#include <assert.h>

Val::Val(addr_t loc, int len) :
	len_(len)
{
	val_ = new unsigned char[len_];
	T::arget().readTarget(loc, val_, len_);
	mkStr();
}
Val::Val(unsigned char* val, int len) :
	len_(len)
{
	val_ = new unsigned char[len_];
	for(int i=0; i<len_; ++i)
		val_[i] = val[i];
	mkStr();
}
Val::Val(const Val& val) :
	len_(val.len_)
{
	val_ = new unsigned char[len_];
	for(int i=0; i<len_; ++i)
		val_[i] = val.val_[i];
	mkStr();
}
Val::~Val()
{
	delete[] val_;
	delete[] str_;
}

int Val::mkStr()
{
	//if(str_) delete[] str_;
	str_ = new char[len_*4+1];
	int i;
	for(i=0; i<len_; i++) {
		snprintf(&str_[i*3], 4, "%2X ", val_[i]);
	}
	str_[i*3] = 0;
	return i;
}

bool Val::cmp(const Val &val) const
{
	for(int i=0; i<len_ && i<val.len(); ++i) {
		if(val_[i]!=val.val()[i])
			return false;
	}
	return true;
}

void Val::len(int newLen)
{
	if(newLen>len_) {
		unsigned char* newVal = new unsigned char[newLen];
		for(int i=0; i<len_; ++i)
			newVal[i] = val_[i];
		for(int i=len_; i<newLen; ++i)
			newVal[i] = 0;
		delete val_;
		val_=newVal;
		len_ = newLen;
	}
}
void Val::val(unsigned char* newVal, int newLen)
{
	len(newLen);
	for(int i=0; i<newLen; ++i) {
		val_[i] = newVal[i];
	}
	mkStr();
}
