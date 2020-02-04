
#include "cpu.h"
#include "ncpu32k-msr.h"

#define immu_tlb_count_log2 7 /* 0~7 = log2(count of TLBs) */
#define dmmu_tlb_count_log2 7 /* 0~7 = log2(count of TLBs) */

static const int cpuid_ver = 1;
static const int cpuid_rev = 0;
static const char enable_imm = 1;
static const char enable_dmm = 1;
static const char enable_icache = 0;
static const char enable_dcache = 0;
static const char enable_dbg = 1;
static const char enable_fpu = 0;
static const char enable_tsc = 1;
static const char enable_irqc = 1;
const int immu_tlb_count = (1<<immu_tlb_count_log2);
const int dmmu_tlb_count = (1<<dmmu_tlb_count_log2);

#define normalize_bit(val) (val ? 1:0)

#define msr_unpack_bit(reg, bit, val) \
          do { msr.reg.bit = normalize_bit(val & MSR_ ## reg ## _ ## bit); } while(0)
#define msr_unpack_bit_dup(reg2,reg, bit, val) \
          do { msr.reg2.bit = normalize_bit(val & MSR_ ## reg ## _ ## bit); } while(0)
#define msr_unpack_field(reg, field, val) \
          do { msr.reg.field = (val & MSR_ ## reg ## _ ## field) >> MSR_ ## reg ## _ ## field ## _SHIFT; } while(0)
#define msr_unpack_field_dup(reg2, reg, field, val) \
          do { msr.reg2.field = (val & MSR_ ## reg ## _ ## field) >> MSR_ ## reg ## _ ## field ## _SHIFT; } while(0)
#define msr_pack_bit(reg, bit) \
          (normalize_bit(msr.reg.bit) << MSR_ ## reg ## _ ## bit ## _SHIFT)
#define msr_pack_bit_dup(reg2,reg, bit) \
          (normalize_bit(msr.reg2.bit) << MSR_ ## reg ## _ ## bit ## _SHIFT)
#define msr_pack_field(reg, field) \
          ((msr.reg.field << MSR_ ## reg ## _ ## field ## _SHIFT) & MSR_ ## reg ## _ ## field)
#define msr_pack_field_dup(reg2, reg, field) \
          ((msr.reg2.field << MSR_ ## reg ## _ ## field ## _SHIFT) & MSR_ ## reg ## _ ## field)
#define val_pack_bit(reg, bit, val) \
          (normalize_bit(val) << MSR_ ## reg ## _ ## bit ## _SHIFT)
#define val_pack_field(reg, field, val) \
          ((val << MSR_ ## reg ## _ ## field ## _SHIFT) & MSR_ ## reg ## _ ## field)

void warn_illegal_access_reg(const char *reg) {
  fprintf(stderr, "warning: illegal access to %s in non-root mode at PC=%#x\n", reg, cpu_pc);
}
          
void wmsr(msr_index_t index, cpu_word_t v)
{
  cpu_unsigned_word_t val=v;
  if(msr.PSR.RM)
    {
      switch(index)
      {
        /* MSR bank - PS */
        case MSR_PSR:
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
          break;
        
        case MSR_EPSR:
          msr_unpack_bit_dup(EPSR, PSR, CC, val);
          msr_unpack_bit_dup(EPSR, PSR, CY, val);
          msr_unpack_bit_dup(EPSR, PSR, OV, val);
          msr_unpack_bit_dup(EPSR, PSR, OE, val);
          msr_unpack_bit_dup(EPSR, PSR, RM, val);
          msr_unpack_bit_dup(EPSR, PSR, IRE, val);
          msr_unpack_bit_dup(EPSR, PSR, IMME, val);
          msr_unpack_bit_dup(EPSR, PSR, DMME, val);
          msr_unpack_bit_dup(EPSR, PSR, ICAE, val);
          msr_unpack_bit_dup(EPSR, PSR, DCAE, val);
          break;
          
        case MSR_EPC:
          msr.EPC = val;
          break;
          
        case MSR_ELSA:
          msr.ELSA = val;
          break;
          
        /* MSR bank - DBG */
        case MSR_DBGR_NUMPORT:
          {
#if 1
            if(val==0) {
              printf("break point!");
              getchar();
            }
#endif
            char buff[64], *p = buff;
            snprintf(buff, sizeof(buff), "DEBUG NUM PORT - %#x\n", val);
            while(*p)
              debug_putc(*p++);
            break;
          }
          
        case MSR_DBGR_MSGPORT:
          debug_putc(val);
          break;
          
        /* MSR bank - TSC */
        case MSR_TSR:
          msr.TSR = val;
          break;
         
        case MSR_TCR:
          msr_unpack_field(TCR, CNT, val);
          msr_unpack_bit(TCR, EN, val);
          msr_unpack_bit(TCR, P, val);
          tsc_update_tcr();
          break;
          
        /* MSR bank - IRQC */
        case MSR_IMR:
          msr.IMR = val;
          break;
          
        default:
          {
            /* MSR bank - IMM */
            if(index >= MSR_ITLBL && index < MSR_ITLBL+immu_tlb_count)
              {
                int offset = index - MSR_ITLBL;
                msr_unpack_bit_dup(ITLBL[offset], ITLBL, V, val);
                msr_unpack_field_dup(ITLBL[offset], ITLBL, VPN, val);
                break;
              }
            if(index >= MSR_ITLBH && index < MSR_ITLBH+immu_tlb_count)
              {
                int offset = index - MSR_ITLBH;
                msr_unpack_bit_dup(ITLBH[offset], ITLBH, P, val);
                msr_unpack_bit_dup(ITLBH[offset], ITLBH, D, val);
                msr_unpack_bit_dup(ITLBH[offset], ITLBH, A, val);
                msr_unpack_bit_dup(ITLBH[offset], ITLBH, UX, val);
                msr_unpack_bit_dup(ITLBH[offset], ITLBH, RX, val);
                msr_unpack_bit_dup(ITLBH[offset], ITLBH, NC, val);
                msr_unpack_bit_dup(ITLBH[offset], ITLBH, S, val);
                msr_unpack_field_dup(ITLBH[offset], ITLBH, PPN, val);
                break;
              }
              
            /* MSR bank - DMM */
            if(index >= MSR_DTLBL && index < MSR_DTLBL+dmmu_tlb_count)
              {
                int offset = index - MSR_DTLBL;
                msr_unpack_bit_dup(DTLBL[offset], DTLBL, V, val);
                msr_unpack_field_dup(DTLBL[offset], DTLBL, VPN, val);
                break;
              }
            if(index >= MSR_DTLBH && index < MSR_DTLBH+dmmu_tlb_count)
              {
                int offset = index - MSR_DTLBH;
                msr_unpack_bit_dup(DTLBH[offset], DTLBH, P, val);
                msr_unpack_bit_dup(DTLBH[offset], DTLBH, D, val);
                msr_unpack_bit_dup(DTLBH[offset], DTLBH, A, val);
                msr_unpack_bit_dup(DTLBH[offset], DTLBH, UW, val);
                msr_unpack_bit_dup(DTLBH[offset], DTLBH, UR, val);
                msr_unpack_bit_dup(DTLBH[offset], DTLBH, RW, val);
                msr_unpack_bit_dup(DTLBH[offset], DTLBH, RR, val);
                msr_unpack_bit_dup(DTLBH[offset], DTLBH, NC, val);
                msr_unpack_bit_dup(DTLBH[offset], DTLBH, S, val);
                msr_unpack_field_dup(DTLBH[offset], DTLBH, PPN, val);
                break;
              }
              
            fprintf(stderr, "wmsr() invalid register index %#x at PC=%#x\n", index, cpu_pc);
            panic(1);
            break;
          }
      } /* switch */
    }
  else
    warn_illegal_access_reg("MSR.PSR");
}

cpu_word_t rmsr(msr_index_t index)
{
  if(msr.PSR.RM)
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
          ret |= val_pack_bit(CPUID, FIRQC, enable_irqc);
          ret |= val_pack_bit(CPUID, FTSC, enable_tsc);
          return ret;
          
        case MSR_EPSR:
          ret |= msr_pack_bit_dup(EPSR, PSR, CC);
          ret |= msr_pack_bit_dup(EPSR, PSR, CY);
          ret |= msr_pack_bit_dup(EPSR, PSR, OV);
          ret |= msr_pack_bit_dup(EPSR, PSR, OE);
          ret |= msr_pack_bit_dup(EPSR, PSR, RM);
          ret |= msr_pack_bit_dup(EPSR, PSR, IRE);
          ret |= msr_pack_bit_dup(EPSR, PSR, IMME);
          ret |= msr_pack_bit_dup(EPSR, PSR, DMME);
          ret |= msr_pack_bit_dup(EPSR, PSR, ICAE);
          ret |= msr_pack_bit_dup(EPSR, PSR, DCAE);
          return ret;
          
        case MSR_EPC:
          return msr.EPC;
          
        case MSR_ELSA:
          return msr.ELSA;
          
        case MSR_COREID:
          /* SMP is not supported yet */
          return 0;
          
        /* MSR bank - IMM */
        case MSR_IMMID:
          ret = val_pack_field(IMMID, STLB, immu_tlb_count_log2);
          return ret;
          
        /* MSR bank - DMM */
        case MSR_DMMID:
          ret = val_pack_field(DMMID, STLB, dmmu_tlb_count_log2);
          return ret;
          
        /* MSR bank - TSC */
        case MSR_TSR:
          return msr.TSR;
         
        case MSR_TCR:
          ret = msr_pack_field(TCR, CNT);
          ret |= msr_pack_bit(TCR, EN);
          ret |= msr_pack_bit(TCR, P);
          return ret;
          
        /* MSR bank - IRQC */
        case MSR_IMR:
          return msr.IMR;

        case MSR_IRR:
          return msr.IRR;
          
        default:
          {
            /* MSR bank - IMM */
            if(index >= MSR_ITLBL && index < MSR_ITLBL+immu_tlb_count)
              {
                int offset = index - MSR_ITLBL;
                ret = msr_pack_bit_dup(ITLBL[offset], ITLBL, V);
                ret |= msr_pack_field_dup(ITLBL[offset], ITLBL, VPN);
                return ret;
              }
            if(index >= MSR_ITLBH && index < MSR_ITLBH+immu_tlb_count)
              {
                int offset = index - MSR_ITLBH;
                ret = msr_pack_bit_dup(ITLBH[offset], ITLBH, P);
                ret |= msr_pack_bit_dup(ITLBH[offset], ITLBH, D);
                ret |= msr_pack_bit_dup(ITLBH[offset], ITLBH, A);
                ret |= msr_pack_bit_dup(ITLBH[offset], ITLBH, UX);
                ret |= msr_pack_bit_dup(ITLBH[offset], ITLBH, RX);
                ret |= msr_pack_bit_dup(ITLBH[offset], ITLBH, NC);
                ret |= msr_pack_bit_dup(ITLBH[offset], ITLBH, S);
                ret |= msr_pack_field_dup(ITLBH[offset], ITLBH, PPN);
                return ret;
              }
              
            /* MSR bank - DMM */
            if(index >= MSR_DTLBL && index < MSR_DTLBL+dmmu_tlb_count)
              {
                int offset = index - MSR_DTLBL;
                ret = msr_pack_bit_dup(DTLBL[offset], DTLBL, V);
                ret |= msr_pack_field_dup(DTLBL[offset], DTLBL, VPN);
                break;
              }
            if(index >= MSR_DTLBH && index < MSR_DTLBH+dmmu_tlb_count)
              {
                int offset = index - MSR_DTLBH;
                ret = msr_pack_bit_dup(DTLBH[offset], DTLBH, P);
                ret |= msr_pack_bit_dup(DTLBH[offset], DTLBH, D);
                ret |= msr_pack_bit_dup(DTLBH[offset], DTLBH, A);
                ret |= msr_pack_bit_dup(DTLBH[offset], DTLBH, UW);
                ret |= msr_pack_bit_dup(DTLBH[offset], DTLBH, UR);
                ret |= msr_pack_bit_dup(DTLBH[offset], DTLBH, RW);
                ret |= msr_pack_bit_dup(DTLBH[offset], DTLBH, RR);
                ret |= msr_pack_bit_dup(DTLBH[offset], DTLBH, NC);
                ret |= msr_pack_bit_dup(DTLBH[offset], DTLBH, S);
                ret |= msr_pack_field_dup(DTLBH[offset], DTLBH, PPN);
                return ret;
              }
              
            fprintf(stderr, "rmsr() invalid register index %#x at PC=%#x\n", index, cpu_pc);
            panic(1);
            break;
          }
      }
    }
  else
    warn_illegal_access_reg("MSR.PSR");
  
  return 0;
}

void init_msr()
{
  memset(&msr, 0, sizeof msr);
  msr.PSR.RM = 1;
}
