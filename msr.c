
#include "cpu.h"
#include "ncpu32k-msr.h"

static int cpuid_ver = 1;
static int cpuid_rev = 0;
static char enable_imm = 1;
static char enable_dmm = 1;
static char enable_icache = 1;
static char enable_dcache = 1;
static char enable_dbg = 1;
static char enable_fpu = 0;
static char enable_tsc = 1;

#define normalize_bit(val) (val ? 1:0)

#define msr_unpack_bit(reg, bit, val) \
          do { msr.reg.bit = normalize_bit(val & MSR_ ## reg ## _ ## bit ## _SHIFT); } while(0)
#define msr_unpack_field(reg, field, val) \
          do { msr.reg.field = (val & MSR_ ## reg ## _ ## bit) >> MSR_ ## reg ## _ ## bit ## _SHIFT; } while(0)
#define msr_pack_bit(reg, bit) \
          (normalize_bit(msr.reg.bit) << MSR_ ## reg ## _ ## bit ## _SHIFT);
#define msr_pack_field(reg, field) \
          ((msr.reg.field << MSR_ ## reg ## _ ## field ## _SHIFT) & MSR_ ## reg ## _ ## field);
#define val_pack_bit(reg, bit, val) \
          (normalize_bit(val) << MSR_ ## reg ## _ ## bit ## _SHIFT);
#define val_pack_field(reg, field, val) \
          ((val << MSR_ ## reg ## _ ## field ## _SHIFT) & MSR_ ## reg ## _ ## field);

void warn_illegal_access_reg(const char *reg) {
  fprintf(stderr, "warning: illegal access to %s in non-root mode at PC=%#x\n", reg, cpu_pc);
}
          
void wmsr(msr_index_t index, cpu_word_t val)
{
  switch(index)
  {
    /* MSR bank - PS */
    case MSR_PSR:
      if(msr.PSR.RM)
        {
          msr_unpack_bit(PSR, CC, val);
          msr_unpack_bit(PSR, CY, val);
          msr_unpack_bit(PSR, OV, val);
          msr_unpack_bit(PSR, OE, val);
          msr_unpack_bit(PSR, RM, val);
          msr_unpack_bit(PSR, IRE, val);
          msr_unpack_bit(PSR, IMME, val);
          msr_unpack_bit(PSR, DMME, val);
          msr_unpack_bit(PSR, ICAE, val);
          msr_unpack_bit(PSR, DCAE, val);
        }
      else
        warn_illegal_access_reg("MSR.PSR");
      break;
      
    case MSR_CPUID:
      /* read-only */
      break;

    /* MSR bank - DBG */
    case MSR_DBGR_NUMPORT:
      {
        char buff[64], *p = buff;
        snprintf(buff, sizeof(buff), "DEBUG NUM PORT - %#x\n", val);
        while(*p)
          debug_putc(*p++);
        break;
      }
      
    case MSR_DBGR_MSGPORT:
      debug_putc(val);
      break;
      
    default:
      fprintf(stderr, "wmsr() invalid register index %#x at PC=%#x\n", index, cpu_pc);
      panic(1);
      break;
  }
}

cpu_word_t rmsr(msr_index_t index)
{
  cpu_word_t ret = 0;
  switch(index)
  {
    /* MSR bank - PS */
    case MSR_PSR:
      ret |= msr_pack_bit(PSR, CC);
      ret |= msr_pack_bit(PSR, CY);
      ret |= msr_pack_bit(PSR, OV);
      ret |= msr_pack_bit(PSR, OE);
      ret |= msr_pack_bit(PSR, RM);
      ret |= msr_pack_bit(PSR, IRE);
      ret |= msr_pack_bit(PSR, IMME);
      ret |= msr_pack_bit(PSR, DMME);
      ret |= msr_pack_bit(PSR, ICAE);
      ret |= msr_pack_bit(PSR, DCAE);
      return ret;
      
    case MSR_CPUID:
      ret |= val_pack_field(CPUID, VER, cpuid_ver);
      ret |= val_pack_field(CPUID, REV, cpuid_rev);
      ret |= val_pack_bit(CPUID, FIMM, enable_imm);
      ret |= val_pack_bit(CPUID, FDMM, enable_dmm);
      ret |= val_pack_bit(CPUID, FICA, enable_icache);
      ret |= val_pack_bit(CPUID, FDCA, enable_dcache);
      ret |= val_pack_bit(CPUID, FDBG, enable_dbg);
      ret |= val_pack_bit(CPUID, FFPU, enable_fpu);
      ret |= val_pack_bit(CPUID, FTSC, enable_tsc);
      return ret;
  }
  fprintf(stderr, "rmsr() invalid register index %#x at PC=%#x\n", index, cpu_pc);
  panic(1);
  return 0;
}


void init_msr()
{
  memset(&msr, 0, sizeof msr);
  msr.PSR.RM = 1;
}
