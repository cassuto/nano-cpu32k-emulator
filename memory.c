
#include "openpx64k-emu.h"

char *cpu_memory;

int
memory_load_address_fp(FILE *fp, phy_addr_t baseaddr)
{
  size_t pos = 0;
  size_t len;
  
  while( (len = fread(cpu_memory + baseaddr + pos, 1, CHUNK_SIZE, fp)) > 0 )
    {
      pos += len;
    }
  return 0;
}
