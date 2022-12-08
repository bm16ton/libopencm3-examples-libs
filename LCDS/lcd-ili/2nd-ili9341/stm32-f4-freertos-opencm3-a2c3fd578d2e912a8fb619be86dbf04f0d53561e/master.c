
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/fsmc.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wctype.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <time.h>

#include <syscall.h>
#include <fsmcwr.h>

#include <buffer.h>
#include <uastdio.h>
#include <ili9341.h>
#include <console.h>


void delay(uint32_t n) {
    for (volatile int i = 0; i < n * 10; i++)
        __asm__("nop");
}

void _delay_ms(uint32_t n) {
    for (volatile int i = 0; i < n * 80000; i++)
        __asm__("nop");
}


static void clock_setup(void) {
    rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_GPIOE);

    rcc_periph_clock_enable(RCC_FSMC);

    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_USART1);
}

/*** USART ***/
void usart_setup(void) {

    usart_disable(USART1);
    nvic_enable_irq(NVIC_USART1_IRQ);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);


    usart_set_baudrate(USART1, 115200);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART1, USART_MODE_TX_RX);

    usart_enable_rx_interrupt(USART1);

    usart_enable(USART1);
}

inline bool usart_recv_is_ready(uint32_t usart) {
    return (USART_SR(usart) & USART_SR_RXNE);
}

inline bool usart_rx_int_is_enable(uint32_t usart) {
    return (USART_CR1(USART1) & USART_CR1_RXNEIE);
}

void usart1_isr(void) {
    static uint8_t data = 0;

    if (usart_rx_int_is_enable(USART1) && usart_recv_is_ready(USART1)) {
        data = usart_recv(USART1);
        buffer_put_byte(&stdin_buffer, data);
        buffer_put_byte(&stdout_buffer, '3');
        if (data == '\r')
            buffer_put_byte(&stdout_buffer, '\n');
        //usart_enable_tx_interrupt(USART1);
    }
}

/*** TIM2 ***/
static void tim2_setup(void) {

    rcc_periph_clock_enable(RCC_TIM2);
    nvic_enable_irq(NVIC_TIM2_IRQ);

    timer_reset(TIM2);

    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    timer_direction_up(TIM2);
    timer_set_prescaler(TIM2, 64);

    timer_disable_preload(TIM2);
    timer_continuous_mode(TIM2);
    timer_set_period(TIM2, 64);

    timer_set_oc_value(TIM2, TIM_OC1, 1);
    timer_enable_irq(TIM2, TIM_DIER_CC1IE);

    timer_enable_counter(TIM2);
}

void tim2_isr(void) {
    if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {
        timer_clear_flag(TIM2, TIM_SR_CC1IF);

        uint8_t c;
        while ((c = buffer_get_byte(&stdout_buffer)) > 0)
            usart_send_blocking(USART1, c);

    }
}

static void systick_setup(void) {
    /* clock rate / 1000 to get 1mS interrupt rate */
    systick_set_reload(168000);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    systick_interrupt_enable();
}



void fsmc_setup(void) {
    /*
      D0   PD14
      D1   PD15
      D2   PD0
      D3   PD1

      D4   PE7
      D5   PE8
      D6   PE9
      D7   PE10
      D8   PE11
      D9   PE12
      D10  PE13
      D11  PE14
      D12  PE15

      D13  PD8
      D14  PD9
      D15  PD10

      NOE  PD4
      NWE  PD5
      NE1  PD7
      A18 PD13
     */

#define FSMC_PD (GPIO4 | GPIO5 | GPIO7 | GPIO13 | GPIO14 | GPIO15 | GPIO0 | GPIO1 | GPIO8 | GPIO9 | GPIO10)
#define FSMC_PE (GPIO7 | GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12 | GPIO13 | GPIO14 | GPIO15)

    gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, FSMC_PD);
    gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, FSMC_PD);
    gpio_set_af(GPIOD, GPIO_AF12, FSMC_PD);

    gpio_mode_setup(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, FSMC_PE);
    gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, FSMC_PE);
    gpio_set_af(GPIOE, GPIO_AF12, FSMC_PE);

    /*
    FSMC_BTR(0) = FSMC_BTR_ACCMODx(FSMC_BTx_ACCMOD_B) | 
                FSMC_BTR_DATLATx(0) |
                FSMC_BTR_CLKDIVx(0) |
                FSMC_BTR_BUSTURNx(0) |
                FSMC_BTR_DATASTx(5) |
                FSMC_BTR_ADDHLDx(0) |
                FSMC_BTR_ADDSETx(1);
     */

    /* FSMC_BCR(0) = FSMC_BCR_WREN | FSMC_BCR_MWID | FSMC_BCR_MBKEN; */

    fsmc_set_access_mode(BANK1, FSMC_BTx_ACCMOD_B);
    fsmc_set_data_latency(BANK1, 0);
    fsmc_set_clock_divide_ratio(BANK1, 0);
    fsmc_set_turnaround_phase_duration(BANK1, 0);
    fsmc_set_data_phase_duration(BANK1, 5);
    fsmc_set_address_hold_phase_duration(BANK1, 0);
    fsmc_set_address_setup_phase_duration(BANK1, 1);


    fsmc_address_data_multiplexing_disable(BANK1);
    fsmc_write_burst_disable(BANK1);
    fsmc_wrapped_burst_mode_disable(BANK1);
    fsmc_extended_mode_disable(BANK1);
    fsmc_write_enable(BANK1);

    fsmc_wait_signal_polarity_low(BANK1);
    fsmc_wait_timing_configuration_enable(BANK1);
    fsmc_wait_disable(BANK1);
    fsmc_memory_bank_enable(BANK1);
}


void demo_gpio_setup(void) {
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);
}

int main(void) {
    clock_setup();
    systick_setup();
    io_setup();
    usart_setup();
    tim2_setup();

    delay(10);

    fsmc_setup();
    lcd_setup();
    lcd_clear();
    lcd_clear();

    console_xyputs(&console, 0, 0,  "STM32-F4 CONSOLE V0.1\r\n");
    console_xyputs(&console,  1, 0,  "SYS READY>");

    uint32_t i = 1;
    while (1) {

        #define STR_LEN 16
        uint8_t str[STR_LEN + 1];


        snprintf(str, STR_LEN, "0x%08X", i);
        console_xyputs(&console, 16, 0, str);

        uint16_t y_value = i %  LCD_HEIGHT;
        uint16_t y_prev;

        uint16_t x_value = i % LCD_WIDTH;
        uint16_t x_prev;

        if ((x_prev != x_value) && (y_prev != y_value)) {
            lcd_write_rect(20, y_prev, 10, 10, LCD_BLUE);
            lcd_write_rect(20, y_value, 10, 10, LCD_GREEN);
        }
        y_prev = y_value;
        x_prev = x_value;

        delay(10);

        i++;
    }
    return 0;
}

/* EOF */
