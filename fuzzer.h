#ifndef FUZZER_H
#define FUZZER_H

#include "types.h"
#include "tracer.h"
#include "vrange.h"

class Fuzzer {
	public:
		Fuzzer(int argc, char** argv);
		~Fuzzer();

		void fuzz(addr_t from, addr_t to, const std::vector<const VRange*>& vrange);

		const Tracer* tracer() const { return tracer_;}

	private:
		Tracer* tracer_;
};

#endif
