#ifndef NCPU32K_MSR_H
#define NCPU32K_MSR_H

#define MAX_MSR_BANK_BITS 10 /* (1 << MAX_MSR_BANK_BITS) = 65536 */

/* MSR banks  */
#define MSR_BANK_PS		(0L << MAX_MSR_BANK_BITS)
#define MSR_BANK_IMM	(1L << MAX_MSR_BANK_BITS)
#define MSR_BANK_DMM	(2L << MAX_MSR_BANK_BITS)
#define MSR_BANK_ICA	(3L << MAX_MSR_BANK_BITS)
#define MSR_BANK_DCA	(4L << MAX_MSR_BANK_BITS)
#define MSR_BANK_DBG	(5L << MAX_MSR_BANK_BITS)
#define MSR_BANK_TSC	(6L << MAX_MSR_BANK_BITS)

/*********************************************************************
* MSR bank - PS
**********************************************************************/

/* MSR.PSR */
#define MSR_PSR	(MSR_BANK_PS + 0x0)
#define MSR_PSR_CC_SHIFT	0
#define MSR_PSR_CC		(1L << MSR_PSR_CC_SHIFT)
#define MSR_PSR_CY_SHIFT	1
#define MSR_PSR_CY		(1L << MSR_PSR_CY_SHIFT)
#define MSR_PSR_OV_SHIFT	2
#define MSR_PSR_OV		(1L << MSR_PSR_OV_SHIFT)
#define MSR_PSR_OE_SHIFT	3
#define MSR_PSR_OE		(1L << MSR_PSR_OE_SHIFT)
#define MSR_PSR_RM_SHIFT	4
#define MSR_PSR_RM		(1L << MSR_PSR_RM_SHIFT)
#define MSR_PSR_IRE_SHIFT	5
#define MSR_PSR_IRE		(1L << MSR_PSR_IRE_SHIFT)
#define MSR_PSR_IMME_SHIFT	6
#define MSR_PSR_IMME	(1L << MSR_PSR_IMME_SHIFT)
#define MSR_PSR_DMME_SHIFT	7
#define MSR_PSR_DMME	(1L << MSR_PSR_DMME_SHIFT)
#define MSR_PSR_ICAE_SHIFT	8
#define MSR_PSR_ICAE	(1L << MSR_PSR_ICAE_SHIFT)
#define MSR_PSR_DCAE_SHIFT	9
#define MSR_PSR_DCAE	(1L << MSR_PSR_DCAE_SHIFT)

/* MSR.CPUID */
#define MSR_CPUID	(MSR_BANK_PS + 0x1)
#define MSR_CPUID_VER_SHIFT	0
#define MSR_CPUID_VER	0x000000ff
#define MSR_CPUID_REV_SHIFT	8
#define MSR_CPUID_REV	0x0003ff00
#define MSR_CPUID_FIMM_SHIFT 18
#define MSR_CPUID_FIMM	(1L << MSR_CPUID_FIMM_SHIFT)
#define MSR_CPUID_FDMM_SHIFT 19
#define MSR_CPUID_FDMM	(1L << MSR_CPUID_FDMM_SHIFT)
#define MSR_CPUID_FICA_SHIFT 20
#define MSR_CPUID_FICA	(1L << MSR_CPUID_FICA_SHIFT)
#define MSR_CPUID_FDCA_SHIFT 21
#define MSR_CPUID_FDCA	(1L << MSR_CPUID_FDCA_SHIFT)
#define MSR_CPUID_FDBG_SHIFT 22
#define MSR_CPUID_FDBG	(1L << MSR_CPUID_FDBG_SHIFT)
#define MSR_CPUID_FFPU_SHIFT 23
#define MSR_CPUID_FFPU  (1L << MSR_CPUID_FFPU_SHIFT)
#define MSR_CPUID_FTSC_SHIFT 24
#define MSR_CPUID_FTSC  (1L << MSR_CPUID_FTSC_SHIFT)

/* MSR.EPSR */
#define MSR_EPSR	(MSR_BANK_PS + 0x2)

/* MSR.EPC */
#define MSR_EPC	(MSR_BANK_PS + 0x3)

/* MSR.ELSA */
#define MSR_ELSA	(MSR_BANK_PS + 0x4)

/* MSR.COREID */
#define MSR_COREID	(MSR_BANK_PS + 0x5)


/*********************************************************************
* MSR bank - IMM
**********************************************************************/

/* MSR.IMMID */
#define MSR_IMMID	(MSR_BANK_IMM + 0x0)
#define MSR_IMMID_STLB	0x7
#define MSR_IMMID_STLB_SHIFT	0
/* MSR.ITLBL */
#define MSR_ITLBL	(MSR_BANK_IMM + 0x100)
/* MSR.ITLBH */
#define MSR_ITLBH	(MSR_BANK_IMM + 0x180)


/*********************************************************************
* MSR bank - DMM
**********************************************************************/

/* MSR.DMMID */
#define MSR_DMMID	(MSR_BANK_DMM + 0x0)
#define MSR_DMMID_STLB	0x7
#define MSR_DMMID_STLB_SHIFT	0
/* MSR.DTLBL */
#define MSR_DTLBL	(MSR_BANK_DMM + 0x100)
/* MSR.DTLBH */
#define MSR_DTLBH	(MSR_BANK_DMM + 0x180)


/*********************************************************************
* MSR bank - ICA
**********************************************************************/

/* MSR.ICID */
#define MSR_ICID	(MSR_BANK_ICA + 0x0)
#define MSR_ICID_SS	0xf
#define MSR_ICID_SS_SHIFT	0
#define MSR_ICID_SL	(1L << 4)
#define MSR_ICINV	(MSR_BANK_ICA + 0x1)
#define MSR_ICFLS	(MSR_BANK_ICA + 0x2)


/*********************************************************************
* MSR bank - DCA
**********************************************************************/

/* MSR.DCID */
#define MSR_DCID	(MSR_BANK_DCA + 0x0)
#define MSR_DCID_SS	0xf
#define MSR_DCID_SS_SHIFT	0
#define MSR_DCID_SL	(1L << 4)
#define MSR_DCID_SL_SHIFT	4
#define MSR_DCINV	(MSR_BANK_DCA + 0x1)
#define MSR_DCFLS	(MSR_BANK_DCA + 0x2)


/*********************************************************************
* MSR bank - DBG
**********************************************************************/

/* MSR.DBGR */
#define MSR_DBGR_NUMPORT (MSR_BANK_DBG + 0x0)
#define MSR_DBGR_MSGPORT (MSR_BANK_DBG + 0x1)

/*********************************************************************
* MSR bank - TSC
**********************************************************************/

/* MSR.TSC */
#define MSR_TSC_TSR (MSR_BANK_TSC + 0x0)

#endif /* NCPU32K_MSR_H */