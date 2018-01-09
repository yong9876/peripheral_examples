/**************************************************************************//**
 * @main_series1.c
 * @brief Demonstrates USART1 as SPI slave.
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
#include "em_gpio.h"
#include "em_usart.h"

#define RX_BUFFER_SIZE   10
#define TX_BUFFER_SIZE   RX_BUFFER_SIZE

uint8_t RxBuffer[RX_BUFFER_SIZE] = {0};
uint8_t RxBufferIndex = 0;

uint8_t TxBuffer[TX_BUFFER_SIZE] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9};
uint8_t TxBufferIndex = 0;

/**************************************************************************//**
 * @brief USART1 RX IRQ Handler
 *****************************************************************************/
void USART1_RX_IRQHandler(void)
{
  uint8_t rxData;

  if (USART1->STATUS & USART_STATUS_RXDATAV)
  {
      // Read data
	  rxData = USART1->RXDATA;

    if (RxBuffer != 0)
    {
      // Store data in buffer
      RxBuffer[RxBufferIndex] = rxData;
      RxBufferIndex++;

      if (RxBufferIndex == RX_BUFFER_SIZE)
      {
    	  // Place breakpoint here and observe RxBuffer in IDE varible/expressions window
    	  // RxBuffer should contain 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
    	  RxBufferIndex = 0;
      }
    }
  }
}

/**************************************************************************//**
 * @brief USART1 TX IRQ Handler
 *****************************************************************************/
void USART1_TX_IRQHandler(void)
{
  if (USART1->STATUS & USART_STATUS_TXBL)
  {
    // Send data
	USART1->TXDATA = TxBuffer[TxBufferIndex];

	TxBufferIndex++;
    if (TxBufferIndex == TX_BUFFER_SIZE)
    {
      TxBufferIndex = 0;
    }
  }
}

/**************************************************************************//**
 * @brief Initialize USART1
 *****************************************************************************/
void initUSART1 (void)
{
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_USART1, true);

	// Start with default config, then modify as necessary
	USART_InitSync_TypeDef config = USART_INITSYNC_DEFAULT;
	config.master    = false;
	config.clockMode = usartClockMode0; // clock idle low, sample on rising/first edge
	config.msbf      = true;            // send MSB first
    USART_InitSync(USART1, &config);

    // Set USART pin locations
    USART1->ROUTELOC0 = (USART_ROUTELOC0_CLKLOC_LOC11) | // US1_CLK       on location 11 = PC8 per datasheet section 6.4 = EXP Header pin 8
                        (USART_ROUTELOC0_CSLOC_LOC11)  | // US1_CS        on location 11 = PA9 per datasheet section 6.4 = EXP Header pin 10
                        (USART_ROUTELOC0_TXLOC_LOC11)  | // US1_TX (MOSI) on location 11 = PA6 per datasheet section 6.4 = EXP Header pin 4
                        (USART_ROUTELOC0_RXLOC_LOC11);   // US1_RX (MISO) on location 11 = PA7 per datasheet section 6.4 = EXP Header pin 6

    // Enable USART pins
    USART1->ROUTEPEN = USART_ROUTEPEN_CLKPEN | USART_ROUTEPEN_CSPEN | USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN;

    // Configure GPIO mode
    GPIO_PinModeSet(gpioPortC, 8, gpioModeInput, 1);    // US1_CLK is input
    GPIO_PinModeSet(gpioPortC, 9, gpioModeInput, 1);    // US1_CS is input
    GPIO_PinModeSet(gpioPortC, 6, gpioModeInput, 1);    // US1_TX (MOSI) is input
    GPIO_PinModeSet(gpioPortC, 7, gpioModePushPull, 1); // US1_RX (MISO) is push pull

    // Clear RX buffer and shift register
    USART1->CMD |= USART_CMD_CLEARRX;

    // Enable USART1 RX interrupts
    USART_IntClear(USART1, USART_IF_RXDATAV);
    USART_IntEnable(USART1, USART_IF_RXDATAV);
    NVIC_ClearPendingIRQ(USART1_RX_IRQn);
    NVIC_EnableIRQ(USART1_RX_IRQn);

    // Clear TX buffer and shift register
    USART1->CMD |= USART_CMD_CLEARTX;

    // Enable USART1 TX interrupts
	USART_IntClear(USART1, USART_IF_TXBL);
	USART_IntEnable(USART1, USART_IF_TXBL);
	NVIC_ClearPendingIRQ(USART1_TX_IRQn);
	NVIC_EnableIRQ(USART1_TX_IRQn);

    // Enable USART1
    USART_Enable(USART1, usartEnable);
}

/**************************************************************************//**
 * @brief Main function
 *****************************************************************************/
int main(void)
{
  // Initialize chip
  CHIP_Init();

  // Initialize USART1 as SPI slave
  initUSART1();

  while(1);
}

