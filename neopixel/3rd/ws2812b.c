#include "ws2812b.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/dma.h>
#include <atom.h>
#include <string.h>
// A version
// 500ns + 2000ns
// 1200ns + 1300ns
//
//
// * PB15 - MOSI
//
static int8_t rest=0;
static uint8_t bin[24*8];
static void prepare_bin(uint8_t r, uint8_t g, uint8_t b, uint8_t *pbin);

void ws2812b_send_pixels(uint32_t *data, uint8_t len){
  rest=len;
  uint8_t *ptr=bin;
  for(int i=0;i<8;i++){
    uint32_t rgb=*data;
    prepare_bin(0xff&(rgb>>16), 0xff&(rgb>>8), 0xff&(rgb), ptr);
    ptr+=24;
    data++;
  }

  dma_set_memory_address(DMA1, DMA_CHANNEL5, (uint32_t)bin);
  dma_set_number_of_data(DMA1, DMA_CHANNEL5, 24*8);
  dma_enable_channel(DMA1, DMA_CHANNEL5);
}

static void byte_decode(uint8_t data, uint8_t* p) {
  for(uint8_t mask = 0x80; mask; mask >>= 1){
    *p= (data & mask)?0b11111000:0b11100000;
    p++;
  }
}

static inline void prepare_bin(uint8_t r, uint8_t g, uint8_t b, uint8_t *pbin){
  byte_decode(g,pbin);
  byte_decode(r,pbin+8);
  byte_decode(b,pbin+16);
}

void ws2812b_setup(void) {
  rcc_periph_clock_enable(RCC_DMA1);
  rcc_periph_clock_enable(RCC_SPI2);
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO15 );

  spi_reset(SPI2);

  SPI2_I2SCFGR = 0; //disable i2s
  /* Set up SPI in Master mode with:
   * Clock baud rate: 1/64 of peripheral clock frequency
   * Clock polarity: Idle High
   * Clock phase: Data valid on 2nd clock pulse
   * Data frame format: 8-bit
   * Frame format: MSB First
   */
  spi_init_master(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_4, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
      SPI_CR1_CPHA_CLK_TRANSITION_2, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

  spi_enable_software_slave_management(SPI2);
  spi_set_nss_high(SPI2);

  spi_enable(SPI2);

  nvic_set_priority(NVIC_DMA1_CHANNEL5_IRQ, 0);
  nvic_enable_irq(NVIC_DMA1_CHANNEL5_IRQ);
  dma_channel_reset(DMA1, DMA_CHANNEL5);

  dma_set_peripheral_address(DMA1, DMA_CHANNEL5, (uint32_t)&SPI2_DR);
  dma_set_read_from_memory(DMA1, DMA_CHANNEL5);
  dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL5);

  dma_set_peripheral_size(DMA1, DMA_CHANNEL5, DMA_CCR_PSIZE_8BIT);
  dma_set_memory_size(DMA1, DMA_CHANNEL5, DMA_CCR_MSIZE_8BIT);

  dma_set_priority(DMA1, DMA_CHANNEL5, DMA_CCR_PL_HIGH);
  dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);

  spi_enable_tx_dma(SPI2);

}

void dma1_channel5_isr(void) {
  //    atomIntEnter();
  if ((DMA1_ISR &DMA_ISR_TCIF5) != 0) {
    DMA1_IFCR |= DMA_IFCR_CTCIF5;
  }
  //spi_disable_tx_dma(SPI2);
  dma_disable_channel(DMA1, DMA_CHANNEL5);
  //    atomSemPut (&dma_busy);
  //    atomIntExit(0);
}
