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

/* MSR.PSR R/W */
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

/* MSR.CPUID R */
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

/* MSR.EPSR R/W */
#define MSR_EPSR	(MSR_BANK_PS + 0x2)

/* MSR.EPC R/W */
#define MSR_EPC	(MSR_BANK_PS + 0x3)

/* MSR.ELSA R/W */
#define MSR_ELSA	(MSR_BANK_PS + 0x4)

/* MSR.COREID R */
#define MSR_COREID	(MSR_BANK_PS + 0x6)


/*********************************************************************
* MSR bank - IMM
**********************************************************************/

/* MSR.IMMID R */
#define MSR_IMMID	(MSR_BANK_IMM + 0x0)
#define MSR_IMMID_STLB_SHIFT	0
#define MSR_IMMID_STLB	0x7
/* MSR.ITLBL R/W */
#define MSR_ITLBL	(MSR_BANK_IMM + 0x100)
#define MSR_ITLBL_V_SHIFT 0
#define MSR_ITLBL_V		0x1
#define MSR_ITLBL_VPN_SHIFT 13
#define MSR_ITLBL_VPN	(~((1L << MSR_ITLBL_VPN_SHIFT)-1))
/* MSR.ITLBH R/W */
#define MSR_ITLBH	(MSR_BANK_IMM + 0x180)
#define MSR_ITLBH_P_SHIFT	0
#define MSR_ITLBH_P	(1L << MSR_ITLBH_P_SHIFT)
#define MSR_ITLBH_D_SHIFT	1
#define MSR_ITLBH_D	(1L << MSR_ITLBH_D_SHIFT)
#define MSR_ITLBH_A_SHIFT	2
#define MSR_ITLBH_A	(1L << MSR_ITLBH_A_SHIFT)
#define MSR_ITLBH_UX_SHIFT	3
#define MSR_ITLBH_UX	(1L << MSR_ITLBH_UX_SHIFT)
#define MSR_ITLBH_RX_SHIFT	4
#define MSR_ITLBH_RX	(1L << MSR_ITLBH_RX_SHIFT)
#define MSR_ITLBH_NC_SHIFT	7
#define MSR_ITLBH_NC	(1L << MSR_ITLBH_NC_SHIFT)
#define MSR_ITLBH_S_SHIFT	8
#define MSR_ITLBH_S		(1L << MSR_ITLBH_S_SHIFT)
#define MSR_ITLBH_PPN_SHIFT 13
#define MSR_ITLBH_PPN	(~((1L << MSR_ITLBH_PPN_SHIFT)-1))

/*********************************************************************
* MSR bank - DMM
**********************************************************************/

/* MSR.DMMID R */
#define MSR_DMMID	(MSR_BANK_DMM + 0x0)
#define MSR_DMMID_STLB_SHIFT	0
#define MSR_DMMID_STLB	0x7
/* MSR.DTLBL R/W */
#define MSR_DTLBL	(MSR_BANK_DMM + 0x100)
#define MSR_DTLBL_V_SHIFT 0
#define MSR_DTLBL_V		0x1
#define MSR_DTLBL_VPN_SHIFT 13
#define MSR_DTLBL_VPN	(~((1L << MSR_DTLBL_VPN_SHIFT)-1))
/* MSR.DTLBH R/W */
#define MSR_DTLBH	(MSR_BANK_DMM + 0x180)
#define MSR_DTLBH_P_SHIFT	0
#define MSR_DTLBH_P	(1L << MSR_DTLBH_P_SHIFT)
#define MSR_DTLBH_D_SHIFT	1
#define MSR_DTLBH_D	(1L << MSR_DTLBH_D_SHIFT)
#define MSR_DTLBH_A_SHIFT	2
#define MSR_DTLBH_A	(1L << MSR_DTLBH_A_SHIFT)
#define MSR_DTLBH_UW_SHIFT	3
#define MSR_DTLBH_UW	(1L << MSR_DTLBH_UW_SHIFT)
#define MSR_DTLBH_UR_SHIFT	4
#define MSR_DTLBH_UR	(1L << MSR_DTLBH_UR_SHIFT)
#define MSR_DTLBH_RW_SHIFT	5
#define MSR_DTLBH_RW	(1L << MSR_DTLBH_RW_SHIFT)
#define MSR_DTLBH_RR_SHIFT	6
#define MSR_DTLBH_RR	(1L << MSR_DTLBH_RR_SHIFT)
#define MSR_DTLBH_NC_SHIFT	7
#define MSR_DTLBH_NC	(1L << MSR_DTLBH_NC_SHIFT)
#define MSR_DTLBH_S_SHIFT	8
#define MSR_DTLBH_S		(1L << MSR_DTLBH_S_SHIFT)
#define MSR_DTLBH_PPN_SHIFT 13
#define MSR_DTLBH_PPN	(~((1L << MSR_DTLBH_PPN_SHIFT)-1))

/*********************************************************************
* MSR bank - ICA
**********************************************************************/

/* MSR.ICID */
#define MSR_ICID	(MSR_BANK_ICA + 0x0)
#define MSR_ICID_SS_SHIFT	0
#define MSR_ICID_SS	0xf
#define MSR_ICID_SL_SHIFT 4
#define MSR_ICID_SL	(1L << MSR_ICID_SL_SHIFT)
#define MSR_ICINV	(MSR_BANK_ICA + 0x1)
#define MSR_ICFLS	(MSR_BANK_ICA + 0x2)


/*********************************************************************
* MSR bank - DCA
**********************************************************************/

/* MSR.DCID */
#define MSR_DCID	(MSR_BANK_DCA + 0x0)
#define MSR_DCID_SS_SHIFT	0
#define MSR_DCID_SS	0xf
#define MSR_DCID_SL_SHIFT	4
#define MSR_DCID_SL	(1L << MSR_DCID_SL_SHIFT)
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

/* MSR.TSR R/W */
#define MSR_TSR (MSR_BANK_TSC + 0x0)

/* MSR.TCR R/W */
#define MSR_TCR	(MSR_BANK_TSC + 0x1)
#define MSR_TCR_CNT_SHIFT 0
#define MSR_TCR_CNT	0x0fffffff
#define MSR_TCR_EN_SHIFT 28
#define MSR_TCR_EN	(1L << MSR_TCR_EN_SHIFT)
#define MSR_TCR_IE_SHIFT 29
#define MSR_TCR_IE	(1L << MSR_TCR_IE_SHIFT)
#define MSR_TCR_IR_SHIFT 30
#define MSR_TCR_IR	(1L << MSR_TCR_IR_SHIFT)

#endif /* NCPU32K_MSR_H */
