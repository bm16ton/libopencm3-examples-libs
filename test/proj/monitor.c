/*
 * Simple monitor for the EMBEST Baseboard + STM32F4-Discovery
 * board. 
 */

#include <stdint.h>
#include <libopencm3/stm32/sdio.h>
#include <libopencm3/cm3/scb.h>
#include "bb.h"
#include "debug.h"

#include "../openfat/include/openfat.h"
#include "../openfat/include/openfat/mbr.h"

//#include "../logger/logger.h"
//#include "../sddriver/sddriver.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
/* 
 * Not sure I love this design, but this integer is the 'console'
 * port. That is the port where all the serial I/O is to be sent
 * or 'standard out' in UNIX/Linux parlance. It is defined as a global
 * here because it is impractical to be sending all around, much like
 * the file descriptor for standard out is global for the same reason.
 * It also means that if someone or something stomps on it then your
 * code will stop writing to the serial port you expected.
 */
int console = 0;

const char *greet = "\nARM Baseboard Monitor v0.01\nEnter Command, ? or H for help.\n";

int sdDrvReadSector(const struct block_device *bldev, uint32_t sector, uint32_t count, void *buf2);
int sdDrvWriteSector(const struct block_device *bldev, uint32_t sector, uint32_t count, const void *buf2);

void do_cmd(int);
#define is_hex(c) ((((c) >= '0') && ((c) <= '9')) ||\
                   (((c) >= 'a') && ((c) <= 'f')) ||\
                   (((c) >= 'A') && ((c) <= 'F')))

#define CMD_READ_SD     1
#define CMD_WRITE_SD    2
#define CMD_IDENT_SD    3
void do_cmd(int cmd) {
    switch (cmd) {
        case CMD_READ_SD:
        case CMD_WRITE_SD:
        case CMD_IDENT_SD:
            break;
        default:
            uart_puts(console, (const char *)"Unrecognized command\n");
    }
}

SDIO_CARD my_card;
uint8_t blk_read_buf[512];
char buf[256];

int sdDrvReadSector(const struct block_device *bldev, uint32_t sector, uint32_t count, void *buf2)
{
    int err;
    (void)bldev;
    (void)sector;
    (void)count;
    my_card = sdio_open();
    err = sdio_readblock(my_card, 0, buf2);
    return err;
}
 
int sdDrvWriteSector(const struct block_device *bldev, uint32_t sector, uint32_t count, const void *buf2)
{
int err;
uint32_t blk;
unsigned char* buf3 = (unsigned char*)buf2;
    (void)bldev;
    (void)sector;
    (void)count;
     my_card = sdio_open();
     blk = uart_getnum(console);
    err = sdio_writeblock(my_card, blk, buf3);
    return err;
}
    

#define valid_addr(x) \
    ((((uint32_t) x >= 0x10000000) && ((uint32_t) x < 0x10010000)) ||\
     (((uint32_t) x >= 0x20000000) && ((uint32_t) x < 0x20020000)) ||\
     (((uint32_t) x >= 0x08000000) && ((uint32_t) x < 0x08100000)))

#define MIN_ADDR (uint8_t *)(0x20000000)
int
main(void)
{
    char c;
    uint8_t *addr;
	struct block_mbr_partition part;
	struct block_device dev;
	struct fat_vol_handle volHandle;
	struct fat_file_handle fileHandle;

	char statusBuff[30];

	char dirname[20];
	char filename[20];
	char buffer[2000];
	
    SCB_VTOR = (uint32_t) 0x08080000;

    // setup 115,200 baud
    bb_setup(115200);
    debug_init();
    debug_puts("\nBBMON: Debug Channel\n");
    
    
	// Initialize OpenFAT block device structure with SD driver functions.
	dev.read_sectors = sdDrvReadSector;
	dev.write_sectors = sdDrvWriteSector;
	dev.get_sector_size = sdio_bit_slice(buf, 512, 431, 428);;
	
		if(mbr_partition_init(&part, &dev, 0) == 0)
	{
            uart_puts(console, "mbr_partition_init successful");
	}
	else
	{
            uart_puts(console, "mbr_partition_init fail");
            while(1);
	}

	// Initialize and mount the FAT volume.
	if(fat_vol_init((struct block_device *)&part, &volHandle) == 0)
	{
            uart_puts(console, "fat_vol_init successful");
            sprintf(statusBuff, "FAT type = %d", volHandle.type);
            uart_puts(console, statusBuff);
	}
	else
	{
            uart_puts(console, "fat_vol_init fail");
            while(1);
	}
	
		for(int i = 0; i < 5; i++) 
	{
            sprintf(dirname, "Test%d", i);
            fat_mkdir(&volHandle, dirname);
            assert(fat_chdir(&volHandle, dirname) == 0);

		for(int j = 0; j < 5; j++) 
		{
                    sprintf(filename, "File%d", j);
                    assert(fat_create(&volHandle, filename, O_WRONLY, &fileHandle) == 0);
                    assert(fat_write(&fileHandle, buffer, sizeof(buffer)) == sizeof(buffer));
		}

            assert(fat_chdir(&volHandle, "..") == 0);
	}	

	
    text_color(console, DEFAULT);
    clear_screen(console);
    move_cursor(console, 1,1);
    uart_puts(console, (const char *)greet);
    uart_puts(console, "Endian test : ");
    uart_putnum(console, FMT_HEX_CONSTANT, 0xaabbccdd);
    uart_puts(console, "\n");
    uart_puts(console, "Greeting is at : 0x");
    uart_putnum(console, FMT_HEX_CONSTANT, (uint32_t)(greet));
    uart_puts(console, "\n");
    addr = MIN_ADDR;
    move_cursor(console, 13, 1);
    addr = dump_page(console, addr, NULL);
    addr = dump_page(console, addr, NULL);
    /* really should go into command line mode here */
    while(1) {
        move_cursor(console, 11, 1);
        uart_puts(console, "Enter Command:                                ");
        move_cursor(console, 11, 16);
        c = uart_getc(console, 1);
        switch (c) {
            case 'd':
                uart_puts(console, "dump (address) :");
                addr = (uint8_t *)uart_getnum(console);
                addr = (valid_addr(addr)) ? addr : MIN_ADDR;
                move_cursor(console, 13, 1);
                addr = dump_page(console, addr, NULL);
                addr = dump_page(console, addr, NULL);
                break;
            case '\r':
                uart_puts(console, "\n");
                move_cursor(console, 13, 1);
                addr = dump_page(console, addr, NULL);
                addr = dump_page(console, addr, NULL);
                break;
            case 'a':
                uart_puts(console, "address: ");
                addr = (uint8_t *)uart_getnum(console);
                addr = (valid_addr(addr)) ? addr : MIN_ADDR;
                break;
            case 's':
                sdio_explorer();
                break;
            default:
                uart_puts(console, "?\n");
        }
	}
	return 0;
}
