#ifndef OPENPX64K_OPCODES_H
#define OPENPX64K_OPCODES_H


#define INS32_COND_NO 0
#define INS32_COND_EQ 1
#define INS32_COND_NE 2
#define INS32_COND_GE 3
#define INS32_COND_GT 4
#define INS32_COND_LE 5
#define INS32_COND_LT 6
#define INS32_OP_GRPSI 1  // in opcodes.cc: line 45. GRPSI
#define INS32_OP_GRPMI 2  // in opcodes.cc: line 128. GRPMI
#define INS32_OP_GRPL 3  // in opcodes.cc: line 196. GRPL
#define INS32_OP_GRPT 4  // in opcodes.cc: line 294. GRPT
#define INS32_OP_GRPJ 5  // in opcodes.cc: line 336. GRPJ
#define INS32_SUBOP_add 0  // in opcodes.cc: line 54. 
#define INS32_SUBOP_sub 1  // in opcodes.cc: line 68. 
#define INS32_SUBOP_cmpeq 2  // in opcodes.cc: line 83. 
#define INS32_SUBOP_cmpgt 3  // in opcodes.cc: line 97. 
#define INS32_SUBOP_cmpgt_u 4  // in opcodes.cc: line 111. 
#define INS32_SUBOP_mul 0  // in opcodes.cc: line 137. 
#define INS32_SUBOP_mul_u 1  // in opcodes.cc: line 151. 
#define INS32_SUBOP_div 2  // in opcodes.cc: line 165. 
#define INS32_SUBOP_div_u 3  // in opcodes.cc: line 179. 
#define INS32_SUBOP_and 0  // in opcodes.cc: line 205. 
#define INS32_SUBOP_or 1  // in opcodes.cc: line 219. 
#define INS32_SUBOP_xor 2  // in opcodes.cc: line 233. 
#define INS32_SUBOP_lsl 3  // in opcodes.cc: line 248. 
#define INS32_SUBOP_lsr 4  // in opcodes.cc: line 262. 
#define INS32_SUBOP_asr 5  // in opcodes.cc: line 276. 
#define INS32_SUBOP_cmov 0  // in opcodes.cc: line 317. 
#define INS32_SUBOP_jmp 0  // in opcodes.cc: line 345. 
#define INS32_SUBOP_jmpl 1  // in opcodes.cc: line 359. 
#define INS32_OP_add_i 10  // in opcodes.cc: line 380. 
#define INS32_OP_and_i 11  // in opcodes.cc: line 393. 
#define INS32_OP_or_i 12  // in opcodes.cc: line 406. 
#define INS32_OP_xor_i 13  // in opcodes.cc: line 419. 
#define INS32_OP_lsl_i 14  // in opcodes.cc: line 432. 
#define INS32_OP_lsr_i 15  // in opcodes.cc: line 445. 
#define INS32_OP_asr_i 16  // in opcodes.cc: line 458. 
#define INS32_OP_cmp_i 17  // in opcodes.cc: line 471. 
#define INS32_OP_movh 18  // in opcodes.cc: line 484. 
#define INS32_OP_rdsmr 19  // in opcodes.cc: line 497. 
#define INS32_OP_wrsmr 20  // in opcodes.cc: line 510. 
#define INS32_OP_ldb 25  // in opcodes.cc: line 523. 
#define INS32_OP_ldb_u 26  // in opcodes.cc: line 536. 
#define INS32_OP_ldh 27  // in opcodes.cc: line 549. 
#define INS32_OP_ldh_u 28  // in opcodes.cc: line 562. 
#define INS32_OP_ldw 29  // in opcodes.cc: line 575. 
#define INS32_OP_ldw_u 30  // in opcodes.cc: line 588.
#define INS32_OP_ldwa 31  // in opcodes.cc: line 601. 
#define INS32_OP_stb 32  // in opcodes.cc: line 614. 
#define INS32_OP_sth 33  // in opcodes.cc: line 627. 
#define INS32_OP_stw 34  // in opcodes.cc: line 640. 
#define INS32_OP_stwa 35  // in opcodes.cc: line 653. 
#define INS32_OP_syscall 36  // in opcodes.cc: line 666. 
#define INS32_OP_trap 37  // in opcodes.cc: line 679. 
#define INS32_OP_msync 38  // in opcodes.cc: line 692. 
#define INS32_OP_resume 39  // in opcodes.cc: line 705. 
#define INS32_OP_jmps 21  // in opcodes.cc: line 723. 
#define INS32_OP_jmpsl 22  // in opcodes.cc: line 736. 
#define INS32_OP_bnct 23  // in opcodes.cc: line 749. 
#define INS32_OP_bncf 24  // in opcodes.cc: line 762. 
#define INS32_OP_cmpeq_i 2  // in opcodes.cc: line 479. 
#define INS32_OP_cmpgt_i 3  // in opcodes.cc: line 492. 
#define INS32_OP_cmpgt_ui 4  // in opcodes.cc: line 505. 
#define INS32_OP_cmplt_i 5  // in opcodes.cc: line 518. 
#define INS32_OP_cmplt_ui 6  // in opcodes.cc: line 531. 

#define INS32_MASK_OPCODE   0x0000003f
#define INS32_MASK_RD       0x000007c0
#define INS32_MASK_RS1      0x0000f800
#define INS32_MASK_ALU_OPC  0x003f0000
#define INS32_MASK_RS2      0x07ffffff
#define INS32_MASK_SUB_OPC INS32_MASK_RD
#define INS32_MASK_IMM16    0xffff0000
#define INS32_MASK_REL26    0xffffffc0

#define INS32_SHIFT_OPCODE   0
#define INS32_SHIFT_RD       6
#define INS32_SHIFT_RS1      6+5
#define INS32_SHIFT_ALU_OPC  6+5+5
#define INS32_SHIFT_RS2      6+5+5+6
#define INS32_SHIFT_SUB_OPC INS32_SHIFT_RD
#define INS32_SHIFT_IMM16    16
#define INS32_SHIFT_REL26    6

#define INS32_GET_BITS(src, opc) ((uint32_t)(src & INS32_MASK_ ## opc) >> (INS32_SHIFT_ ## opc))

#define VECT_ERST           0x00000000
#define VECT_EBUS           0x00000010
#define VECT_EINSN          0x00000020
#define VECT_ESYSCALL       0x00000030
#define VECT_EIPF           0x00000040
#define VECT_EDPF           0x00000050
#define VECT_EITLB          0x00000060
#define VECT_EDTLB          0x00000070
#define VECT_EIRQ           0x00000080
#define VECT_EOVERFLOW      0x00000090
#define VECT_EFPU           0x000000a0
#define VECT_ETRAP          0x000000b0
#define VECT_EDALIGN        0x000000c0

#define INSN_LEN            4
#define INSN_LEN_SHIFT      2

#define ADDR_RLNK           0x1d        /* LNK register */

#endif
