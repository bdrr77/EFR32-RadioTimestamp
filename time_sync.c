#include "time_sync.h"

#include "em_ldma.h"
#include "em_prs.h"
#include "em_cmu.h"

#include "efr32mg13p_prs_signals.h"

/* DMA channel used for the examples */
#define DMA_CHANNEL         0
#define DMA_CH_MASK         1 << DMA_CHANNEL

/* Memory to memory transfer buffer size and constant for GPIO PRS */
#define BUFFER_SIZE         1
#define TRANSFER_SIZE       BUFFER_SIZE - 1
#define RADIO_PRS_CHANNEL    1

/* Buffer for memory to memory transfer */
uint32_t dstBuffer[BUFFER_SIZE];

static void radioPrsSetup(void);
static void initLdma(void);

void timeSyncInit(void)
{
  /* Initialize Radio for PRS */
  radioPrsSetup();
  /* Initialize LDMA */
  initLdma();
  
  dstBuffer[0] = 0;
}

static void radioPrsSetup(void)
{
  //Check that radio is ready
  //TODO

  /* Select Radio as PRS source and push button BTN1 as signal for PRS channel */
  CMU_ClockEnable(cmuClock_PRS, true);
  PRS_SourceSignalSet(RADIO_PRS_CHANNEL, 
                      PRS_MODEM_PRESENT & 0xFF00,
                      ((PRS_MODEM_PRESENT & _PRS_CH_CTRL_SIGSEL_MASK) >> _PRS_CH_CTRL_SIGSEL_SHIFT),
                      prsEdgePos);


  /* Select PRS channel for DMA request 0 */
  PRS->DMAREQ0 = PRS_DMAREQ0_PRSSEL_PRSCH1;

}

static void initLdma(void)
{

  LDMA_Init_t init = LDMA_INIT_DEFAULT;
  LDMA_Init( &init );

  /* Writes directly to the LDMA channel registers */
  LDMA->CH[DMA_CHANNEL].CTRL =
      LDMA_CH_CTRL_SIZE_WORD
      | LDMA_CH_CTRL_REQMODE_ALL
      | LDMA_CH_CTRL_BLOCKSIZE_UNIT1
      | (TRANSFER_SIZE) << _LDMA_CH_CTRL_XFERCNT_SHIFT
      | LDMA_CH_CTRL_SRCINC_NONE
      | LDMA_CH_CTRL_DSTINC_NONE;
  
  LDMA->CH[DMA_CHANNEL].SRC = (0x4001A000 + 0x024);
  LDMA->CH[DMA_CHANNEL].DST = (uint32_t)&dstBuffer;

  /* Enable interrupt and wait PRS on DMAREQ0 to start transfer */
  LDMA->CH[DMA_CHANNEL].REQSEL = ldmaPeripheralSignal_PRS_REQ0;
  LDMA->IFC = DMA_CH_MASK;
  LDMA->IEN = DMA_CH_MASK;

  /* Enable LDMA Channel */
  LDMA->CHEN = DMA_CH_MASK;
}

/***************************************************************************//**
 * @brief
 *   LDMA IRQ handler.
 ******************************************************************************/
void LDMA_IRQHandler( void )
{
  uint32_t pending;

  /* Read interrupt source */
  pending = LDMA_IntGet();

  /* Clear interrupts */
  LDMA_IntClear(pending);

  /* Check for LDMA error */
  if ( pending & LDMA_IF_ERROR ){
    /* Loop here to enable the debugger to see what has happened */
    while (1);
  }
}
