#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

// peripheral register physical address
#define GPIO_PHY_BASEADDR  0x3F200000
#define GPIO_AREA_SIZE	4096	// PAGE_SIZE
#define GPIO_GPFSEL0	0x0000	// for gpio 0..9, MSB 2bits are reserved
// omit GPFSEL1..GPFSEL5
#define GPIO_GPSET0	0x001C	// gpio 0..31
#define GPIO_GPSET1	0x0020	// gpio 32..53
#define GPIO_GPCLR0	0x0028	// gpio 0..31
#define GPIO_GPCLR1	0x002C	// gpio 32..53
#define GPIO_GPLEV0	0x0034	// gpio 0..31
#define GPIO_GPLEV1	0x0038	// gpio 32..53

void ledOnOff();
unsigned int memread(void *baseaddr, int offset);
void memwrite(void *baseaddr, int offset, unsigned int x);

void *gpio_baseaddr;

void ledOnOff()
{
    unsigned int gpfsel0;
    gpfsel0= memread(gpio_baseaddr, GPIO_GPFSEL0);	// get GPIO5 mode
    gpfsel0 = gpfsel0 | (1<<15);		// 15=GPIO5*3, bit15 ON
    memwrite(gpio_baseaddr, GPIO_GPFSEL0, gpfsel0);	// GPIO5 output mode
    memwrite(gpio_baseaddr, GPIO_GPSET0, (1 << 5));	// GPIO5 high
    sleep(1);
    memwrite(gpio_baseaddr, GPIO_GPCLR0, (1 << 5));	// GPIO5 low
}

unsigned int memread(void *baseaddr, int offset)
{
    unsigned int *p;
    p = baseaddr+offset;
    return *p;	// read memory-mapped register
}

void memwrite(void *baseaddr, int offset, unsigned int x)
{
    unsigned int *p;
    p = baseaddr+offset;
    *p = x;	// write memory-mapped register
}
