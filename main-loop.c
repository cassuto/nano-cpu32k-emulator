
#include "openpx64k-emu.h"
#include "openpx64k-opcodes.h"

static int memory_size = 8 * 1024 * 1024;

static int
report_error(const char *func, int rc)
{
  fprintf(stderr, "failed. %s with RC=%d\n",func, rc);
  return 1;
}

int main(void)
{
  int rc;
  FILE *fp;
  if( !(fp = fopen("bootloader.bin", "rb")) )
    {
      perror("Open file");
      return 1;
    }

  if( (rc = cpu_exec_init(memory_size)) )
    return report_error("cpu_exec_init()", rc);

  if( (rc = memory_load_address_fp(fp, VECT_ERST)) )
    return report_error("memory_load_address_fp()", rc);


  cpu_reset();

  if( (rc = cpu_exec()) )
    return report_error("cpu_exec()", rc);

  return 0;
}
