
#include "cpu.h"
#include "ncpu32k-opcodes.h"
#include "ncpu32k-exceptions.h"

/*
 * Configurations
 */
//#define TRACE_CALL_STACK
/* #undef TRACE_CALL_STACK */
//#define TRACE_STACK_POINTER
/* #undef TRACE_STACK_POINTER */
//#define TRACE_EXCEPTION
/* #undef TRACE_EXCEPTION */
#define VERBOSE 1

/*
 * Global data variables
 */
vm_addr_t cpu_pc = 0;
static struct regfile_s cpu_regfile;
#ifdef TRACE_CALL_STACK
static int stack_depth = 0;
#endif
struct msr_s msr;

#if VERBOSE > 1
#define verbose_print(...) printf(__VA_ARGS__)
#else
#define verbose_print(...) ((void)0)
#endif
#if VERBOSE == 1
#define verbose_print_1(...) printf(__VA_ARGS__)
#else
#define verbose_print_1(...) ((void)0)
#endif

int
cpu_exec_init(int memory_size)
{
  if( !(cpu_memory = calloc(memory_size, 1)) )
    return -EM_NO_MEMORY;
  return 0;
}

void
cpu_reset(vm_addr_t reset_vect)
{
  cpu_pc = reset_vect;

  memset(&cpu_regfile.r, 0, sizeof(cpu_regfile.r));
  memset(&msr, 0, sizeof msr);
  init_msr();
}

/*
 * Debugging staffs
 */
#ifdef TRACE_CALL_STACK
static inline void
trace_call_stack_jmp(vm_addr_t insn_pc, vm_addr_t lnk_pc, vm_addr_t new_pc)
{
  verbose_print_1("%d# (%#x): call %x, ret=%x\n", ++stack_depth, insn_pc, new_pc, lnk_pc);
}

static inline void
trace_call_stack_return(vm_addr_t insn_pc, vm_addr_t new_pc)
{
  verbose_print_1("%d# (%#x): return to %x\n", --stack_depth, insn_pc, new_pc);
}
#endif

#ifdef TRACE_STACK_POINTER
static inline void
trace_stack_pointer_ld(vm_addr_t pc, vm_addr_t addr, cpu_word_t sp)
{
  verbose_print_1("(%#x): sp <- &%#x(%#x)\n", pc, addr, sp);
}

static inline void
trace_stack_pointer_mov(vm_addr_t pc, cpu_word_t sp)
{
  verbose_print_1("(%#x): sp <- %#x\n", pc, sp);
}

static inline void
trace_stack_pointer_st(vm_addr_t pc, vm_addr_t addr, cpu_word_t sp)
{
  verbose_print_1("(%#x): %#x <- sp(%#x)\n", pc, addr, sp);
}
#endif

void
memory_breakpoint(vm_addr_t addr, uint32_t val)
{
  verbose_print_1("memory BP (%#x): addr = %#x, val = %#x\n", cpu_pc, addr, val);
}


static inline void
cpu_set_reg(uint16_t addr, cpu_word_t val)
{
  if(addr >= 32)
  {
    fprintf(stderr, "cpu_set_reg() invalid register index at PC=%#x\n", cpu_pc);
    panic(1);
  }
  if(addr)
    cpu_regfile.r[addr] = val;
#ifdef TRACE_STACK_POINTER
  if(addr == ADDR_SP) trace_stack_pointer_mov(cpu_pc, val);
#endif
}

static inline cpu_word_t
cpu_get_reg(uint16_t addr)
{
  if(addr >= 32)
  {
    fprintf(stderr, "cpu_get_reg() invalid register index at PC=%#x, index=%#x\n", cpu_pc, addr);
    panic(1);
  }
  if(addr)
    return cpu_regfile.r[addr];
  else
    return 0;
}

static inline vm_signed_addr_t
rel26_sig_ext(uint32_t rel26)
{
  return (((int32_t)(rel26<<INSN_LEN_SHIFT) ^ 0x8000000) - 0x8000000);
}

