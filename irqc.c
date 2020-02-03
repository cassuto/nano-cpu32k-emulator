
#include "cpu.h"
#include "ncpu32k-exceptions.h"

/**
 * @brief set IRQ for given channel.
 * @param channel Number of target channel
 * @param raise Nonzero if IRQ is raised.
 */
void irqc_set_interrupt(int channel, char raise)
{
  if (raise)
    msr.IRR |= (1 << channel);
  else
    msr.IRR &= ~(1 << channel);
}

/**
 * @brief Query if given channel is masked.
 * @param channel Number of target channel
 * @return 1 if maksed, otherwise 0.
 */
int irqc_is_masked(int channel)
{
  return msr.IMR & (1 << channel);
}

/**
 * @brief Handle IRQ Exception
 * @retval >=0 No exception
 * @retval -EM_IRQ Interrupt Request Exception.
 */
int irqc_handle_irqs()
{
  if (msr.PSR.IRE && (msr.IRR & ~msr.IMR))
    {
      cpu_raise_exception(VECT_EIRQ, 0, 0);
      return -EM_IRQ;
    }
  return 0;
}
