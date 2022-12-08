/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2012 Karl Palsson <karlp@tweak.net.au>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <stdio.h>
#include <errno.h>
#include "ILI9341.h"

int _write(int file, char *ptr, int len);

static void clock_setup(void)
{
	/* We are running on MSI after boot. */
	/* Enable GPIOD clock for LED & USARTs. */
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]); // F103
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);

	/* Enable clocks for USART2. */
	rcc_periph_clock_enable(RCC_USART1);
}

static void usart_setup(void)
{
	/* Setup USART2 parameters. */
	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(USART1);
}

static void gpio_setup(void)
{
		
#if defined STM32F1
    #warning Compile for STM32F1
	/* Setup GPIO pin GPIO7 on GPIO port B for Green LED. */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, GPIO7);  //F103
	gpio_set_mode(DC_PORT, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, DC_PIN);  //F103
	gpio_set_mode(RESET_PORT, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, RESET_PIN);  //F103
	gpio_set_mode(CS_PORT, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, CS_PIN);  //F103
	gpio_set_mode(BL_PORT, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,  BL_PIN);  //F103


	/* Setup GPIO pins for USART2 transmit. */

	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);  //F103
	
#elif defined STM32L1
	#warning Compile for STM32L1
	/* Setup GPIO pin GPIO7 on GPIO port B for Green LED. */
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);  // L152
	gpio_mode_setup(DC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DC_PIN);
	gpio_mode_setup(RESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RESET_PIN);
	gpio_mode_setup(CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CS_PIN);
	gpio_mode_setup(BL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BL_PIN);

	/* Setup GPIO pins for USART2 transmit. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);  // L152

	/* Setup USART2 TX pin as alternate function. */
	gpio_set_af(GPIOA, GPIO_AF7, GPIO9); // L152

#else
#error "Nieznany Typ Procesora."
#endif
}



int _write(int file, char *ptr, int len)
{
	int i;

	if (file == 1) {
		for (i = 0; i < len; i++)
			usart_send_blocking(USART1, ptr[i]);
		return i;
	} // if (file == 1)

	errno = EIO;
	return -1;
}

int main(void)
{
	int i;

	clock_setup();
	gpio_setup();
	usart_setup();
	spi_setup();
	
	printf("START\r\n");
	
	TFT_BL_ON;
	    for (i = 0; i < 100000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}
	TFT_BL_OFF;
		
	printf("Wyświetlacz wyłączony\r\n"); 
	printf("Stworzono: %s. Czas: %s. Plik: %s. Wiersz: %i.n",__DATE__,__TIME__,__FILE__,__LINE__);
	
	gpio_set(GPIOB, GPIO7); /* LED on/off */
	TFTinit();
	fillScreenALL();
		gpio_clear(GPIOB, GPIO7); /* LED on/off */

		for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}
		
		drawHorizontalLine( 200, 60,10,GRAY1);
		for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}
		setPixel( 310, 200, RED);
		for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}
		setPixel( 100, 200, YELLOW );
		for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}
				
		fillRectangle(50, 100, 10, 10, GREEN);
		
		for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}
		fillRectangle(200, 10, 50, 50, BLUE);
				for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}
		fillRectangle(190, 210, 10,30,WHITE);
				for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}
		fillRectangle(0, 0, 10,30,YELLOW);
		for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}
		
		//drawLine( uint16_t x0,uint16_t y0,uint16_t x1, uint16_t y1,uint16_t color);
		drawLine( 200,200,100, 100, GREEN);
		for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}		
		
		//void drawCircle(uint16_t  poX, uint16_t  poY, uint16_t  r,uint16_t  color);
		drawCircle(90, 50, 30, RED);
		for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}			
		//void fillCircle(uint16_t  poX, uint16_t  poY, uint16_t  r,uint16_t  color);
		fillCircle(150, 50, 50, GRAY2);
		for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}	
		
		//void drawChar( uint8_t ascii, uint16_t poX, uint16_t poY,uint16_t size, uint16_t fgcolor);
		//drawChar( 'A',100, 100,6, RED);
		drawString("TEST",20, 140, 3, GREEN);
		for (i = 0; i < 10000; i++) { /* Wait a bit. */
			__asm__("NOP");
		}

		
		while (1) {
		
	}

	return 0;
}