int
cpu_exec(void)
{
  for(;;)
    {
      /* not a cycle-accuracy emulation for TSC */
      if (tsc_clk() < 0)
        {
          goto handle_exception;
        }
      
      /* response asynchronous interrupts */
      if (irqc_handle_irqs() < 0)
        {
          goto handle_exception;
        }
      
      /* fetch instruction */
      phy_addr_t insn_pa = 0;
      if (immu_translate_vma(cpu_pc, &insn_pa) < 0)
        {
          goto handle_exception;
        }
      insn_t current_ins = (insn_t)phy_readm32(insn_pa);
      
      /* decode and execute */
      uint16_t opcode = current_ins & INS32_MASK_OPCODE;
      uint16_t rs1 = INS32_GET_BITS(current_ins, RS1);
      uint16_t rs2 = INS32_GET_BITS(current_ins, RS2);
      uint16_t rd = INS32_GET_BITS(current_ins, RD);
      uint32_t uimm18 = INS32_GET_BITS(current_ins, IMM18);
      uint16_t uimm14 = INS32_GET_BITS(current_ins, IMM14);
      int16_t simm14 = (((int16_t)uimm14) ^ 0x2000) - 0x2000; /* sign extend */
      uint32_t rel26 = INS32_GET_BITS(current_ins, REL26);
      uint8_t attr = INS32_GET_BITS(current_ins, ATTR);

      switch( opcode )
        {
          case INS32_OP_AND:
            cpu_set_reg(rd, cpu_get_reg(rs1) & cpu_get_reg(rs2));
            break;
          case INS32_OP_AND_I:
            cpu_set_reg(rd, cpu_get_reg(rs1) & uimm14);
            break;
            
          case INS32_OP_OR:
            cpu_set_reg(rd, cpu_get_reg(rs1) | cpu_get_reg(rs2));
            break;
          case INS32_OP_OR_I:
            cpu_set_reg(rd, cpu_get_reg(rs1) | uimm14);
            break;
            
          case INS32_OP_XOR:
            cpu_set_reg(rd, cpu_get_reg(rs1) ^ cpu_get_reg(rs2));
            break;
          case INS32_OP_XOR_I:
            cpu_set_reg(rd, cpu_get_reg(rs1) ^ simm14);
            break;
            
          case INS32_OP_LSL:
            cpu_set_reg(rd, cpu_get_reg(rs1) << cpu_get_reg(rs2));
            break;
          case INS32_OP_LSL_I:
            cpu_set_reg(rd, cpu_get_reg(rs1) << uimm14);
            break;
            
          case INS32_OP_LSR:
            cpu_set_reg(rd, (cpu_unsigned_word_t)cpu_get_reg(rs1) >> cpu_get_reg(rs2));
            break;
          case INS32_OP_LSR_I:
            cpu_set_reg(rd, (cpu_unsigned_word_t)cpu_get_reg(rs1) >> uimm14);
            break;
          
          case INS32_OP_JMP:
            {
              vm_addr_t lnkpc = cpu_pc + INSN_LEN;
              cpu_set_reg(rd, lnkpc); /* link the returning address */
              cpu_pc = cpu_get_reg(rs1);
#ifdef TRACE_CALL_STACK
              if(rs1 == ADDR_RLNK)
                trace_call_stack_return(lnkpc - INSN_LEN, cpu_pc);
              else if(rd == ADDR_RLNK)
                trace_call_stack_jmp(lnkpc - INSN_LEN, lnkpc, cpu_pc);
#endif
              goto flush_pc;
            }
          
          case INS32_OP_JMP_I:
            {
              cpu_pc = cpu_pc + rel26_sig_ext(rel26);
#ifdef TRACE_CALL_STACK
              vm_addr_t lnkpc = cpu_pc + INSN_LEN;
              trace_call_stack_jmp(lnkpc - INSN_LEN, lnkpc, cpu_pc);
#endif
              goto flush_pc;
            }
          case INS32_OP_JMP_I_LNK:
            {
              vm_addr_t lnkpc = cpu_pc + INSN_LEN;
              cpu_set_reg(ADDR_RLNK, lnkpc); /* link the returning address */
              cpu_pc = cpu_pc + rel26_sig_ext(rel26);
#ifdef TRACE_CALL_STACK
              trace_call_stack_jmp(lnkpc - INSN_LEN, lnkpc, cpu_pc);
#endif
              goto flush_pc;
            }
            
          case INS32_OP_CMP:
            {
              switch(attr)
                {
                  case INS32_ATTR_CMPEQ:
                    msr.PSR.CC = ( cpu_get_reg(rs1) == cpu_get_reg(rs2) );
                    break;
                    
                  case INS32_ATTR_CMPGT:
                    msr.PSR.CC = ( cpu_get_reg(rs1) > cpu_get_reg(rs2) );
                    break;
                    
                  case INS32_ATTR_CMPGTU:
                    msr.PSR.CC = ( (cpu_unsigned_word_t)cpu_get_reg(rs1) > (cpu_unsigned_word_t)cpu_get_reg(rs2) );
                    break;
                  default:
                    cpu_raise_exception(VECT_EINSN, 0, 0);
                    goto handle_exception;
                }
              break;
            }
            
          case INS32_OP_BT:
            if( msr.PSR.CC )
              {
                cpu_pc = cpu_pc + rel26_sig_ext(rel26);
                goto flush_pc;
              }
            break;
            
          case INS32_OP_BF:
            if( !msr.PSR.CC )
              {
                cpu_pc = cpu_pc + rel26_sig_ext(rel26);
                goto flush_pc;
              }
            break;
            
          case INS32_OP_LDWU:
            {
              vm_addr_t va = cpu_get_reg(rs1) + (cpu_word_t)simm14;
              phy_addr_t pa = 0;
              if (dmmu_translate_vma(va, &pa, 0) < 0)
                {
                  goto handle_exception;
                }
#ifdef TRACE_STACK_POINTER
              if(rs1 == ADDR_SP) { trace_stack_pointer_ld(cpu_pc, va, phy_readm32(pa)); }
#endif
              cpu_set_reg(rd, (cpu_unsigned_word_t)phy_readm32(pa));
            }
            break;
            
          case INS32_OP_STW:
            {
              vm_addr_t va = cpu_get_reg(rd) + (cpu_word_t)simm14;
              phy_addr_t pa = 0;
              if (dmmu_translate_vma(va, &pa, 1) < 0)
                {
                  goto handle_exception;
                }
#ifdef TRACE_STACK_POINTER
              if(rs1 == ADDR_SP) { trace_stack_pointer_st(cpu_pc, va, cpu_get_reg(rd)); }
#endif
              phy_writem32(pa, (uint32_t)cpu_get_reg(rs1));
            }
            break;
            
          case INS32_OP_BARR:
            break;
          
          case INS32_OP_SYSCALL:
            cpu_raise_exception(VECT_ESYSCALL, 0, /*syscall*/1);
            goto handle_exception;
            
          case INS32_OP_RET:
            /* restore PSR and PC */
            msr.PSR = msr.EPSR;
            cpu_pc = msr.EPC;
            goto flush_pc;
            
          case INS32_OP_WMSR:
            wmsr(cpu_get_reg(rd) | uimm14, cpu_get_reg(rs1));
            break;
          case INS32_OP_RMSR:
            cpu_set_reg(rd, rmsr(cpu_get_reg(rs1) | uimm14));
            break;
            
          case INS32_OP_VENTER:
            break;
          case INS32_OP_VLEAVE:
            break;

          case INS32_OP_ASR:
            cpu_set_reg(rd, cpu_get_reg(rs1) >> cpu_get_reg(rs2));
            break;
          case INS32_OP_ASR_I:
            cpu_set_reg(rd, cpu_get_reg(rs1) >> uimm14);
            break;
            
          case INS32_OP_ADD:
            cpu_set_reg(rd, cpu_get_reg(rs1) + cpu_get_reg(rs2));
            break;
          case INS32_OP_ADD_I:
            cpu_set_reg(rd, cpu_get_reg(rs1) + (cpu_word_t)simm14);
            break;
          
          case INS32_OP_SUB:
            cpu_set_reg(rd, cpu_get_reg(rs1) - cpu_get_reg(rs2));
            break;
            
          case INS32_OP_MUL:
            cpu_set_reg(rd, cpu_get_reg(rs1) * cpu_get_reg(rs2));
            break;
            
          case INS32_OP_DIV:
          case INS32_OP_DIVU:
            cpu_set_reg(rd, cpu_get_reg(rs1) / cpu_get_reg(rs2));
            break;
            
          case INS32_OP_MOD:
          case INS32_OP_MODU:
            cpu_set_reg(rd, cpu_get_reg(rs1) % cpu_get_reg(rs2));
            break;
          
          case INS32_OP_LDB:
            {
              vm_addr_t va = cpu_get_reg(rs1) + (cpu_word_t)simm14;
              phy_addr_t pa = 0;
              if (dmmu_translate_vma(va, &pa, 0) < 0)
                {
                  goto handle_exception;
                }
              cpu_set_reg(rd, (cpu_word_t)phy_readm8(pa));
            }
            break;
          case INS32_OP_LDBU:
            {
              vm_addr_t va = cpu_get_reg(rs1) + (cpu_word_t)simm14;
              phy_addr_t pa = 0;
              if (dmmu_translate_vma(va, &pa, 0) < 0)
                {
                  goto handle_exception;
                }
              cpu_set_reg(rd, (cpu_unsigned_word_t)phy_readm8(pa));
            }
            break;
          case INS32_OP_LDH:
            {
              vm_addr_t va = cpu_get_reg(rs1) + (cpu_word_t)simm14;
              phy_addr_t pa = 0;
              if (dmmu_translate_vma(va, &pa, 0) < 0)
                {
                  goto handle_exception;
                }
              cpu_set_reg(rd, (cpu_word_t)phy_readm16(pa));
            }
            break;
          case INS32_OP_LDHU:
            {
              vm_addr_t va = cpu_get_reg(rs1) + (cpu_word_t)simm14;
              phy_addr_t pa = 0;
              if (dmmu_translate_vma(va, &pa, 0) < 0)
                {
                  goto handle_exception;
                }
              cpu_set_reg(rd, (cpu_unsigned_word_t)phy_readm16(pa));
            }
            break;
            
          case INS32_OP_STB:
            {
              vm_addr_t va = cpu_get_reg(rd) + (cpu_word_t)simm14;
              phy_addr_t pa = 0;
              if (dmmu_translate_vma(va, &pa, 1) < 0)
                {
                  goto handle_exception;
                }
              phy_writem8(pa, (uint8_t)cpu_get_reg(rs1));
            }
            break;
          case INS32_OP_STH:
            {
              vm_addr_t va = cpu_get_reg(rd) + (cpu_word_t)simm14;
              phy_addr_t pa = 0;
              if (dmmu_translate_vma(va, &pa, 1) < 0)
                {
                  goto handle_exception;
                }
              phy_writem16(pa, (uint16_t)cpu_get_reg(rs1));
            }
            break;
            
          case INS32_OP_MHI:
            cpu_set_reg(rd, (cpu_word_t)uimm18 << 14);
            break;
            
          default:
            cpu_raise_exception(VECT_EINSN, 0, 0);
            goto handle_exception;
        }
      
handle_exception:
      cpu_pc += INSN_LEN;
flush_pc:
      (void)0;
    }
}

/* must goto handle_exception after raised an exception */
void
cpu_raise_exception(vm_addr_t vector, vm_addr_t lsa, char syscall)
{
  msr.EPC = cpu_pc + (syscall ? INSN_LEN : 0);
  msr.ELSA = lsa;
  /* save old PSR */
  msr.EPSR = msr.PSR;
  /* set up new PSR for exception */
  msr.PSR.RM = 1;
  msr.PSR.IMME = 0;
  msr.PSR.DMME = 0;
  msr.PSR.IRE = 0;
  /* transfer to exception handler */
  /* -INSN_LEN will be eliminated by insn fetch in cpu_exec() */
  cpu_pc = (vm_signed_addr_t)vector -INSN_LEN;
  
#ifdef TRACE_EXCEPTION
  verbose_print_1("Raise Exception %X EPC=%x\n", vector, msr.EPC);
  getchar();
#endif
}
