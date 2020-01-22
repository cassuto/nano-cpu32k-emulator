#ifndef NCPU32K_MSR_H
#define NCPU32K_MSR_H

#define MAX_MSR_BANK_BITS 10 /* (1 << MAX_MSR_BANK_BITS) = 65536 */

/* MSR banks  */
#define MSR_BANK_PS		(0L << MAX_MSR_BANK_BITS)
#define MSR_BANK_IM		(1L << MAX_MSR_BANK_BITS)
#define MSR_BANK_DM		(2L << MAX_MSR_BANK_BITS)
#define MSR_BANK_IC		(3L << MAX_MSR_BANK_BITS)
#define MSR_BANK_DC		(4L << MAX_MSR_BANK_BITS)
#define MSR_BANK_DBG	(5L << MAX_MSR_BANK_BITS)

/* MSR bank - PS */

/* MSR.PSR */
#define MSR_PSR	(MSR_BANK_PS + 0x0)
#define MSR_PSR_CC		(1L << 0)
#define MSR_PSR_CY		(1L << 1)
#define MSR_PSR_OV		(1L << 2)
#define MSR_PSR_OE		(1L << 3)
#define MSR_PSR_RM		(1L << 4)
#define MSR_PSR_IRE		(1L << 5)
#define MSR_PSR_IMME	(1L << 6)
#define MSR_PSR_DMME	(1L << 7)
#define MSR_PSR_ICAE	(1L << 8)
#define MSR_PSR_DCAE	(1L << 9)

/* MSR.CPUID */
#define MSR_CPUID	(MSR_BANK_PS + 0x1)
#define MSR_CPUID_VER	0x000000ff
#define MSR_CPUID_REV	0x0003ff00
#define MSR_CPUID_FIMM	(1L << 18)
#define MSR_CPUID_FDMM	(1L << 19)
#define MSR_CPUID_FICA	(1L << 20)
#define MSR_CPUID_FDCA	(1L << 21)
#define MSR_CPUID_FDBG	(1L << 22)
#define MSR_CPUID_FFPI  (1L << 23)

/* MSR.EPSR */
#define MSR_EPSR	(MSR_BANK_PS + 0x2)

/* MSR.EPC */
#define MSR_EPC	(MSR_BANK_PS + 0x3)

/* MSR.MSR_ELSA */
#define MSR_ELSA	(MSR_BANK_PS + 0x4)


/* MSR bank - DBG */

/* MSR.DBGR */
#define MSR_DBGR_NUMPORT (MSR_BANK_DBG + 0x0)
#define MSR_DBGR_MSGPORT (MSR_BANK_DBG + 0x1)

#endif /* NCPU32K_MSR_H */