/*
 * GPIODriver.cpp
 *
 *  Created on: Feb 18, 2013
 *      Author: mamigo
 */

#include "GPIODriver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>


GPIODriver::GPIODriver(unsigned int gpio):m_gpio(gpio) {
	int fd, len;
	char buf[11];
	printf("Trying to get GPIO %d \n",m_gpio);
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		//return fd;
	}
	len = snprintf(buf, sizeof(buf), "%d", m_gpio);
	write(fd, buf, len);
	close(fd);
}

GPIODriver::~GPIODriver() {
	int fd, len;
	char buf[11];

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		//return fd;
	}
	len = snprintf(buf, sizeof(buf), "%d", m_gpio);
	write(fd, buf, len);
	close(fd);
}

int GPIODriver::GPIODir(unsigned int dir)
{
	int fd, len;
	char buf[60];

	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", m_gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}

	if (dir == GPIO_DIR_OUT)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	return 0;
}

int GPIODriver::SetGPIODirOut()
{
	printf("Setting  GPIO %d HIGH \n",m_gpio);
	return GPIODir(GPIO_DIR_OUT);
}

int  GPIODriver::SetGPIODirIn()
{
	return GPIODir(GPIO_DIR_IN);
}

int GPIODriver::SetGPIOValue(unsigned int value){
	int fd, len;
	char buf[60];
	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", m_gpio);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/value");
		return fd;
	}
	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}

