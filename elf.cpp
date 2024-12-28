#include "myelf.h"
#include <math.h>
#include <limits>


Elf::Elf(char* fname)
{
	code_min_ = std::numeric_limits<addr_t>::max();
	code_max_ = std::numeric_limits<addr_t>::min();
	Elf         *elf;
	Elf_Scn     *scn = NULL;
	Elf_Scn     *scn_sym = NULL;
	GElf_Shdr*   sht_sym = NULL;
	GElf_Shdr   shdr;
	Elf_Data    *data;
	int         fd, ii, count;

	elf_version(EV_CURRENT);

	fd = open(fname, O_RDONLY);
	if(fd<0) perror("fopen");

	elf = elf_begin(fd, ELF_C_READ, NULL);

	while ((scn = elf_nextscn(elf, scn)) != NULL) {
		gelf_getshdr(scn, &shdr);
		if (shdr.sh_type == SHT_SYMTAB) {
			sht_sym = new GElf_Shdr;
			memcpy(sht_sym, &shdr, sizeof(GElf_Shdr));
			scn_sym = scn;
		} 
	}

	if(sht_sym != NULL && scn_sym !=NULL) {
		data = elf_getdata(scn_sym, NULL);
		count = sht_sym->sh_size / sht_sym->sh_entsize;

		std::pair<std::map<char*,addr_t>::iterator,bool> ret;
		for (ii = 0; ii < count; ++ii) {
			GElf_Sym sym;
			gelf_getsym(data, ii, &sym);
			char* symname = elf_strptr(elf, sht_sym->sh_link, sym.st_name);
			int n = strlen(symname)+1;
			unsigned char type = ELF64_ST_TYPE(sym.st_info);
			if(type!=STT_FUNC || sym.st_value==0) continue;
			code_min_ = fmin(code_min_, sym.st_value);
			code_max_ = fmax(code_max_, sym.st_value);
			char* tmp = new char[n];
			strncpy(tmp, symname, n);
			//fprintf(stderr, "%lx, %s\n", sym.st_value, tmp);
			ret = func_.insert(std::pair<char*, addr_t>(
						tmp,
						sym.st_value));
			if(!ret.second)
				delete [] tmp;
			funcA_.insert(std::pair<addr_t, char*>(
						sym.st_value,
						tmp));
		}
	} else {
		fprintf(stderr, "ERROR: no SHT_SYMTAB\n");
		elf_end(elf);
		delete sht_sym;
		exit(1);
	}
	elf_end(elf);
	delete sht_sym;
	if(close(fd)!=0) perror("close");
}


Elf::~Elf()
{
	std::map<char*, addr_t>::iterator it = func_.begin();
	for( ; it!=func_.end(); ++it)
		delete [] it->first;
	func_.clear();
}

addr_t Elf::get_func(char* name) const 
{
	auto func = func_.find(name);
	if(func==func_.end()) {
		fprintf(stderr, "ERROR: can not find %s\n", name);
		exit(1);
	}
	return func->second;
}

const char* Elf::get_func(addr_t ip) const
{
	auto func = funcA_.lower_bound(ip);
	if(func==funcA_.end()) {
		fprintf(stderr, "ERROR: %lx out of range\n", ip);
		exit(1);
	}
	return func->second;
}
