#include "inc.h"
#include <stdio.h>
#include <string.h>
#include <limits>
#include <assert.h>
#include <math.h>

Inc::Inc(const unsigned char* incStep, int len, ITYPE itype) :
    len_(len), itype_(itype)
{
    incStep_ = new unsigned char[len_];
    for(int i=0; i<len_ ; ++i)
        incStep_[i] = incStep[i];
    maxChar_ = std::numeric_limits<unsigned char>::max();
}
Inc::~Inc()
{
    delete[] incStep_;
}
IncNum::IncNum(const unsigned char* incStep, int len) :
    Inc(incStep, len, IVAL)
{
}
IncNum::~IncNum()
{
}
addr_t IncNum::inc(Val* val, unsigned char maxByte)
{
    int i;
    addr_t newLoc = 0;
    int vLen = val->len();
    if(len_>=vLen) {
        val->len(len_+1);
        vLen = len_ + 1;
    }
    unsigned char overflow = 0;
    for(i=0; i<len_ ; ++i) {
        val->val()[i] += incStep_[i] + overflow;
        overflow = fmax(0,val->val()[i] + incStep_[i] - maxChar_);
    }
    val->val()[i] += overflow;
    val->mkStr();
    return newLoc;
}

IncStrLen::IncStrLen(const unsigned char* incStep, int len) :
    Inc(incStep, len, ILEN), currentByte_(len)
{
    assert(len==1);
}
IncStrLen::~IncStrLen()
{
}
addr_t IncStrLen::inc(Val* val, unsigned char maxByte)
{
    addr_t newLoc = 0;
    int vLen = val->len()+1;
    if(vLen < maxByte)
    {
        val->len(maxByte);
        vLen = maxByte;
    }
    int currentByte = strlen((char*)val->val());
    val->val()[currentByte] = 'a';
    val->val()[currentByte+1] = 0;
    val->mkStr();
    if(currentByte >= vLen-2) return 1;
    return newLoc;
}

IncSkip::IncSkip(const unsigned char* incStep, int len) :
    Inc(incStep, len, SKIP)
{
}
IncSkip::~IncSkip()
{
}
addr_t IncSkip::inc(Val* val, unsigned char maxByte)
{
    addr_t newLoc = 0;
    int vLen = val->len();
    if(vLen < maxByte)
    {
        val->len(maxByte);
        vLen = maxByte;
    }
    for(int i=0; i<vLen; ++i) {
        val->val()[i] = incStep_[i];
    }
    val->mkStr();
    return newLoc;
}
