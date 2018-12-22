
#include "openpx64k-emu.h"
#include "openpx64k-opcodes.h"

static phy_addr_t cpu_pc = 0;
static struct regfile_s cpu_regfile;
static struct csmr_s csmr;

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
      uint16_t imm16 = INS32_GET_BITS(current_ins, IMM16);
      uint16_t rel26 = INS32_GET_BITS(current_ins, REL26);
      
      switch( opcode )
        {
          case INS32_OP_GRPSI:
            {
              switch( aluopc )
                {
                  case INS32_SUBOP_add:
                    cpu_regfile.r[rd] = cpu_regfile.r[rs1] + cpu_regfile.r[rs2];
                    break;
                    
                  case INS32_SUBOP_sub:
                    cpu_regfile.r[rd] = cpu_regfile.r[rs1] - cpu_regfile.r[rs2];
                    break;
                    
                  case INS32_SUBOP_cmpeq:
                    csmr.psr.cf = (cpu_regfile.r[rs1] == cpu_regfile.r[rs2]);
                    break;
                  case INS32_SUBOP_cmpgt:
                    csmr.psr.cf = (cpu_regfile.r[rs1] > cpu_regfile.r[rs2]);
                    break;
                  case INS32_SUBOP_cmpgt_u:
                    csmr.psr.cf = ( (cpu_unsigned_word_t)(cpu_regfile.r[rs1]) > (cpu_unsigned_word_t)(cpu_regfile.r[rs2]) );
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
                    cpu_regfile.r[rd] = cpu_regfile.r[rs1] - cpu_regfile.r[rs2];
                    break;
                  case INS32_SUBOP_div:
                  case INS32_SUBOP_div_u:
                    cpu_regfile.r[rd] = cpu_regfile.r[rs1] / cpu_regfile.r[rs2];
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
                    cpu_regfile.r[rd] = cpu_regfile.r[rs1] & cpu_regfile.r[rs2];
                    break;
                    
                  case INS32_SUBOP_or:
                    cpu_regfile.r[rd] = cpu_regfile.r[rs1] | cpu_regfile.r[rs2];
                    break;
                    
                  case INS32_SUBOP_xor:
                    cpu_regfile.r[rd] = cpu_regfile.r[rs1] ^ cpu_regfile.r[rs2];
                    break;
                    
                  case INS32_SUBOP_lsl:
                    cpu_regfile.r[rd] = cpu_regfile.r[rs1] << cpu_regfile.r[rs2];
                    break;
                    
                  case INS32_SUBOP_lsr:
                    cpu_regfile.r[rd] = (cpu_unsigned_word_t)cpu_regfile.r[rs1] >> cpu_regfile.r[rs2];
                    break;
                  
                  case INS32_SUBOP_asr:
                    cpu_regfile.r[rd] = cpu_regfile.r[rs1] >> cpu_regfile.r[rs2];
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
                  case INS32_SUBOP_mov:
                    cpu_regfile.r[rd] = cpu_regfile.r[rs1];
                    break;
                    
                  case INS32_SUBOP_cmov:
                    cpu_regfile.r[rd] = cpu_regfile.r[csmr.psr.cf ? rs1 : rs2];
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
                    cpu_pc = cpu_regfile.r[rs1];
                    goto flush_pc;
                    
                  case INS32_SUBOP_jmpl:
                    cpu_pc = cpu_regfile.r[rs1];
                    cpu_regfile.r[ADDR_RLNK] = cpu_pc; /* link the returning address */
                    break;
                    
                  default:
                    cpu_raise_excp(VECT_EINSN);
                }
            }
            break;
            
          case INS32_OP_add_i:
            cpu_regfile.r[rd] = cpu_regfile.r[rs1] + (cpu_word_t)imm16;
            break;
            
          case INS32_OP_and_i:
            cpu_regfile.r[rd] = cpu_regfile.r[rs1] & imm16;
            break;
          
          case INS32_OP_or_i:
            cpu_regfile.r[rd] = cpu_regfile.r[rs1] | imm16;
            break;
            
          case INS32_OP_xor_i:
            cpu_regfile.r[rd] = cpu_regfile.r[rs1] ^ imm16;
            break;
            
          case INS32_OP_lsl_i:
            cpu_regfile.r[rd] = cpu_regfile.r[rs1] << imm16;
            break;
            
          case INS32_OP_lsr_i:
            cpu_regfile.r[rd] = (cpu_unsigned_word_t)cpu_regfile.r[rs1] >> imm16;
            break;
          
          case INS32_OP_asr_i:
            cpu_regfile.r[rd] = cpu_regfile.r[rs1] >> imm16;
            break;
            
          case INS32_OP_cmp_i:
            {
              uint16_t subopc = INS32_GET_BITS(current_ins, SUB_OPC);
              
              switch(subopc)
                {
                  case INS32_OP_cmpeq_i:
                    csmr.psr.cf = ( cpu_regfile.r[rs1] == (cpu_word_t)imm16 );
                    break;
                    
                  case INS32_OP_cmpgt_i:
                    csmr.psr.cf = ( cpu_regfile.r[rs1] > (cpu_word_t)imm16 );
                    break;
                    
                  case INS32_OP_cmpgt_ui:
                    csmr.psr.cf = ( (cpu_unsigned_word_t)(cpu_regfile.r[rs1]) > (cpu_unsigned_word_t)imm16 );
                    break;
                    
                  case INS32_OP_cmplt_i:
                    csmr.psr.cf = ( cpu_regfile.r[rs1] < (cpu_word_t)imm16 );
                    break;
                    
                  case INS32_OP_cmplt_ui:
                    csmr.psr.cf = ( cpu_regfile.r[rs1] < (cpu_unsigned_word_t)imm16 );
                    break;
                    
                  default:
                    cpu_raise_excp(VECT_EINSN);
                }
            }
            break;
            
          case INS32_OP_movh:
            cpu_regfile.r[rd] = cpu_regfile.r[rs1] | ((cpu_word_t)imm16 << 16);
            break;
            
          case INS32_OP_rdsmr:
            break;
          case INS32_OP_wrsmr:
            break;
            
          case INS32_OP_ldb:
            {
              phy_addr_t addr = cpu_regfile.r[rs1] + (cpu_word_t)imm16;
              cpu_regfile.r[rd] = (cpu_word_t)readm8(addr);
            }
            break;
          case INS32_OP_ldb_u:
            {
              phy_addr_t addr = cpu_regfile.r[rs1] + (cpu_word_t)imm16;
              cpu_regfile.r[rd] = (cpu_unsigned_word_t)readm8(addr);
            }
            break;
          case INS32_OP_ldh:
            {
              phy_addr_t addr = cpu_regfile.r[rs1] + (cpu_word_t)imm16;
              cpu_regfile.r[rd] = (cpu_word_t)readm16(addr);
            }
            break;
          case INS32_OP_ldh_u:
            {
              phy_addr_t addr = cpu_regfile.r[rs1] + (cpu_word_t)imm16;
              cpu_regfile.r[rd] = (cpu_unsigned_word_t)readm16(addr);
            }
            break;
          case INS32_OP_ldw:
            {
              phy_addr_t addr = cpu_regfile.r[rs1] + (cpu_word_t)imm16;
              cpu_regfile.r[rd] = (cpu_word_t)readm32(addr);
            }
            break;
          case INS32_OP_ldw_u:
            {
              phy_addr_t addr = cpu_regfile.r[rs1] + (cpu_word_t)imm16;
              cpu_regfile.r[rd] = (cpu_unsigned_word_t)readm32(addr);
            }
            break;
          case INS32_OP_ldwa:
            break;
            
          case INS32_OP_stb:
            {
              phy_addr_t addr = cpu_regfile.r[rd] + (cpu_word_t)imm16;
              writem8(addr, (uint8_t)cpu_regfile.r[rs1]);
            }
            break;
          case INS32_OP_sth:
            {
              phy_addr_t addr = cpu_regfile.r[rd] + (cpu_word_t)imm16;
              writem16(addr, (uint16_t)cpu_regfile.r[rs1]);
            }
            break;
          case INS32_OP_stw:
            {
              phy_addr_t addr = cpu_regfile.r[rd] + (cpu_word_t)imm16;
              writem32(addr, (uint32_t)cpu_regfile.r[rs1]);
            }
            break;
            
          case INS32_OP_stwa:
            break;
            
          case INS32_OP_jmps:
            cpu_pc = cpu_pc + ((phy_addr_t)rel26 << INSN_LEN);
            goto flush_pc;
            
          case INS32_OP_jmpsl:
            cpu_pc = cpu_pc + ((phy_addr_t)rel26 << INSN_LEN);
            cpu_regfile.r[ADDR_RLNK] = cpu_pc; /* link the returning address */
            goto flush_pc;
            
          case INS32_OP_bnct:
            if( csmr.psr.cf )
              {
                cpu_pc = cpu_pc + ((phy_addr_t)rel26 << INSN_LEN);
                goto flush_pc;
              }
            break;
            
          case INS32_OP_bncf:
            if( !csmr.psr.cf )
              {
                cpu_pc = cpu_pc + ((phy_addr_t)rel26 << INSN_LEN);
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
