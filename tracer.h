#ifndef TRACER_H
#define TRACER_H

#include "types.h"
#include "tag.h"
#include "branch.h"

#include <set>
#include <vector>
#include <map>

typedef std::multimap<addr_t, Tag*> tagmap;
typedef std::multimap<addr_t, Branch*> branchmap;

class Tracer
{
	public:
		Tracer(int argc, char** argv);
		~Tracer();

		std::set<Tag*> tags() const {return tags_;}
		const std::vector<Branch*> branchs() const;

		// run target until event occurs
		int trace();

		addr_t heap_min() const { return heap_min_; }
		addr_t heap_max() const { return heap_max_; }
		addr_t stack_min() const { return stack_min_; }
		addr_t stack_max() const { return stack_max_; }
		addr_t last() const {return last_;}

	private:
		int handle_cjmp();
		int handle_segv(addr_t loc, addr_t rip);
		int addTag(Tag* t);
		int addBranch(Branch* t);

		std::set<Tag*> tags_; // list of all tags
		std::vector<Tag*> last_tag_;
		tagmap mem_tags_; // access to tags via memory address
		std::vector<Branch*> branchs_;
		std::map<addr_t, Branch*> rip_branch_;

		addr_t heap_min_;
		addr_t heap_max_;
		addr_t stack_min_;
		addr_t stack_max_;
		addr_t last_;
};

#endif
