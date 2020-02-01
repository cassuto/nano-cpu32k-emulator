#ifndef NCPU32K_CPU_H
#define NCPU32K_CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef int32_t cpu_word_t;
typedef uint32_t cpu_unsigned_word_t;
typedef uint32_t vm_addr_t;
typedef uint32_t vm_signed_addr_t;
typedef uint32_t phy_addr_t;
typedef uint32_t phy_signed_addr_t;
typedef uint32_t insn_t;
typedef uint16_t msr_index_t;

#define CHUNK_SIZE 8192

#define MAX_ITLB_COUNT 128
#define MAX_DTLB_COUNT 128

#define EM_SUCCEEDED 0
#define EM_FAULT 1
#define EM_NO_MEMORY 2
#define EM_TLB_MISS 3
#define EM_PAGE_FAULT 4

/*
 * exec.c
 */
struct regfile_s
{
  cpu_word_t r[32];
};
extern vm_addr_t cpu_pc;
extern struct msr_s msr;
int cpu_exec_init(int memory_size);
void cpu_reset(vm_addr_t reset_vect);
int cpu_exec(void);
void cpu_raise_exception(vm_addr_t vector, vm_addr_t lsa, char syscall);
void memory_breakpoint(vm_addr_t addr, uint32_t val);

/*
 * i-mmu.c
 */
int immu_translate_vma(vm_addr_t va, phy_addr_t *pa);

/*
 * d-mmu.c
 */
int dmmu_translate_vma(vm_addr_t va, phy_addr_t *pa, char store_insn);

/*
 * msr.c
 */
struct psr_s
{
  char CC;
  char CY;
  char OV;
  char OE;
  char RM;
  char IRE;
  char IMME;
  char DMME;
  char ICAE;
  char DCAE;
};
/* ITLB */
struct itlbl_s
{
  char V;
  vm_addr_t VPN;
};
struct itlbh_s
{
  char P;
  char D;
  char A;
  char UX;
  char RX;
  char NC;
  char S;
  vm_addr_t PPN;
};
/* DTLB */
struct dtlbl_s
{
  char V;
  vm_addr_t VPN;
};
struct dtlbh_s
{
  char P;
  char D;
  char A;
  char UW;
  char UR;
  char RW;
  char RR;
  char NC;
  char S;
  vm_addr_t PPN;
};
struct msr_s
{
  struct psr_s PSR;
  struct psr_s EPSR;
  vm_addr_t EPC;
  vm_addr_t ELSA;
  struct itlbl_s ITLBL[MAX_ITLB_COUNT];
  struct itlbh_s ITLBH[MAX_ITLB_COUNT];
  struct dtlbl_s DTLBL[MAX_DTLB_COUNT];
  struct dtlbh_s DTLBH[MAX_DTLB_COUNT];
};
void init_msr();
void wmsr(msr_index_t index, cpu_word_t val);
cpu_word_t rmsr(msr_index_t index);
extern const int immu_tlb_count;
extern const int dmmu_tlb_count;

/*
 * memory.c
 */
int memory_load_address_fp(FILE *fp, phy_addr_t baseaddr);

extern char *cpu_memory;

/*
 * debug.c
 */
void debug_putc(uint8_t ch);

static inline uint8_t phy_readm8(phy_addr_t addr)
{
  return cpu_memory[addr];
}
static inline uint16_t phy_readm16(phy_addr_t addr)
{
  return ((uint16_t)phy_readm8(addr + 1) << 8) | (uint16_t)phy_readm8(addr);
}
static inline uint32_t phy_readm32(phy_addr_t addr)
{
  return ((uint32_t)phy_readm8(addr + 3) << 24) |
         ((uint32_t)phy_readm8(addr + 2) << 16) |
         ((uint32_t)phy_readm8(addr + 1) << 8) |
         (uint32_t)phy_readm8(addr);
}

static inline void phy_writem8(phy_addr_t addr, uint8_t val)
{
  cpu_memory[addr] = val;
}
static inline void phy_writem16(phy_addr_t addr, uint16_t val)
{
  phy_writem8(addr, val & 0x00ff);
  phy_writem8(addr + 1, val >> 8);
}
static inline void phy_writem32(phy_addr_t addr, uint32_t val)
{
  phy_writem8(addr, val & 0xff);
  phy_writem8(addr + 1, (val >> 8) & 0xff);
  phy_writem8(addr + 2, (val >> 16) & 0xff);
  phy_writem8(addr + 3, val >> 24);
}

#define panic(rc) exit(rc);

#endif
