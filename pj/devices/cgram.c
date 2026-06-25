// write LCD for Apple Pi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int lcd_cmdwrite(int fd, unsigned char dat)
{
	unsigned char buff[2];
	buff[0] = 0;
	buff[1] = dat;
	return write(fd,buff,2);
}

int lcd_datawrite(int fd, char dat[], int len)
{
	char buff[100];
	if (len>99) {printf("too long string\n"); exit(1); }
	memcpy(buff+1, dat, len);
	buff[0] = 0x40;	// DATA Write command
	return write(fd, buff, len+1);
}

void initLCD(int fd)
{
	int i;
	unsigned char init1[]={ 0x38, 0x39, 0x14, 0x70, 0x56, 0x6c };
	unsigned char init2[]={ 0x38, 0x0c, 0x01 };

	usleep(100000);	// wait 100ms
	for (i=0;i<sizeof(init1)/sizeof(unsigned char);i++) {
		if(lcd_cmdwrite(fd, init1[i])!=2){
			printf("internal error1\n");
			exit(1);
		}
		usleep(50); // wait 50us
	}

	usleep(300000);	// wait 300ms

	for (i=0;i<sizeof(init2)/sizeof(unsigned char);i++) {
		if(lcd_cmdwrite(fd, init2[i])!=2){
			printf("internal error2\n");
			exit(1);
		}
		usleep(50);
	}
	usleep(2000);	// wait 2ms
}

void setCGRAM(int fd)
{
	lcd_cmdwrite(fd, 0x40);		// define char 0
	lcd_datawrite(fd, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
	lcd_cmdwrite(fd, 0x48);		// define char 1
	lcd_datawrite(fd, "\x1f\x00\x00\x1f\x00\x00\x00\x00", 8);
	lcd_cmdwrite(fd, 0x50);		// define char 2
	lcd_datawrite(fd, "\x1f\x00\x00\x00\x00\x00\x1f\x00", 8);
	lcd_cmdwrite(fd, 0x58);		// define char 3
	lcd_datawrite(fd, "\x00\x00\x00\x1f\x00\x00\x1f\x00", 8);
	lcd_cmdwrite(fd, 0x60);		// define char 4
	lcd_datawrite(fd, "\x1f\x00\x00\x1f\x00\x00\x1f\x00", 8);
	lcd_cmdwrite(fd, 0x68);		// define char 5
	lcd_datawrite(fd, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
}

void printLCD(int fd, char buff[])
{
	int i, len;
	len = strlen(buff);
	if (lcd_datawrite(fd, buff, len) != len+1) {
		printf("internal error4\n");
		exit(1);
	}
}

int main()
{

	int i2c, i, j;
	char ch=0, buff[]="\xff-_\x01\x02\x03\x04\x05";

	if( (i2c=open("/dev/i2c-1", O_RDWR)) <0 ){
		perror("open");
		return 1;
	}

	// set slave device's address to 0x3E (LCD)
	if (ioctl(i2c, I2C_SLAVE, 0x3e)<0) {
		perror("ioctl");
		return 1;
	}

	initLCD(i2c);

	setCGRAM(i2c);

	lcd_cmdwrite(i2c, 0x80);
	for (j=0; j<8; j++) {
		lcd_datawrite(i2c, &(buff[j]), 1);
	}
	sleep(3);

	for (i=0; i<30; i++) {
		lcd_cmdwrite(i2c, 0x80);
		for (j=((i+1)%8); j!=(i%8); j=(j+1)%8) {
			lcd_datawrite(i2c, &(buff[j]), 1);
		}
		usleep(100000);
	}

	close(i2c);
	return 0;
}
