
#include "cpu.h"
#include "ncpu32k-exceptions.h"
#include "parse-symtable.h"
#include "device-tree.h"

static int memory_size = 32 * 1024 * 1024;

static int
report_error(const char *func, int rc)
{
  fprintf(stderr, "failed. %s with RC=%d\n",func, rc);
  return 1;
}

static int
usage(const char *program)
{
  fprintf(stderr, "ncpu32k emulator\n\t%s. [ram_image_file] [-r [(hex) reset_vector]]\n", program);
  return 1;
}

int main(int argc, char *argv[])
{
  int rc, i;
  FILE *fp;
  const char *filename;
  vm_addr_t reset_vector = VECT_ERST;
  const char *symtable_filename = NULL;
  
  if(argc < 2)
    return usage(argv[0]);
  for(i=1; i < argc; i++)
    {
      if(0 == strcmp(argv[i], "-r"))
        {
          if(++i >= argc)
            return usage(argv[0]);
          reset_vector = strtol(argv[i], NULL, 16);
        }
      else if(0 == strcmp(argv[i], "-s"))
        {
          if(++i >= argc)
            return usage(argv[0]);
          symtable_filename = argv[i];
        }
    }
  filename = argv[1];
  if( !(fp = fopen(filename, "rb")) )
    {
      perror("Open file");
      return 1;
    }
  if(symtable_filename)
    {
      if(load_symtable(symtable_filename) < 0)
        {
          perror("Open symbol table file");
          return 1;
        }
      printf("Symbol table '%s' is loaded!\n", symtable_filename);
    }
  
  if( (rc = memory_init(memory_size)) )
    return report_error("memory_init()", rc);
  
  if( (rc = memory_load_address_fp(fp, VECT_ERST)) )
    return report_error("memory_load_address_fp()", rc);

  if( (rc = devicetree_init()) )
    return report_error("devicetree_init()", rc);

  if( (rc = cpu_exec_init()) )
    return report_error("cpu_exec_init()", rc);
  
  cpu_reset(reset_vector);

  if( (rc = cpu_exec()) )
    return report_error("cpu_exec()", rc);

  return 0;
}
