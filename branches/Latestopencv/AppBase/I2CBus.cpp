/*
 * I2CBus.cpp
 *
 *  Created on: Feb 1, 2012
 *      Author: dhirvonen
 */

#include "I2CBus.h"

I2CBus::I2CBus(const char *name) : m_I2CBus(name)
{
}

I2CBus::~I2CBus()
{
}

int I2CBus::Read(unsigned char reg, unsigned int *val)
{
	int result;
	unsigned char buff[2];

	buff[0] = reg;

	result = write(m_fd, buff, 1);
	if (result != 1) {
		perror("write");
		return -1;
	}

	buff[0] = 0;

	result = read(m_fd, buff, 1);
	if (result != 1) {
                printf("error \n");
		perror("read");
		return -1;
	}

	if (val)
		*val = buff[0];

	return 0;
}

int I2CBus::Write(unsigned char reg, unsigned int val)
{
	int result;
	unsigned char buff[2];

	buff[0] = reg;
	buff[1] = val;

	result = write(m_fd, buff, 2);

	if (result != 2)
		perror("write");
	else
		result = 0;

	return result;
}

// return -1 on error, otherwise return FD
int I2CBus::Open()
{
	int result;
	m_fd = open(m_I2CBus.c_str(), O_RDWR);
	if (m_fd < 0) {
		perror("open");
	}
	return m_fd;
}

// return -1 on error, otherwise return 0
int I2CBus::Assign(int address)
{
	int result = ioctl(m_fd, I2C_SLAVE, address);
	if (result < 0) {
		perror("ioctl");
		close(m_fd);
	}
	return result;
}

int I2CBus::Close()
{
	close(m_fd);
}

// Nano specialization
#ifdef CMX_C1
#define I2C_BUS_NANO "/dev/i2c-1"
#else
#define I2C_BUS_NANO "/dev/i2c-3"
#endif


I2CBusNano::I2CBusNano() : I2CBus(I2C_BUS_NANO)
{
	Open();
	pthread_mutex_init(&m_Lock, 0);
}

I2CBusNano::~I2CBusNano()
{
	pthread_mutex_destroy(&m_Lock);
}
