
#include "openpx64k-emu.h"
#include "openpx64k-opcodes.h"

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
  fprintf(stderr, "OpenPX-64k emulator\n\t%s. [ram_image_file] [-r [(hex) reset_vector]]\n", program);
  return 1;
}

int main(int argc, char *argv[])
{
  int rc, i;
  FILE *fp;
  const char *filename;
  phy_addr_t reset_vector = VECT_ERST;

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
    }
  filename = argv[1];
  if( !(fp = fopen(filename, "rb")) )
    {
      perror("Open file");
      return 1;
    }

  if( (rc = cpu_exec_init(memory_size)) )
    return report_error("cpu_exec_init()", rc);

  if( (rc = memory_load_address_fp(fp, VECT_ERST)) )
    return report_error("memory_load_address_fp()", rc);


  cpu_reset(reset_vector);

  if( (rc = cpu_exec()) )
    return report_error("cpu_exec()", rc);

  return 0;
}
