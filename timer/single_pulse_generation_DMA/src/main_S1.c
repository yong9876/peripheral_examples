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

// First compare edge for 3 second delay
// 55664 / (19MHz / 1024) = 3 seconds
#define COMPARE_VALUE1 55664

// Second compare edge for 1 ms pulse
// (20 - 1) / (19MHz / 1024) = 1 millisecond
#define COMPARE_VALUE2 (COMPARE_VALUE1 + 20)

// Globally stored value for DMA transfer
uint32_t value = COMPARE_VALUE2;

// Globally declared link descriptor
LDMA_Descriptor_t descLink;

/***************************************************************************//**
 * @brief
 *   LDMA IRQ handler.
 ******************************************************************************/
void LDMA_IRQHandler( void )
{
  // Interrupt occurs after transfer complete
  // Clear interrupt source
  LDMA->IFC = 0x1;
}

/**************************************************************************//**
 * @brief Configure and start DMA
 *****************************************************************************/
void initLDMA(void)
{
  // Default LDMA init
  LDMA_Init_t init = LDMA_INIT_DEFAULT;
  LDMA_Init(&init);

  // Configure LDMA for transfer from memory to TIMER0 CC0
  // LDMA will perform a single unit transfer
  LDMA_TransferCfg_t periTransferTx =
          LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_TIMER0_CC0);

  // Get default descriptor for transfer from TIM0_CC0 to the buffer
  LDMA_Descriptor_t xfer = LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(&value, &TIMER0->CC[0].CCV, 1);

  // Store descriptor globally
  descLink = xfer;

  // Transfer one word per unit
  descLink.xfer.size = ldmaCtrlSizeWord;

  // Do not ignore single requests
  // IE: When Timer CC0 event occurs, transfer 1 unit then wait for next compare
  descLink.xfer.ignoreSrec = 0;

  // Start transfer, LDMA will trigger after first compare event
  LDMA_StartTransfer(0, (void*)&periTransferTx, (void*)&descLink);
}

/**************************************************************************//**
 * @brief GPIO initialization
 *****************************************************************************/
void initGPIO(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Set TIM0_CC0 #15 GPIO Pin (PC10) as Output
  GPIO_PinModeSet(gpioPortC, 10, gpioModePushPull, 0);
}

/**************************************************************************//**
 * @brief TIMER initialization
 *****************************************************************************/
void initTIMER(void)
{
  // Enable clock for TIMER0 module
  CMU_ClockEnable(cmuClock_TIMER0, true);

  // Configure TIMER0 Compare/Capture for output compare
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModeCompare;

  // Toggle output on compare match
  timerCCInit.cmoa = timerOutputActionToggle;
  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Set route to Location 15 and enable
  // TIM0_CC0 #15 is PC10 (Expansion Header Pin 16)
  TIMER0->ROUTELOC0 |= TIMER_ROUTELOC0_CC0LOC_LOC15;
  TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN;

  // Load first compare edge
  TIMER_CompareSet(TIMER0, 0, COMPARE_VALUE1);

  // Initialize and start timer with highest prescale
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  timerInit.prescale = timerPrescale1024;

  // Generate only one pulse
  timerInit.oneShot = true;
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
