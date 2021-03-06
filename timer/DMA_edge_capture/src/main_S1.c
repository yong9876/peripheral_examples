/**************************************************************************//**
 * @file
 * @brief This project demonstrates edge capture with LDMA. The first 512 events
 * captured by TIMER0 CC0 are transferred to a fixed length buffer by the
 * LDMA. For this example both rising and falling edges are captured.
 * @version 0.0.1
 ******************************************************************************
 * @section License
 * <b>Copyright 2015 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_core.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_device.h"
#include "em_ldma.h"
#include "bsp.h"

#define BUFFERSIZE 512
#define TIMER0_PRESCALE timerPrescale1

// Globally declared link descriptor
LDMA_Descriptor_t descLink;

// Edge capture buffer
volatile uint32_t buffer[BUFFERSIZE];

/**************************************************************************//**
 * @brief Configure and start DMA
 *****************************************************************************/
void initLDMA(void)
{
  // Default LDMA init
  LDMA_Init_t init = LDMA_INIT_DEFAULT;
  LDMA_Init(&init);

  // Configure LDMA for transfer from TIMER0 CC0 to memory
  // LDMA will perform a single unit transfer for each CC0 event
  LDMA_TransferCfg_t periTransferTx =
          LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_TIMER0_CC0);

  // Get default descriptor for transfer from TIM0_CC0 to the buffer
  LDMA_Descriptor_t xfer = LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&TIMER0->CC[0].CCV, &buffer, BUFFERSIZE);

  // Store descriptor globally
  descLink = xfer;
  // Transfer one word per unit
  descLink.xfer.size = ldmaCtrlSizeWord;
  // No interrupt after transfer complete
  descLink.xfer.doneIfs = 0;
  // Do not ignore single requests
  // IE: When Timer CC0 event occurs, transfer 1 unit then wait for next capture
  descLink.xfer.ignoreSrec = 0;

  // Start transfer, LDMA will transfer each edge
  LDMA_StartTransfer(0, (void*)&periTransferTx, (void*)&descLink);
}

/**************************************************************************//**
 * @brief GPIO initialization
 *****************************************************************************/
void initGPIO(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Set TIM0_CC0 #15 GPIO Pin (PC10) as Input
  GPIO_PinModeSet(gpioPortC, 10, gpioModeInput, 0);
}

/**************************************************************************//**
 * @brief TIMER initialization
 *****************************************************************************/
void initTIMER(void)
{
  // Enable clock for TIMER0 module
  CMU_ClockEnable(cmuClock_TIMER0, true);

  // Configure TIMER0 Compare/Capture settings
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  // Set event on every edge
  timerCCInit.eventCtrl = timerEventEveryEdge;
  timerCCInit.edge = timerEdgeBoth;
  timerCCInit.mode = timerCCModeCapture;
  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Route TIMER0 CC0 to location 15 and enable CC0 route pin
  // TIM0_CC0 #15 is GPIO Pin PC10
  TIMER0->ROUTEPEN |=  TIMER_ROUTEPEN_CC0PEN;
  TIMER0->ROUTELOC0 |= TIMER_ROUTELOC0_CC0LOC_LOC15;

  // Initialize timer with defined prescale
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  timerInit.prescale = TIMER0_PRESCALE;
  TIMER_Init(TIMER0, &timerInit);
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  // Chip errata
  CHIP_Init();

  // Initializations
  initLDMA();
  initGPIO();
  initTIMER();

  while (1)
  {
    EMU_EnterEM1();
  }
}
