
#include "cpu.h"
#include "ncpu32k-exceptions.h"

#define VPN_SHIFT 13
#define PPN_SHIFT 13

/**
 * Translate virtual address to physical address.
 * @param [in] va Target Virtual Address
 * @param [out] pa Indicate where to store the physical address.
 * @retval -EM_TLB_MISS   Exception of TLB MISS
 * @retval -EM_PAGE_FAULT Exception of Page Fault. 
 * @retval >= 0              No exception.
 */
int immu_translate_vma(vm_addr_t va, phy_addr_t *pa)
{
  if (msr.PSR.IMME)
    {
      vm_addr_t vpn = va >> VPN_SHIFT;
      int offset = vpn & (immu_tlb_count-1);
      if (msr.ITLBL[offset].V && msr.ITLBL[offset].VPN == vpn)
        {
          if ((msr.PSR.RM && !msr.ITLBH[offset].RX) ||
              (!msr.PSR.RM && !msr.ITLBH[offset].UX))
            {
              cpu_raise_exception(VECT_EIPF, va, 0);
              return -EM_PAGE_FAULT;
            }
          *pa = (msr.ITLBH[offset].PPN << PPN_SHIFT) | (va & ((1<<PPN_SHIFT)-1));
          return 0;
        }
      else
        {
          cpu_raise_exception(VECT_EITM, va, 0);
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
