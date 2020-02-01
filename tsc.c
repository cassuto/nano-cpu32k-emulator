
#include "cpu.h"
#include "ncpu32k-msr.h"
#include "ncpu32k-exceptions.h"

#define CNT_MASK (MSR_TCR_CNT >> MSR_TCR_CNT_SHIFT)

/**
 * Emulate a clk edge of TSC.
 * @retval >= 0 No any exception.
 * @retval -EM_IRQ  Interrupt Request Exception.
 */
int tsc_clk()
{
  int ret = 0;
  if (msr.PSR.IRE && msr.TCR.IE &&
      (msr.TCR.IR || (msr.TSR & CNT_MASK == msr.TCR.CNT)))
    {
      msr.TCR.IR = 1;
      cpu_raise_exception(VECT_EIRQ, 0, 0);
      ret = -EM_IRQ;
    }
  if (msr.TCR.EN)
    {
      ++msr.TSR;
    }
  return ret;
}
