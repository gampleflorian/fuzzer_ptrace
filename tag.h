#ifndef TAG_H
#define TAG_H

#include "types.h"
#include "access.h"
#include "trace.h"

#include <vector>
#include <map>
#include <tr1/functional>

enum TAGTYPE{TT_NUM, TT_STR, TT_PTR, TT_UNKNOWN};
class Tag
{
	public:
		Tag(addr_t loc, int len, addr_t rip, TAGTYPE type=TT_UNKNOWN);
		Tag(Access* a, TAGTYPE type=TT_UNKNOWN);
		~Tag();

		const Val* val(addr_t rip) const;
		const Access* lastAccess() const;
		const std::map<addr_t, Access*>& rip_access() const {return rip_access_;}
		const Val* init_val() const {return init_val_;}
		addr_t loc() const;
		void loc(addr_t newLoc); 
		int len() const;
		void len(int newLen);
		TAGTYPE type() const { return type_;} // getter returns assumed tagtype
		void type(TAGTYPE type) { type_=type;} // getter returns assumed tagtype

		// log access
		int log(addr_t rip, addr_t loc);

		int addTraceF(addr_t rip, Tag* t, TTYPE type);
		int addTraceB(addr_t rip, Tag* t, TTYPE type);

		const Trace* tforw(addr_t rip) const;
		const Trace* tbackw(addr_t rip) const;

	private:
		TAGTYPE guessType(); // guess what type the tagged memory is
		int addAccess(Access* a);
		std::vector<Access*> access_;
		std::map<addr_t, Access*> rip_access_;

		std::map<addr_t, Trace*, std::greater<addr_t>> tforw_;
		std::map<addr_t, Trace*, std::greater<addr_t>> tbackw_;

		addr_t loc_;
		int len_;
		TAGTYPE type_;
		const Val* init_val_;
};

namespace std { namespace tr1 {
	template <> struct hash<Tag>
	{
		size_t operator()(const Tag& t) const
		{
			return std::tr1::hash<int>()(t.len()) ^
				std::tr1::hash<addr_t>()(t.loc()) << 4;
		}
	};
}}

#endif
