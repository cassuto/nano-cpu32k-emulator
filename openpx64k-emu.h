#ifndef OPENPX64K_EMU_H
#define OPENPX64K_EMU_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef int32_t cpu_word_t;
typedef uint32_t cpu_unsigned_word_t;
typedef uint32_t phy_addr_t;
typedef uint32_t phy_signed_addr_t;
typedef uint32_t insn_t;

#define CHUNK_SIZE 8192

#define EM_SUCCEEDED 0
#define EM_FAULT 1
#define EM_NO_MEMORY 2

/*
 * exec.c
 */
struct regfile_s
{
  cpu_word_t r[32];
};

struct psr_s
{
  char cf;
};

struct csmr_s
{
  struct psr_s psr;
};

int cpu_exec_init(int memory_size);
void cpu_reset(void);
int cpu_exec(void);
void cpu_raise_excp(int excp_no);

/*
 * memory.c
 */
int memory_load_address_fp(FILE *fp, phy_addr_t baseaddr);

extern char *cpu_memory;

/*
 * debug.c
 */
void debug_putc(uint8_t ch);

#define DEBUG_CHAR_PORT 0xe0000000

static inline uint8_t readm8(phy_addr_t addr)
{
  return cpu_memory[addr];
}
static inline uint16_t readm16(phy_addr_t addr)
{
  return ((uint16_t)readm8(addr + 1) << 8) | (uint16_t)readm8(addr);
}
static inline uint32_t readm32(phy_addr_t addr)
{
  return ((uint32_t)readm8(addr + 3) << 24) |
         ((uint32_t)readm8(addr + 2) << 16) |
         ((uint32_t)readm8(addr + 1) << 8) |
         (uint32_t)readm8(addr);
}

static inline void writem8(phy_addr_t addr, uint8_t val)
{
  if(addr == DEBUG_CHAR_PORT)
    debug_putc(val);
  else
    cpu_memory[addr] = val;
}
static inline void writem16(phy_addr_t addr, uint16_t val)
{
  writem8(addr, val & 0x00ff);
  writem8(addr + 1, val >> 8);
}
static inline void writem32(phy_addr_t addr, uint32_t val)
{
  writem8(addr, val & 0xff);
  writem8(addr + 1, (val >> 8) & 0xff);
  writem8(addr + 2, (val >> 16) & 0xff);
  writem8(addr + 3, val >> 24);
}

#endif