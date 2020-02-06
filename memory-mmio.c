
#include "cpu.h"

#define MMIO_PHY_BASE (0xc0000000)

struct mmio_node
{
  char write;
  phy_addr_t start_addr;
  phy_addr_t end_addr;
  void *callback;
  struct mmio_node *next;
};

static struct mmio_node *mmio8, *mmio16, *mmio32;
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

static inline struct mmio_node *
match_mmio_handler(struct mmio_node *doamin, phy_addr_t addr, char write)
{
  if(addr >= MMIO_PHY_BASE)
    {
      for(struct mmio_node *node=doamin; node; node=node->next)
        {
          if (node->write==write && node->start_addr >= addr && addr <= node->end_addr)
            {
              return node;
            }
        }
    }
  return NULL;
}

/*
 * Physical Memory accessing functions, in little-endian.
 * Note that there is not any address translation for physical memory.
 * PhyMemory must be accessed through these funcs for CPU.
 */

inline uint8_t
phy_readm8(phy_addr_t addr)
{
  struct mmio_node *mmio_handler = match_mmio_handler(mmio8, addr, 0);
  if(mmio_handler)
    {
      return ((pfn_readm8)mmio_handler->callback)(addr);
    }
  else
    return cpu_memory[addr];
}

inline uint16_t
phy_readm16(phy_addr_t addr)
{
  struct mmio_node *mmio_handler = match_mmio_handler(mmio16, addr, 0);
  if(mmio_handler)
    {
      return ((pfn_readm16)mmio_handler->callback)(addr);
    }
  else
    return ((uint16_t)phy_readm8(addr + 1) << 8) | (uint16_t)phy_readm8(addr);
}

inline uint32_t
phy_readm32(phy_addr_t addr)
{
  struct mmio_node *mmio_handler = match_mmio_handler(mmio32, addr, 0);
  if(mmio_handler)
    {
      return ((pfn_readm32)mmio_handler->callback)(addr);
    }
  else
    {
      return ((uint32_t)phy_readm8(addr + 3) << 24) |
             ((uint32_t)phy_readm8(addr + 2) << 16) |
             ((uint32_t)phy_readm8(addr + 1) << 8) |
             (uint32_t)phy_readm8(addr);
    }
}

inline void
phy_writem8(phy_addr_t addr, uint8_t val)
{
  struct mmio_node *mmio_handler = match_mmio_handler(mmio8, addr, 1);
  if(mmio_handler)
    {
      ((pfn_writem8)mmio_handler->callback)(addr, val);
    }
  else
    cpu_memory[addr] = val;
}

inline void
phy_writem16(phy_addr_t addr, uint16_t val)
{
  struct mmio_node *mmio_handler = match_mmio_handler(mmio16, addr, 1);
  if(mmio_handler)
    {
      ((pfn_writem16)mmio_handler->callback)(addr, val);
    }
  else
    {
      phy_writem8(addr, val & 0x00ff);
      phy_writem8(addr + 1, val >> 8);
    }
}

inline void
phy_writem32(phy_addr_t addr, uint32_t val)
{
  struct mmio_node *mmio_handler = match_mmio_handler(mmio32, addr, 1);
  if(mmio_handler)
    {
      ((pfn_writem32)mmio_handler->callback)(addr, val);
    }
  else
    {
      phy_writem8(addr, val & 0xff);
      phy_writem8(addr + 1, (val >> 8) & 0xff);
      phy_writem8(addr + 2, (val >> 16) & 0xff);
      phy_writem8(addr + 3, val >> 24);
    }
}

/*
 * Register MMIO devices
 */
static inline
void mmio_append_node(struct mmio_node **doamin,
                      char write,
                      phy_addr_t start_addr,
                      phy_addr_t end_addr,
                      void *callback)
{
  struct mmio_node *node = (struct mmio_node *)malloc(sizeof *node);
  node->write = write;
  node->start_addr = start_addr;
  node->end_addr = end_addr;
  node->callback = callback;
  node->next = *doamin;
  *doamin = node;
}

void
mmio_register_writem8(phy_addr_t start_addr, phy_addr_t end_addr, pfn_writem8 callback)
{
  mmio_append_node(&mmio8, 1, start_addr, end_addr, callback);
}

void
mmio_register_writem16(phy_addr_t start_addr, phy_addr_t end_addr, pfn_writem16 callback)
{
  mmio_append_node(&mmio16, 1, start_addr, end_addr, callback);
}

void
mmio_register_writem32(phy_addr_t start_addr, phy_addr_t end_addr, pfn_writem32 callback)
{
  mmio_append_node(&mmio32, 1, start_addr, end_addr, callback);
}

void
mmio_register_readm8(phy_addr_t start_addr, phy_addr_t end_addr, pfn_readm8 callback)
{
  mmio_append_node(&mmio8, 0, start_addr, end_addr, callback);
}

void
mmio_register_readm16(phy_addr_t start_addr, phy_addr_t end_addr, pfn_readm16 callback)
{
  mmio_append_node(&mmio16, 0, start_addr, end_addr, callback);
}

void
mmio_register_readm32(phy_addr_t start_addr, phy_addr_t end_addr, pfn_readm32 callback)
{
  mmio_append_node(&mmio32, 0, start_addr, end_addr, callback);
}
