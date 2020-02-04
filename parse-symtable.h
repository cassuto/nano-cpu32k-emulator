#ifndef PARSE_SYMTABLE_H_
#define PARSE_SYMTABLE_H_

#include "cpu.h"

struct sym_node {
  vm_addr_t vm_addr;
  char sym_type[8];
  char symbol[2048];
  struct sym_node *next;
};

extern int load_symtable(const char *symfile);
extern const struct sym_node *find_symbol(vm_addr_t addr);

#endif /* PARSE_SYMTABLE_H_ */