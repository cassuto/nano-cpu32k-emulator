
#include "cpu.h"
#include "ncpu32k-msr.h"
#include "ncpu32k-exceptions.h"

#define CNT_MASK (MSR_TCR_CNT >> MSR_TCR_CNT_SHIFT)

/**
 * Emulate a clk edge of TSC.
 * @retval >= 0 If not any exception.
 */
int tsc_clk()
{
  if (msr.TCR.EN)
    {
      if (msr.TCR.I && (msr.TSR & CNT_MASK) == msr.TCR.CNT)
        {
          msr.TCR.P = 1;
          tsc_update_tcr();
        }
      ++msr.TSR;
    }
  return 0;
}

/**
 * @brief Called when TCR is updated.
 */
void tsc_update_tcr()
{
  irqc_set_interrupt(IRQ_TSC, msr.TCR.P && msr.TCR.I);
}
