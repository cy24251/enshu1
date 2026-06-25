#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>


#define SLAVE_ADDRESS   0x76

// see BST-BME280-DS001-10
#define	BME280_S32_t	signed int
#define	BME280_U32_t	unsigned int
#define	BME280_S64_t	long long signed int

#define	HUM_LSB	0XFE
#define	HUM_MSB	0XFD
#define	TEMP_XLSB	0XFC
#define	TEMP_LSB	0XFB
#define	TEMP_MSB	0XFA
#define	PRESS_XLSB	0XF9
#define	PRESS_LSB	0XF8
#define	PRESS_MSB	0XF7
#define	CONFIG		0XF5
#define	CTRL_MEAS	0XF4
#define	STATUS		0XF3
#define	CTRL_HUM	0XF2
#define	RESET		0XE0
#define	ID		0XD0
#define	DIG_T1		0x88
#define	DIG_T2		0x8A
#define	DIG_T3		0x8C
#define	DIG_P1		0x8E
#define	DIG_P2		0x90
#define	DIG_P3		0x92
#define	DIG_P4		0x94
#define	DIG_P5		0x96
#define	DIG_P6		0x98
#define	DIG_P7		0x9A
#define	DIG_P8		0x9C
#define	DIG_P9		0x9E
#define	DIG_H1		0xA1
#define	DIG_H2		0xE1
#define	DIG_H3		0xE3
#define	DIG_H4		0xE4
#define	DIG_H6		0xE7

unsigned short dig_T1;
signed short dig_T2, dig_T3;
unsigned short dig_P1;
signed short dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
unsigned char dig_H1, dig_H3;
signed short dig_H2, dig_H4, dig_H5;
signed char dig_H6;

