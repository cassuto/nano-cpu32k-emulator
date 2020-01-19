
#include "ncpu32k-emu.h"
#include "ncpu32k-opcodes.h"

/*
 * Configurations
 */
#define TRACE_CALL_STACK
/* #undef TRACE_CALL_STACK */
//#define TRACE_STACK_POINTER
/* #undef TRACE_STACK_POINTER */
#define VERBOSE 0

/*
 * Global data variables
 */
static phy_addr_t cpu_pc = 0;
static struct regfile_s cpu_regfile;
static struct msr_s msr;
#ifdef TRACE_CALL_STACK
static int stack_depth = 0;
#endif

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
cpu_reset(phy_addr_t reset_vect)
{
  cpu_pc = reset_vect;

  memset(&cpu_regfile.r, sizeof(cpu_regfile.r), 0);
  memset(&msr, sizeof msr, 0);
}

/*
 * Debugging staffs
 */
#ifdef TRACE_CALL_STACK
static inline void
trace_call_stack_jmp(phy_addr_t insn_pc, phy_addr_t lnk_pc, phy_addr_t new_pc)
{
  verbose_print_1("%d# (%#x): call %x, ret=%x\n", ++stack_depth, insn_pc, new_pc, lnk_pc);
}

static inline void
trace_call_stack_return(phy_addr_t insn_pc, phy_addr_t new_pc)
{
  verbose_print_1("%d# (%#x): return to %x\n", --stack_depth, insn_pc, new_pc);
}
#endif

#ifdef TRACE_STACK_POINTER
static inline void
trace_stack_pointer_ld(phy_addr_t pc, phy_addr_t addr, cpu_word_t sp)
{
  verbose_print_1("(%#x): sp <- &%#x(%#x)\n", pc, addr, sp);
}

static inline void
trace_stack_pointer_mov(phy_addr_t pc, cpu_word_t sp)
{
  verbose_print_1("(%#x): sp <- %#x\n", pc, sp);
}

static inline void
trace_stack_pointer_st(phy_addr_t pc, phy_addr_t addr, cpu_word_t sp)
{
  verbose_print_1("(%#x): %#x <- sp(%#x)\n", pc, addr, sp);
}
#endif

void
memory_breakpoint(phy_addr_t addr, uint32_t val)
{
  verbose_print_1("memory BP (%#x): addr = %#x, val = %#x\n", cpu_pc, addr, val);
}


static inline void
cpu_set_reg(uint16_t addr, cpu_word_t val)
{
  if(addr >= 32)
  {
    fprintf(stderr, "cpu_set_reg() invalid register index at PC=%#x\n", cpu_pc);
    exit(1);
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
    exit(1);
  }
  if(addr)
    return cpu_regfile.r[addr];
  else
    return 0;
}

static inline phy_signed_addr_t
rel20_sig_ext(uint32_t rel20)
{
  if(rel20 & (1 << 19))
  {
    return 0xfff00000 | (rel20 <<= INSN_LEN_SHIFT);
  }
  else
    return rel20 << INSN_LEN_SHIFT;
}

