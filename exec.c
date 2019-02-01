
#include "openpx64k-emu.h"
#include "openpx64k-opcodes.h"

/*
 * Configurations
 */
#define TRACE_CALL_STACK
/* #undef TRACE_CALL_STACK */
#define TRACE_STACK_POINTER
/* #undef TRACE_STACK_POINTER */
#define VERBOSE 0

/*
 * Global data variables
 */
static phy_addr_t cpu_pc = 0;
static struct regfile_s cpu_regfile;
static struct csmr_s csmr;
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
  memset(&csmr, sizeof csmr, 0);
}

/*
 * Debugging staffs
 */
#ifdef TRACE_CALL_STACK
static inline void
trace_call_stack_jmp(phy_addr_t call_pc, phy_addr_t ori_pc, phy_addr_t new_pc)
{
  verbose_print_1("%d# (%#x): call %x, ret=%x\n", ++stack_depth, call_pc, ori_pc, new_pc);
}

static inline void
trace_call_stack_return(phy_addr_t call_pc, phy_addr_t new_pc)
{
  verbose_print_1("%d# (%#x): return to %x\n", --stack_depth, cpu_pc, new_pc);
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
  cpu_regfile.r[addr] = val;
#ifdef TRACE_CALL_STACK
  if(addr == 31) trace_stack_pointer_mov(cpu_pc, val);
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
  return cpu_regfile.r[addr];
}

static inline phy_signed_addr_t
rel26_sig_ext(uint32_t rel26)
{
  if(rel26 & (1 << 25))
  {
    return 0xf0000000 | (rel26 <<= INSN_LEN_SHIFT);
  }
  else
    return rel26 << INSN_LEN_SHIFT;
}

