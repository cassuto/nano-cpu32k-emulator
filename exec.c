
#include "openpx64k-emu.h"
#include "openpx64k-opcodes.h"

static phy_addr_t cpu_pc = 0;
static struct regfile_s cpu_regfile;
static struct csmr_s csmr;

static char verbose = 0;

#define verbose_print(...) do { if(verbose) printf(__VA_ARGS__); } while(0)

int
cpu_exec_init(int memory_size)
{
  if( !(cpu_memory = calloc(memory_size, 1)) )
    return -EM_NO_MEMORY;
  return 0;
}

void
cpu_reset(void)
{
  cpu_pc = VECT_ERST;
  memset(&cpu_regfile.r, sizeof(cpu_regfile.r), 0);
  memset(&csmr, sizeof csmr, 0);
}

static inline void
cpu_set_reg(uint16_t addr, cpu_word_t val)
{
  if(addr >= 32)
  {
    fprintf(stderr, "cpu_set_reg() invalid register index at PC=%#x\n", cpu_pc);
    exit(1);
  }
  verbose_print("set %d <- %d\n", addr, val);
  cpu_regfile.r[addr] = val;
  if(addr == 31) verbose_print("sp(%x) ==%x\n", cpu_pc, val);
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
      
      if(cpu_pc==8) verbose_print("%d %d\n", opcode, aluopc);
      if(cpu_pc < 0x330 && 1)
        {
          verbose_print("PC=%#x\n", cpu_pc);
          //printf("PC=%#x\n", cpu_pc);
        }

      if(cpu_pc==0x418)
        verbose_print("va=%x\n", cpu_get_reg(1));
        
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
                    if(rs1 == ADDR_RLNK) { verbose_print("return %x %x\n", cpu_pc, cpu_get_reg(rs1)); }

                    cpu_pc = cpu_get_reg(rs1);
                    goto flush_pc;
                    
                  case INS32_SUBOP_jmpl:
                    cpu_set_reg(ADDR_RLNK, cpu_pc + INSN_LEN); /* link the returning address */
                    cpu_pc = cpu_get_reg(rs1);
                    break;
                    
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
              verbose_print("ld(%x) %d <- &%x\n", cpu_pc, rd, addr );
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
              cpu_set_reg(rd, (cpu_word_t)readm32(addr));
            }
            break;
          case INS32_OP_ldw_u:
            {
              phy_addr_t addr = cpu_get_reg(rs1) + (cpu_word_t)simm16;
              cpu_set_reg(rd, (cpu_unsigned_word_t)readm32(addr));
              verbose_print("ld(%x) %d <- &%x(%d)\n", cpu_pc, rd, addr, (cpu_unsigned_word_t)readm32(addr) );
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
              writem32(addr, (uint32_t)cpu_get_reg(rs1));
              verbose_print("stw(%x) %d(%d) -> &%x\n", cpu_pc, rs1, cpu_get_reg(rs1), addr );
            }
            break;
            
          case INS32_OP_stwa:
            break;
            
          case INS32_OP_jmps:
            cpu_pc = cpu_pc + rel26_sig_ext(rel26);
            goto flush_pc;
            
          case INS32_OP_jmpsl:
            {
            int oripc = cpu_pc + INSN_LEN;
            cpu_set_reg(ADDR_RLNK, cpu_pc + INSN_LEN); /* link the returning address */
            cpu_pc = cpu_pc + rel26_sig_ext(rel26);
            verbose_print("call %x ret=%x, new=%x", oripc - INSN_LEN, oripc, cpu_pc);
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