int
cpu_exec(void)
{
  phy_addr_t last_pc = 0;
  for(;;)
    {
      //printf("%X\n", cpu_pc);
      insn_t current_ins = (insn_t)readm32(cpu_pc);
      uint16_t opcode = current_ins & INS32_MASK_OPCODE;
      
      uint16_t rs1 = INS32_GET_BITS(current_ins, RS1);
      uint16_t rs2 = INS32_GET_BITS(current_ins, RS2);
      uint16_t rd = INS32_GET_BITS(current_ins, RD);
      uint32_t uimm18 = INS32_GET_BITS(current_ins, IMM18);
      uint16_t uimm14 = INS32_GET_BITS(current_ins, IMM14);
      int16_t simm14 = (((int16_t)uimm14) ^ 0x2000) - 0x2000; /* sign extend */
      uint32_t rel20 = INS32_GET_BITS(current_ins, REL20);
      uint8_t attr = INS32_GET_BITS(current_ins, ATTR);
      
      if(cpu_pc == 0)
        verbose_print("pc = %#x\n", last_pc);

      last_pc = cpu_pc;

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
              phy_addr_t lnkpc = cpu_pc + INSN_LEN;
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
              phy_addr_t lnkpc = cpu_pc + INSN_LEN;
              cpu_set_reg(rd, lnkpc); /* link the returning address */
              cpu_pc = cpu_pc + rel20_sig_ext(rel20);
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
                    msr.psr.cc = ( cpu_get_reg(rs1) == cpu_get_reg(rs2) );
                    break;
                    
                  case INS32_ATTR_CMPGT:
                    msr.psr.cc = ( cpu_get_reg(rs1) > cpu_get_reg(rs2) );
                    break;
                    
                  case INS32_ATTR_CMPGTU:
                    msr.psr.cc = ( (cpu_unsigned_word_t)cpu_get_reg(rs1) > (cpu_unsigned_word_t)cpu_get_reg(rs2) );
                    break;
                  default:
                    cpu_raise_excp(VECT_EINSN);
                    break;
                }
              break;
            }
            
          case INS32_OP_BT:
            if( msr.psr.cc )
              {
                cpu_pc = cpu_pc + rel20_sig_ext(rel20);
                goto flush_pc;
              }
            break;
            
          case INS32_OP_BF:
            if( !msr.psr.cc )
              {
                cpu_pc = cpu_pc + rel20_sig_ext(rel20);
                goto flush_pc;
              }
            break;
            
          case INS32_OP_LDWU:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm14;
#ifdef TRACE_STACK_POINTER
              if(rs1 == ADDR_SP) { trace_stack_pointer_ld(cpu_pc, addr, readm32(addr)); }
#endif
              cpu_set_reg(rd, (cpu_unsigned_word_t)readm32(addr));
            }
            break;
            
          case INS32_OP_STW:
            {
              phy_addr_t addr = cpu_get_reg(rd) + (cpu_word_t)simm14;
#ifdef TRACE_STACK_POINTER
              if(rs1 == ADDR_SP) { trace_stack_pointer_st(cpu_pc, addr, cpu_get_reg(rd)); }
#endif
              writem32(addr, (uint32_t)cpu_get_reg(rs1));
            }
            break;
            
          case INS32_OP_BARR:
            break;
          
          case INS32_OP_RAISE:
            cpu_raise_excp(uimm14);
            break;
            
          case INS32_OP_RET:
            break;
            
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
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm14;
              cpu_set_reg(rd, (cpu_word_t)readm8(addr));
            }
            break;
          case INS32_OP_LDBU:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm14;
              cpu_set_reg(rd, (cpu_unsigned_word_t)readm8(addr));
            }
            break;
          case INS32_OP_LDH:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm14;
              cpu_set_reg(rd, (cpu_word_t)readm16(addr));
            }
            break;
          case INS32_OP_LDHU:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm14;
              cpu_set_reg(rd, (cpu_unsigned_word_t)readm16(addr));
            }
            break;
            
          case INS32_OP_STB:
            {
              phy_addr_t addr = cpu_get_reg(rd) + (cpu_word_t)simm14;
              writem8(addr, (uint8_t)cpu_get_reg(rs1));
            }
            break;
          case INS32_OP_STH:
            {
              phy_addr_t addr = cpu_get_reg(rd) + (cpu_word_t)simm14;
              writem16(addr, (uint16_t)cpu_get_reg(rs1));
            }
            break;
            
          case INS32_OP_MHI:
            cpu_set_reg(rd, (cpu_word_t)uimm18 << 14);
            break;
            
          default:
            cpu_raise_excp(VECT_EINSN);
        }
      
      cpu_pc += INSN_LEN;
flush_pc:
      (void)0;
    }
}

void
cpu_raise_excp(int excp_no)
{
  printf("excp %d at PC=0x%X\n", excp_no, cpu_pc);
}
