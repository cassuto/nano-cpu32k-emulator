
#include "cpu.h"

char *cpu_memory;

int
memory_init(int memory_size)
{
  if( !(cpu_memory = calloc(memory_size, 1)) )
    return -EM_NO_MEMORY;
  return 0;
}

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

/*
 * Physical Memory accessing functions, in little-endian.
 * Note that there is not any address translation for physical memory.
 * PhyMemory must be accessed through these funcs whatever CPU/Devices are.
 */

inline uint8_t
phy_readm8(phy_addr_t addr)
{
  return cpu_memory[addr];
}

inline uint16_t
phy_readm16(phy_addr_t addr)
{
  return ((uint16_t)phy_readm8(addr + 1) << 8) | (uint16_t)phy_readm8(addr);
}

inline uint32_t
phy_readm32(phy_addr_t addr)
{
  return ((uint32_t)phy_readm8(addr + 3) << 24) |
         ((uint32_t)phy_readm8(addr + 2) << 16) |
         ((uint32_t)phy_readm8(addr + 1) << 8) |
         (uint32_t)phy_readm8(addr);
}

inline void
phy_writem8(phy_addr_t addr, uint8_t val)
{
  cpu_memory[addr] = val;
}

inline void
phy_writem16(phy_addr_t addr, uint16_t val)
{
  phy_writem8(addr, val & 0x00ff);
  phy_writem8(addr + 1, val >> 8);
}

inline void
phy_writem32(phy_addr_t addr, uint32_t val)
{
  phy_writem8(addr, val & 0xff);
  phy_writem8(addr + 1, (val >> 8) & 0xff);
  phy_writem8(addr + 2, (val >> 16) & 0xff);
  phy_writem8(addr + 3, val >> 24);
}
