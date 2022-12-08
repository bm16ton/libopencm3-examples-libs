/*
 * sd.c - Secure Digital API functions
 */

#include <stdint.h>
#include "sd.h"
#include <libopencm3/stm32/sdio.h>
/*
 * sd_bus
 *
 * Set the bus width and the clock speed for the
 * SDIO bus.
 *
 * Returns 0 on success
 *      -1 illegal bit specification
 *      -2 illegal clock specification
 */
int
sd_bus(int bits, enum SD_CLOCK_DIV freq) {
    int clkreg = 0;

    switch (bits) {
        case 1:
            clkreg |= SDIO_CLKCR_WIDBUS_1;
            break;
        case 4:
            clkreg |= SDIO_CLKCR_WIDBUS_4;
            break;
        default:
            return -1;
    }
    switch (freq) {
        case CLOCK_24MHZ:
            break;
        case CLOCK_16MHZ:
            clkreg |= 1;
            break;
        case CLOCK_12MHZ:
            clkreg |= 2;
            break;
        case CLOCK_8MHZ:
            clkreg |= 8;
            break;
        case CLOCK_4MHZ:
            clkreg |= 10;
            break;
        case CLOCK_1MHZ:
            clkreg |= 46;
            break;
        case CLOCK_400KHZ:
            clkreg |= 118;
            break;
        default:
            return -2;
    }
    clkreg |= SDIO_CLKCR_CLKEN;
    SDIO_CLKCR = clkreg;
    return 0;
}
