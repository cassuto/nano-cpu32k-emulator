
#include "parse-symtable.h"

static struct sym_node *global_symtable;

/**
 * @brief Load a symbol table file.
 * @param symfile Symbol table file path;
 * @return status code.
 */
int load_symtable(const char *symfile)
{
  struct sym_node node;
  char vm_addr[16];
  FILE *fp = fopen(symfile, "r");
  if(!fp)
    return -EM_FAULT;
  while(fscanf(fp, "%s%s%s", vm_addr, node.sym_type, node.symbol)!=EOF)
    {
      struct sym_node *nd = (struct sym_node *)malloc(sizeof *nd);
      *nd = node;
      nd->vm_addr = strtol(vm_addr, NULL, 16);
      nd->next = global_symtable;
      global_symtable = nd;
    }
  fclose(fp);
  return 0;
}

/**
 * Find a symbol by its virtual address.
 * @param addr Start address of target symbol.
 * @retval pointer to struct sym_node if succeeded.
 * @retval NULL if no matched.
 */
const struct sym_node *find_symbol(vm_addr_t addr)
{
  struct sym_node *node = global_symtable;
  while(node)
    {
      if(node->vm_addr == addr)
        {
          return node;
        }
      node = node ->next;
    }
  return NULL;
}
