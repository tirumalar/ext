/*
 * Interface to the adp8860 charge pump for the eyelock RGB leds 
 * 
 * All functions return zero on success, negative value on failure.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/i2c-dev.h> 

#include "adp8860.h"
#include "adp8860-regs.h"

static char _i2c_bus[] = I2C_BUS;

static int internal_read_reg(int fd, unsigned char reg, unsigned char *val);
static int internal_write_reg(int fd, unsigned char reg, unsigned char val);
static int i2c_start_transaction();


int rgb_led_read_reg(unsigned char reg, unsigned char *val)
{
	int fd, result;

	fd = i2c_start_transaction();
	if (fd < 0)
		return -1;

	result = internal_read_reg(fd, reg, val);

	close(fd);

	return result;
}

int rgb_led_write_reg(unsigned char reg, unsigned char val)
{
	int fd, result;

	fd = i2c_start_transaction();
	if (fd < 0)
		return -1;

	result = internal_write_reg(fd, reg, val);

	close(fd);

	return result;
}

int rgb_led_enable(int enable)
{
	int fd, result;

	fd = i2c_start_transaction();
	if (fd < 0)
		return -1;

	if (!enable) {
		/* reg 0x01 = 0x00 */
		result = internal_write_reg(fd, ADP_MDCR_REG, 0);
	}
	else {
		/* reg 0x10 = 0x00 */
		result = internal_write_reg(fd, ADP_ISCC_REG, 0);
		if (result < 0) 
			goto rgb_enable_done;

		/* reg 0x01 = 0x21 */
		result = internal_write_reg(fd, ADP_MDCR_REG, MDCR_nSTBY |  MDCR_BLEN);
		if (result < 0) 
			goto rgb_enable_done;

		/* reg 0x05 = 0x07 */
		result = internal_write_reg(fd, ADP_BLSEN_REG, BLSEN_D1EN | BLSEN_D2EN | BLSEN_D3EN);
		if (result < 0)
			goto rgb_enable_done;

		/* reg 0x10 = 0x07 */
		result = internal_write_reg(fd, ADP_ISCC_REG, ISCC_SC1_EN | ISCC_SC2_EN | ISCC_SC3_EN);
		if (result < 0) 
			goto rgb_enable_done;
	}

	result = 0;

rgb_enable_done:

	close(fd);

	return result;
}

int rgb_led_set_color(unsigned char r, unsigned char g, unsigned char b)
{
	int fd, result;

	if (r == 0 && g == 0 && b == 0)
		return rgb_led_enable(0);

	fd = i2c_start_transaction();
	if (fd < 0)
		return -1;


	/* D3 is red */
	result = internal_write_reg(fd, ADP_ISC3_REG, ISC_VAL(r));
	if (result < 0)
		goto rgb_set_color_done;

	/* D2 is green */
	result = internal_write_reg(fd, ADP_ISC2_REG, ISC_VAL(g));
	if (result < 0)
		goto rgb_set_color_done;

	/* D1 is blue */		
	result = internal_write_reg(fd, ADP_ISC1_REG, ISC_VAL(b));
	if (result < 0)
		goto rgb_set_color_done;

	result = 0;

rgb_set_color_done:

	close(fd);

	return result;
}

/*====================== Internal Static =======================*/

int internal_read_reg(int fd, unsigned char reg, unsigned char *val)
{
	int result;
	unsigned char buff[2];

	buff[0] = reg;

	result = write(fd, buff, 1);
	if (result != 1) {
		perror("write");
		return -1;
	}

	buff[0] = 0;

	result = read(fd, buff, 1);
	if (result != 1) {
		perror("read");
		return -1;
	}

	if (val)
		*val = buff[0];

	return 0;
}

int internal_write_reg(int fd, unsigned char reg, unsigned char val)
{
	int result;
	unsigned char buff[2];

	buff[0] = reg;
	buff[1] = val;
	
	result = write(fd, buff, 2);

	if (result != 2)
		perror("write");
	else
		result = 0;

	return result;
}

int i2c_start_transaction()
{
	int fd, result;

	fd = open(_i2c_bus, O_RDWR);
	if (fd < 0) {
		perror("open");
		return 0;
	}

	result = ioctl(fd, I2C_SLAVE, ADP8860_I2C_ADDR);
	if (result < 0) {
		perror("ioctl");
		close(fd);
		return 0;
	}

	return fd;
}

