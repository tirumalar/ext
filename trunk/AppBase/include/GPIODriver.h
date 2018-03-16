/*
 * GPIODriver.h
 *
 *  Created on: Feb 18, 2013
 *      Author: mamigo
 */

#ifndef GPIODRIVER_H_
#define GPIODRIVER_H_

#define GPIO_DIR_IN		0
#define GPIO_DIR_OUT	1


class GPIODriver {
public:
	GPIODriver(unsigned int gpio);
	virtual ~GPIODriver();
	int SetGPIODirOut();
	int SetGPIODirIn();
	int SetGPIOValue(unsigned int value);
private:
	unsigned int m_gpio;
	int GPIODir(unsigned int dir);
};


#endif /* GPIODRIVER_H_ */
