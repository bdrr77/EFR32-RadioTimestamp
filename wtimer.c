#include "wtimer.h"

#include "em_cmu.h"
#include "em_timer.h"

/**************************************************************************//**
 * @brief
 *    WTIMER initialization
 *****************************************************************************/
void initWtimer(void)
{
  // Enable clock for WTIMER0 module
  CMU_ClockEnable(cmuClock_WTIMER0, true);

  // Set Top value
  // Note each overflow event constitutes 1/2 the signal period
  uint32_t topValue = 0xFFFFFFFF;//CMU_ClockFreqGet(cmuClock_HFPER) / (2 * OUT_FREQ * (1 << WTIMER0_PRESCALE));
  TIMER_TopSet (WTIMER0, topValue);

  // Initialize and start wtimer with defined prescale
  TIMER_Init_TypeDef wtimerInit = TIMER_INIT_DEFAULT;
  wtimerInit.prescale = timerPrescale32;//1.2MHz tick rate
  TIMER_Init(WTIMER0, &wtimerInit);
}