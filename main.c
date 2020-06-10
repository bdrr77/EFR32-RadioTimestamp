/***************************************************************************//**
 * @file
 * @brief Simple RAIL application which does not include hal
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"

#include "rail.h"
#include "rail_config.h"

#include "hal-config.h"

#include "wtimer.h"
#include "time_sync.h"

// Prototypes
void RAILCb_Generic(RAIL_Handle_t railHandle, RAIL_Events_t events);

RAIL_Handle_t railHandle;

static RAIL_Config_t railCfg = {
  .eventsCallback = &RAILCb_Generic,
};

// Minimum allowed size of the TX FIFO
#define RAIL_TX_FIFO_SIZE 64

static uint8_t transmitData[RAIL_TX_FIFO_SIZE] = {
  0x0F, 0x16, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
};


void initRadio()
{
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
  CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;

  EMU_DCDCInit(&dcdcInit);
  CMU_HFXOInit(&hfxoInit);

  /* Switch HFCLK to HFXO and disable HFRCO */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);

  railHandle = RAIL_Init(&railCfg, NULL);
  if (railHandle == NULL) {
    while (1) ;
  }
  RAIL_ConfigCal(railHandle, RAIL_CAL_ALL);

  // Set us to a valid channel for this config and force an update in the main
  // loop to restart whatever action was going on
  RAIL_ConfigChannels(railHandle, channelConfigs[0], NULL);

  // Initialize the PA now that the HFXO is up and the timing is correct
  RAIL_TxPowerConfig_t txPowerConfig = {
#if HAL_PA_2P4_LOWPOWER
    .mode = RAIL_TX_POWER_MODE_2P4_LP,
#else
    .mode = RAIL_TX_POWER_MODE_2P4_HP,
#endif
    .voltage = BSP_PA_VOLTAGE,
    .rampTime = HAL_PA_RAMP,
  };
  if (channelConfigs[0]->configs[0].baseFrequency < 1000000000UL) {
    // Use the Sub-GHz PA if required
    txPowerConfig.mode = RAIL_TX_POWER_MODE_SUBGIG;
  }
  if (RAIL_ConfigTxPower(railHandle, &txPowerConfig) != RAIL_STATUS_NO_ERROR) {
    // Error: The PA could not be initialized due to an improper configuration.
    // Please ensure your configuration is valid for the selected part.
    while (1) ;
  }
  RAIL_SetTxPower(railHandle, HAL_PA_POWER);
}

int main(void)
{
  CHIP_Init();
  initRadio();
  initWtimer();
  timeSyncInit();

  RAIL_Idle(railHandle, RAIL_IDLE, true);
  uint16_t dataLength = sizeof(transmitData);
  RAIL_SetTxFifo(railHandle, &transmitData[0], dataLength, dataLength);
  RAIL_StartTx(railHandle, 0, RAIL_TX_OPTIONS_DEFAULT, NULL);


  while (1) ;
  return 0;
}

void RAILCb_Generic(RAIL_Handle_t railHandle, RAIL_Events_t events)
{
  (void) railHandle;
  (void) events;
}
