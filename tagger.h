#ifndef TAGGER_H
#define TAGGER_H

/*
 * Tagger:
 * manage tags
 * provide views on tags
 */

#include "tag.h"
#include "access.h"

class Tagger
{
	public:
		Tagger();
		~Tagger();

		int add(Tag* t);
		int add(Access* a);
		const Tag* get(size_t idx);
		const Tag* get(addr_t addr);
	private:
		std::vector<Tag*> tags_;
};

#endif