int
cpu_exec(void)
{
  phy_addr_t last_pc = 0;
  for(;;)
    {
      insn_t current_ins = (insn_t)readm32(cpu_pc);
      uint16_t opcode = current_ins & INS32_MASK_OPCODE;
      
      uint16_t aluopc = INS32_GET_BITS(current_ins, ALU_OPC);
      uint16_t rs1 = INS32_GET_BITS(current_ins, RS1);
      uint16_t rs2 = INS32_GET_BITS(current_ins, RS2);
      uint16_t rd = INS32_GET_BITS(current_ins, RD);
      uint16_t uimm16 = INS32_GET_BITS(current_ins, IMM16);
      int16_t simm16 = (int16_t)uimm16;
      uint32_t rel26 = INS32_GET_BITS(current_ins, REL26);
      
      if(cpu_pc == 0)
        verbose_print("pc = %#x\n", last_pc);

      last_pc = cpu_pc;

      switch( opcode )
        {
          case INS32_OP_GRPSI:
            {
              switch( aluopc )
                {
                  case INS32_SUBOP_add:
                    cpu_set_reg(rd, cpu_get_reg(rs1) + cpu_get_reg(rs2));
                    break;
                    
                  case INS32_SUBOP_sub:
                    cpu_set_reg(rd, cpu_get_reg(rs1) - cpu_get_reg(rs2));
                    break;
                    
                  case INS32_SUBOP_cmpeq:
                    csmr.psr.cf = (cpu_get_reg(rs1) == cpu_get_reg(rs2));
                    break;
                  case INS32_SUBOP_cmpgt:
                    csmr.psr.cf = (cpu_get_reg(rs1) > cpu_get_reg(rs2));
                    break;
                  case INS32_SUBOP_cmpgt_u:
                    csmr.psr.cf = ( (cpu_unsigned_word_t)(cpu_get_reg(rs1)) > (cpu_unsigned_word_t)(cpu_get_reg(rs2)) );
                    break;
                    
                  default:
                    cpu_raise_excp(VECT_EINSN);
                }
            }
            break;
            
          case INS32_OP_GRPMI:
            {
              switch( aluopc )
                {
                  case INS32_SUBOP_mul:
                  case INS32_SUBOP_mul_u:
                    cpu_set_reg(rd, cpu_get_reg(rs1) * cpu_get_reg(rs2));
                    break;
                  case INS32_SUBOP_div:
                  case INS32_SUBOP_div_u:
                    cpu_set_reg(rd, cpu_get_reg(rs1) / cpu_get_reg(rs2));
                    break;
                    
                  default:
                    cpu_raise_excp(VECT_EINSN);
                }
            }
            break;
            
          case INS32_OP_GRPL:
            {
              switch( aluopc )
                {
                  case INS32_SUBOP_and:
                    cpu_set_reg(rd, cpu_get_reg(rs1) & cpu_get_reg(rs2));
                    break;
                    
                  case INS32_SUBOP_or:
                    cpu_set_reg(rd, cpu_get_reg(rs1) | cpu_get_reg(rs2));
                    break;
                    
                  case INS32_SUBOP_xor:
                    cpu_set_reg(rd, cpu_get_reg(rs1) ^ cpu_get_reg(rs2));
                    break;
                    
                  case INS32_SUBOP_lsl:
                    cpu_set_reg(rd, cpu_get_reg(rs1) << cpu_get_reg(rs2));
                    break;
                    
                  case INS32_SUBOP_lsr:
                    cpu_set_reg(rd, (cpu_unsigned_word_t)cpu_get_reg(rs1) >> cpu_get_reg(rs2));
                    break;
                  
                  case INS32_SUBOP_asr:
                    cpu_set_reg(rd, cpu_get_reg(rs1) >> cpu_get_reg(rs2));
                    break;
                    
                  default:
                    cpu_raise_excp(VECT_EINSN);
                }
            }
            break;
            
          case INS32_OP_GRPT:
            {
              switch( aluopc )
                {
                  case INS32_SUBOP_cmov:
                    cpu_set_reg(rd, cpu_regfile.r[csmr.psr.cf ? rs1 : rs2]);
                    break;
                    
                  default:
                    cpu_raise_excp(VECT_EINSN);
                }
            }
            break;
            
          case INS32_OP_GRPJ:
            {
              switch( aluopc )
                {
                  case INS32_SUBOP_jmp:
                    {
                      phy_addr_t new_pc = cpu_get_reg(rs1);

#ifdef TRACE_CALL_STACK
                      if(rs1 == ADDR_RLNK) { trace_call_stack_return(cpu_pc, new_pc); }
#endif
                      cpu_pc = new_pc;
                      goto flush_pc;
                    }
                    
                  case INS32_SUBOP_jmpl:
                    {
                      phy_addr_t oripc = cpu_pc + INSN_LEN;
                      cpu_set_reg(ADDR_RLNK, oripc); /* link the returning address */
                      cpu_pc = cpu_get_reg(rs1);
#ifdef TRACE_CALL_STACK
                      trace_call_stack_jmp(oripc - INSN_LEN, oripc, cpu_pc);
#endif
                      goto flush_pc;
                    }
                    
                  default:
                    cpu_raise_excp(VECT_EINSN);
                }
            }
            break;
            
          case INS32_OP_add_i:
            cpu_set_reg(rd, cpu_get_reg(rs1) + (cpu_word_t)simm16);
            break;
            
          case INS32_OP_and_i:
            cpu_set_reg(rd, cpu_get_reg(rs1) & uimm16);
            break;
          
          case INS32_OP_or_i:
            cpu_set_reg(rd, cpu_get_reg(rs1) | uimm16);
            break;
            
          case INS32_OP_xor_i:
            cpu_set_reg(rd, cpu_get_reg(rs1) ^ simm16);
            break;
            
          case INS32_OP_lsl_i:
            cpu_set_reg(rd, cpu_get_reg(rs1) << uimm16);
            break;
            
          case INS32_OP_lsr_i:
            cpu_set_reg(rd, (cpu_unsigned_word_t)cpu_get_reg(rs1) >> uimm16);
            break;
          
          case INS32_OP_asr_i:
            cpu_set_reg(rd, cpu_get_reg(rs1) >> uimm16);
            break;
            
          case INS32_OP_cmp_i:
            {
              uint16_t subopc = INS32_GET_BITS(current_ins, SUB_OPC);
              
              switch(subopc)
                {
                  case INS32_OP_cmpeq_i:
                    csmr.psr.cf = ( cpu_get_reg(rs1) == (cpu_word_t)simm16 );
                    break;
                    
                  case INS32_OP_cmpgt_i:
                    csmr.psr.cf = ( cpu_get_reg(rs1) > (cpu_word_t)simm16 );
                    break;
                    
                  case INS32_OP_cmpgt_ui:
                    csmr.psr.cf = ( (cpu_unsigned_word_t)(cpu_get_reg(rs1)) > (cpu_unsigned_word_t)simm16 );
                    break;
                    
                  case INS32_OP_cmplt_i:
                    csmr.psr.cf = ( cpu_get_reg(rs1) < (cpu_word_t)simm16 );
                    break;
                    
                  case INS32_OP_cmplt_ui:
                    csmr.psr.cf = ( cpu_get_reg(rs1) < (cpu_unsigned_word_t)simm16 );
                    break;
                    
                  default:
                    cpu_raise_excp(VECT_EINSN);
                }
            }
            break;
            
          case INS32_OP_movh:
            cpu_set_reg(rd, (cpu_word_t)uimm16 << 16);
            break;
            
          case INS32_OP_rdsmr:
            break;
          case INS32_OP_wrsmr:
            break;
            
          case INS32_OP_ldb:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm16;
              cpu_set_reg(rd, (cpu_word_t)readm8(addr));
            }
            break;
          case INS32_OP_ldb_u:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm16;
              cpu_set_reg(rd, (cpu_unsigned_word_t)readm8(addr));
            }
            break;
          case INS32_OP_ldh:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm16;
              cpu_set_reg(rd, (cpu_word_t)readm16(addr));
            }
            break;
          case INS32_OP_ldh_u:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm16;
              cpu_set_reg(rd, (cpu_unsigned_word_t)readm16(addr));
            }
            break;
          case INS32_OP_ldw:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm16;