// Compensation functions provided by BOSCH (in BME280 datasheet, rev.1.1).
// Returns temperature in DegC, resolution is 0.01 DegC.
// Output value of "5123" equals 51.23 DecC.
// t_fine carries fine temperature as global value
BME280_S32_t t_fine;
BME280_S32_t BME280_compensate_T_int32(BME280_S32_t adc_T)
{
	BME280_S32_t var1, var2, T;
	var1  = ((((adc_T>>3) - ((BME280_S32_t)dig_T1<<1))) * ((BME280_S32_t)dig_T2)) >> 11;
	var2  = (((((adc_T>>4) - ((BME280_S32_t)dig_T1)) *
		   ((adc_T>>4) - ((BME280_S32_t)dig_T1))) >> 12) *
		 ((BME280_S32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T  = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format
// (24 integer bits and 8 fractional bits).
// Output value of Åg24674867Åh represents 24674867/256 = 96386.2 Pa = 963.862 hPa
BME280_U32_t BME280_compensate_P_int64(BME280_S32_t adc_P)  {
  BME280_S64_t var1, var2, p;
  var1 = ((BME280_S64_t)t_fine) - 128000;
  var2 = var1 * var1 * (BME280_S64_t)dig_P6;
  var2 = var2 + ((var1*(BME280_S64_t)dig_P5)<<17);
  var2 = var2 + (((BME280_S64_t)dig_P4)<<35);
  var1 = ((var1 * var1 * (BME280_S64_t)dig_P3)>>8) + ((var1 * (BME280_S64_t)dig_P2)<<12);
  var1 = (((((BME280_S64_t)1)<<47)+var1))*((BME280_S64_t)dig_P1)>>33;
  if (var1 == 0) {
    return 0; // avoid exception caused by division by zero
  }
  p = 1048576-adc_P;
  p = (((p<<31)-var2)*3125) / var1;
  var1 = (((BME280_S64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
  var2 = (((BME280_S64_t)dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)dig_P7)<<4);
  return (BME280_U32_t)p;
}
 
// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format
// (22 integer and 10 fractional bits).
// Output value of Åg47445Åh represents 47445/1024 = 46.333 %RH
BME280_U32_t bme280_compensate_H_int32(BME280_S32_t adc_H)  {
  BME280_S32_t v_x1_u32r;
 
  v_x1_u32r = (t_fine - ((BME280_S32_t)76800));
  v_x1_u32r = (((((adc_H<<14)-(((BME280_S32_t)dig_H4)<<20)-
		  (((BME280_S32_t)dig_H5)*v_x1_u32r))+ ((BME280_S32_t)16384)) >> 15) *
	       (((((((v_x1_u32r * ((BME280_S32_t)dig_H6)) >> 10) *
		    (((v_x1_u32r*((BME280_S32_t)dig_H3))>>11)+((BME280_S32_t)32768)))>>10)+
		  ((BME280_S32_t)2097152)) * ((BME280_S32_t)dig_H2) + 8192) >> 14));
  v_x1_u32r = (v_x1_u32r -
	       (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t)dig_H1)) >> 4));
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
  return (BME280_U32_t)(v_x1_u32r>>12);
}

// Write single-byte to I2C device
// fd: file descriptor
// addr: address to be written
// val: written value
// return: 2 if normal, -1 otherwise
int i2c_write(int fd, unsigned char addr, unsigned char val)
{
	unsigned char buff[2];
	buff[0]=addr;
	buff[1]=val;
	return write(fd,buff,2);
}

// Read Multiple-bytes from I2C device
// fd: file descriptor
// addr: first address to be read (address is auto incremented)
// ibuff: read data is stored
// len: number of bytes to be read
// return: always len (at the moment)
int i2c_mread(int fd, unsigned char addr, unsigned char ibuff[], int len)
{
	int ret;
	unsigned char buff[1];
	buff[0]=addr;
	ret = write(fd,buff,1);
	if(ret != 1){
		printf("internal error\n");
		exit(1);
	}
	ret = read(fd,ibuff,len);
	if (ret != len) {
		printf("internal error\n");
		exit(1);
	}
	return ret;
}

void wait_ready(int fd) {
	int val;
	unsigned char data[1];
	while (1) {
		i2c_mread(fd, STATUS, data, 1);
		val = 0x9 & data[0]; // get bit3, bit0
		if (val == 0) break;
		usleep(10000);
	}
}

void get_parameter(int fd)
{
	unsigned char data[3];

	i2c_mread(fd, DIG_T1, data, 2);
	dig_T1 = data[1]<<8 | data[0];	// unsigned short
	i2c_mread(fd, DIG_T2, data, 2);
	dig_T2 = data[1]<<8 | data[0];	// signed short
	i2c_mread(fd, DIG_T3, data, 2);
	dig_T3 = data[1]<<8 | data[0];	// signed short
	printf("T1=%d T2=%d T3=%d\n", dig_T1, dig_T2, dig_T3);
	i2c_mread(fd, DIG_P1, data, 2);
	dig_P1 = data[1]<<8 | data[0];	// unsigned short
	i2c_mread(fd, DIG_P2, data, 2);
	dig_P2 = data[1]<<8 | data[0];	// signed short
	i2c_mread(fd, DIG_P3, data, 2);
	dig_P3 = data[1]<<8 | data[0];	// signed short
	i2c_mread(fd, DIG_P4, data, 2);
	dig_P4 = data[1]<<8 | data[0];	// signed short
	i2c_mread(fd, DIG_P5, data, 2);
	dig_P5 = data[1]<<8 | data[0];	// signed short
	i2c_mread(fd, DIG_P6, data, 2);
	dig_P6 = data[1]<<8 | data[0];	// signed short
	i2c_mread(fd, DIG_P7, data, 2);
	dig_P7 = data[1]<<8 | data[0];	// signed short
	i2c_mread(fd, DIG_P8, data, 2);
	dig_P8 = data[1]<<8 | data[0];	// signed short
	i2c_mread(fd, DIG_P9, data, 2);
	dig_P9 = data[1]<<8 | data[0];	// signed short
	printf("P1=%d P2=%d P3=%d ", dig_P1, dig_P2, dig_P3);
	printf("P4=%d P5=%d P6=%d ", dig_P4, dig_P5, dig_P6);
	printf("P7=%d P8=%d P9=%d\n", dig_P7, dig_P8, dig_P9);
	i2c_mread(fd, DIG_H1, data, 1);
	dig_H1 = data[0];		// unsigned char
	i2c_mread(fd, DIG_H2, data, 2);
	dig_H2 = data[1]<<8 | data[0];	// signed char
	i2c_mread(fd, DIG_H3, data, 1);
	dig_H3 = data[0];		// unsigned char
	i2c_mread(fd, DIG_H4, data, 3);
	dig_H4 = data[0]<<4 | (data[1] & 0x0F);	// signed short
	dig_H5 = data[2]<<4 | (data[1] >> 4);	// signed short
	i2c_mread(fd, DIG_H6, data, 1);
	dig_H6 = data[0];		// signed char
	printf("H1=%d H2=%d H3=%d ", dig_H1, dig_H2, dig_H3);
	printf("H4=%d H5=%d H6=%d\n", dig_H4, dig_H5, dig_H6);
}

void initBME280(int i2c)
{
	wait_ready(i2c);	// maybe needless

	get_parameter(i2c);

	// set config to default
	if (i2c_write(i2c, CONFIG, 0x00)!=2) {
		perror("i2c_write");
		close(i2c);
		return;
	}

	// set ctrl_hum = hum oversampling x1
	if (i2c_write(i2c, CTRL_HUM, 0x01)!=2) {
		perror("i2c_write");
		close(i2c);
		return;
	}

	// set ctrl_meas : press and temp oversampline x1, forced mode
	if (i2c_write(i2c, CTRL_MEAS, 0x25)!=2) {
		perror("i2c_write");
		close(i2c);
		return;
	}

	// Writing "forced mode" to ctrl_meas causes new measurement.
	// So, wait until the completion of this measurement.
	wait_ready(i2c);
}

int main(int argc, char *argv[])
{

	int i2c, ret;
	int press, temp, hum;
	double temp_d;
	unsigned char data[3];

	i2c=open("/dev/i2c-1", O_RDWR);
	if(i2c < 0){
		perror("open");
		exit(1);
	}

	// set slave device's address
	ret = ioctl(i2c, I2C_SLAVE, 0x76);	// 0x76 = slave address
	if (ret < 0) {
		perror("ioctl");
		return 1;
	}

	initBME280(i2c);

	// get temperature
	i2c_mread(i2c, TEMP_MSB, data, 3);
	temp = (data[0]<<12) | (data[1]<<4) | (data[2]>>4);
	printf("%d\n", temp);
	temp_d = BME280_compensate_T_int32(temp)/100.0;
	printf("%f C\n", temp_d);

	// get pressure
	i2c_mread(i2c, PRESS_MSB, data, 3);
	press = (data[0]<<12) | (data[1]<<4) | (data[2]>>4);
	// do BME280_compensate_P_int64
	press = BME280_compensate_P_int64(press);
	printf("%f Pa\n", (double)press/256.0);

	// Ç±Ç±Çé©ï™Ç≈èëÇ≠Åiâ€ëËÅj
	// get Humidity

	close(i2c);
	return 0;
}
