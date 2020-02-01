
#include "cpu.h"
#include "ncpu32k-exceptions.h"

#define VPN_SHIFT 13
#define PPN_SHIFT 13

/**
 * Translate virtual address to physical address.
 * @param [in] va Target Virtual Address
 * @param [out] pa Indicate where to store the physical address.
 * @param [in] store_insn Indicate if the insn is a store.
 * @retval -EM_TLB_MISS   Exception of TLB MISS
 * @retval -EM_PAGE_FAULT Exception of Page Fault. 
 * @retval >= 0              No exception.
 */
int dmmu_translate_vma(vm_addr_t va, phy_addr_t *pa, char store_insn)
{
  if (msr.PSR.DMME)
    {
      vm_addr_t vpn = va >> VPN_SHIFT;
      int offset = vpn & (dmmu_tlb_count-1);
      if (msr.DTLBL[offset].V && msr.DTLBL[offset].VPN == vpn)
        {
          if (msr.PSR.RM)
            {
              if ((store_insn && !msr.DTLBH[offset].RW) || 
                  (!store_insn && !msr.DTLBH[offset].RR))
                {
                  cpu_raise_exception(VECT_EDPF, va, 0);
                  return -EM_PAGE_FAULT;
                }
            }
          else
            {
              if ((store_insn && !msr.DTLBH[offset].UW) ||
                  (!store_insn && !msr.DTLBH[offset].UR))
                {
                  cpu_raise_exception(VECT_EDPF, va, 0);
                  return -EM_PAGE_FAULT;
                }
            }
          *pa = (msr.DTLBH[offset].PPN << PPN_SHIFT) | (va & ((1<<PPN_SHIFT)-1));
          return 0;
        }
      else
        {
          cpu_raise_exception(VECT_EDTM, va, 0);
          return -EM_TLB_MISS;
        }
    }
  else
    {
      /* no translation */
      *pa = va;
    }
  return 0;
}