#ifdef TRACE_STACK_POINTER
              if(rs1 == 31) { trace_stack_pointer_ld(cpu_pc, addr, readm32(addr)); }
#endif
              cpu_set_reg(rd, (cpu_word_t)readm32(addr));
            }
            break;
          case INS32_OP_ldw_u:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm16;
#ifdef TRACE_STACK_POINTER
              if(rs1 == 31) { trace_stack_pointer_ld(cpu_pc, addr, readm32(addr)); }
#endif
              cpu_set_reg(rd, (cpu_unsigned_word_t)readm32(addr));
            }
            break;
          case INS32_OP_ldwa:
            break;
            
          case INS32_OP_stb:
            {
              phy_addr_t addr = cpu_get_reg(rd) + (cpu_word_t)simm16;
              writem8(addr, (uint8_t)cpu_get_reg(rs1));
            }
            break;
          case INS32_OP_sth:
            {
              phy_addr_t addr = cpu_get_reg(rd) + (cpu_word_t)simm16;
              writem16(addr, (uint16_t)cpu_get_reg(rs1));
            }
            break;
          case INS32_OP_stw:
            {
              phy_addr_t addr = cpu_get_reg(rd) + (cpu_word_t)simm16;
#ifdef TRACE_STACK_POINTER
              if(rs1 == 31) { trace_stack_pointer_st(cpu_pc, addr, cpu_get_reg(rd)); }
#endif
              writem32(addr, (uint32_t)cpu_get_reg(rs1));
            }
            break;
            
          case INS32_OP_stwa:
            break;
            
          case INS32_OP_jmps:
            cpu_pc = cpu_pc + rel26_sig_ext(rel26);
            goto flush_pc;
            
          case INS32_OP_jmpsl:
            {
              phy_addr_t oripc = cpu_pc + INSN_LEN;
              cpu_set_reg(ADDR_RLNK, oripc); /* link the returning address */
              cpu_pc = cpu_pc + rel26_sig_ext(rel26);
#ifdef TRACE_CALL_STACK
              trace_call_stack_jmp(oripc - INSN_LEN, oripc, cpu_pc);
#endif
              goto flush_pc;
            }
            
          case INS32_OP_bnct:
            if( csmr.psr.cf )
              {
                cpu_pc = cpu_pc + rel26_sig_ext(rel26);
                goto flush_pc;
              }
            break;
            
          case INS32_OP_bncf:
            if( !csmr.psr.cf )
              {
                cpu_pc = cpu_pc + rel26_sig_ext(rel26);
                goto flush_pc;
              }

            break;
            
          case INS32_OP_resume:
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
}
