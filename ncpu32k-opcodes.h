#ifndef NCPU32K_OPCODES_H
#define NCPU32K_OPCODES_H

#define INS32_OP_AND    0x0
#define INS32_OP_AND_I  0x1
#define INS32_OP_OR     0x2
#define INS32_OP_OR_I   0x3
#define INS32_OP_XOR    0x4
#define INS32_OP_XOR_I  0x5
#define INS32_OP_LSL    0x6
#define INS32_OP_LSL_I  0x7
#define INS32_OP_LSR    0x8
#define INS32_OP_LSR_I  0x9
#define INS32_OP_JMP    0xa
#define INS32_OP_JMP_I  0xb
#define INS32_OP_CMP    0xc
#define INS32_OP_BT     0xd
#define INS32_OP_BF     0xe
#define INS32_OP_LDWU   0xf
#define INS32_OP_STW    0x10
#define INS32_OP_BARR   0x11
#define INS32_OP_SYSCALL 0x12
#define INS32_OP_RET    0x13
#define INS32_OP_WMSR   0x14
#define INS32_OP_RMSR   0x15
#define INS32_OP_VENTER 0x16
#define INS32_OP_VLEAVE 0x17

#define INS32_OP_ASR    0x1a
#define INS32_OP_ASR_I  0x1b
#define INS32_OP_ADD    0x1c
#define INS32_OP_ADD_I  0x1d
#define INS32_OP_SUB    0x1e
#define INS32_OP_MUL    0x1f
#define INS32_OP_DIV    0x20
#define INS32_OP_DIVU   0x21
#define INS32_OP_MOD    0x22
#define INS32_OP_MODU   0x23
#define INS32_OP_LDB    0x24
#define INS32_OP_LDBU   0x25
#define INS32_OP_LDH    0x26
#define INS32_OP_LDHU   0x27
#define INS32_OP_STB    0x28
#define INS32_OP_STH    0x29
#define INS32_OP_MHI    0x2a

#define INS32_ATTR_CMPEQ 0
#define INS32_ATTR_CMPGT 1
#define INS32_ATTR_CMPGTU 2


#define INS32_MASK_OPCODE   0x0000003f
#define INS32_MASK_RD       0x00000fc0
#define INS32_MASK_RS1      0x0003f000
#define INS32_MASK_RS2      0x00fc0000
#define INS32_MASK_IMM14    0xfffc0000
#define INS32_MASK_IMM18    0x3ffff000
#define INS32_MASK_REL20    0xfffff000
#define INS32_MASK_ATTR     0xff000000

#define INS32_SHIFT_OPCODE   0
#define INS32_SHIFT_RD       6
#define INS32_SHIFT_RS1      6+6
#define INS32_SHIFT_RS2      6+6+6
#define INS32_SHIFT_IMM14    6+6+6
#define INS32_SHIFT_IMM18    6+6
#define INS32_SHIFT_REL20    6+6
#define INS32_SHIFT_ATTR     6+6+6+6

#define INS32_GET_BITS(src, opc) ((uint32_t)(src & INS32_MASK_ ## opc) >> (INS32_SHIFT_ ## opc))

#define INSN_LEN            4
#define INSN_LEN_SHIFT      2

#define ADDR_RLNK           1
#define ADDR_SP             2

#endif /* NCPU32K_OPCODES_H */
