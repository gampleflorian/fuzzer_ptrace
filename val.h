#ifndef VAL_H
#define VAL_H

#include "types.h"

class Val
{
	public:
		Val(addr_t loc, int len);
		Val(unsigned char* val, int len);
		Val(const Val& v);
		~Val();

		int len() const {return len_;}
		void len(int newLen);
		const unsigned char* val() const {return val_;}
		unsigned char* val() {return val_;}
		void val(unsigned char* newVal, int newLen);
		bool isTerminated() const { return val_[len_-1]==0; }
		void terminate() {
			len(len_+1);
			val_[len_-1] = 0;
		}

		bool cmp(const Val& val) const;
		const char* str() const {return str_;}
		int mkStr();

	private:
		unsigned char* val_;
		int len_;
		char* str_;
};


#endif
