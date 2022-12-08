/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <info@gerhard-bertelsmann.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gerhard Bertelsmann
 * ----------------------------------------------------------------------------
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/serial.h>
#include <time.h>

#define XPN_SPEED	62500
char *interface = "/dev/ttyUSB2";

void print_usage(char *prg) {
    fprintf(stderr, "\nUsage: %s -i <interface>\n", prg);
    fprintf(stderr, "   Version 0.1\n\n");
    fprintf(stderr, "         -i <interface> - default %s\n", interface);
}

int time_stamp(char *timestamp) {
    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    sprintf(timestamp, "%02d:%02d:%02d.%03d", tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tv.tv_usec / 1000);
    return 0;
}

int xpn_send(int fd, struct termios *config, unsigned char *data, int length) {
    int ret;

    /* use parity mark for address */
    config->c_cflag |= PARENB | CMSPAR | PARODD;
    // ioctl(fd, TCSANOW, config);
    ioctl(fd, TCSETS, config);
    ret = write(fd, data, 1);
    if (ret < 0) {
	fprintf(stderr, "can't write address - %s\n", strerror(errno));
	return EXIT_FAILURE;
    }

    /* use parity space for data */
    config->c_cflag &= ~PARODD;
    // ioctl(fd, TCSANOW, config);
    ioctl(fd, TCSETS, config);
    ret = write(fd, data + 1, length - 1);
    if (ret < 0) {
	fprintf(stderr, "can't write data - %s\n", strerror(errno));
	return EXIT_FAILURE;
    }
    //ioctl(fd, TCSANOW, config);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    int fd, opt;

    unsigned char data[] = { 0xff, 0x20, 0x55, 0x55 };

    struct termios termios_config;
    struct serial_struct serinfo;

    memset(&termios_config, 0, sizeof(struct termios));

    while ((opt = getopt(argc, argv, "i:h?")) != -1) {
	switch (opt) {
	case 'i':
	    interface = optarg;
	    break;
	case 'h':
	case '?':
	    print_usage(basename(argv[0]));
	    exit(EXIT_SUCCESS);
	    break;
	default:
	    fprintf(stderr, "Unknown option %c\n", opt);
	    print_usage(basename(argv[0]));
	    exit(EXIT_FAILURE);
	}
    }

    fd = open(interface, O_RDWR);
    if (fd < 0) {
	fprintf(stderr, "Failed to open %s - %s\n", interface, strerror(errno));
	exit(EXIT_FAILURE);
    }

    serinfo.reserved_char[0] = 0;
    if (ioctl(fd, TIOCGSERIAL, &serinfo) < 0) {
	fprintf(stderr, "can't get serial info - %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }
    serinfo.flags &= ~ASYNC_SPD_MASK;
    serinfo.flags |= ASYNC_SPD_CUST;
    serinfo.custom_divisor = (serinfo.baud_base + (XPN_SPEED / 2)) / XPN_SPEED;
    if (serinfo.custom_divisor < 1)
	serinfo.custom_divisor = 1;
    if (ioctl(fd, TIOCSSERIAL, &serinfo) < 0) {
	fprintf(stderr, "can't set devisor - %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }
    if (ioctl(fd, TIOCGSERIAL, &serinfo) < 0) {
	fprintf(stderr, "can't get serial info - %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }
    printf(" actual rate %d", serinfo.baud_base/serinfo.custom_divisor);

    // config.c_cflag &= ~CBAUD;
    tcgetattr(fd, &termios_config);
    cfsetispeed(&termios_config, B38400);
    cfsetospeed(&termios_config, B38400);
    cfmakeraw(&termios_config);

    termios_config.c_cflag |= CS8 | PARENB | CMSPAR | PARODD;
    if (ioctl(fd, TCSETS, &termios_config) < 0) {
	fprintf(stderr, "can't set speed - %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }
    xpn_send(fd, &termios_config, data, 4);

    return 0;
}
