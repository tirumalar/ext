/*
 * I2CBus.h
 *
 *  Created on: Feb 1, 2012
 *      Author: dhirvonen
 */

#ifndef I2CBUS_H_
#define I2CBUS_H_

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

#include <string>

// http://en.wikipedia.org/wiki/Singleton_pattern#C.2B.2B
template<typename T> class SingletonAPI  /* TODO: Relocate GRITrigger/Singleton.h in repository */
{
  public:
    static T& instance()
    {
        static T theSingleInstance;  // assumes T has a protected default constructor
        return theSingleInstance;
    }
	virtual void init() {}
	virtual void term() {}
};


// Use Singleton::instance method to access
class I2CBus
{
public:
	I2CBus(const char *name);
	virtual ~I2CBus();
	int Read(unsigned char reg, unsigned int *val);
	int Write(unsigned char reg, unsigned int val);
	int Open();
	int Assign(int address);
	int Close();
protected:
	int m_fd;
	std::string m_I2CBus;
};

#define NANO_LED_I2C_ADDR0 0x2e // 46
#define NANO_BOB_I2C_ADDR 0x38
#define ACCESS_SPEED_ADDRESS 3
#define ACCESS_DATA0_ADDRESS 3
#define ACCESS_DATA1_ADDRESS 23
#define BOB_COMMAND_ADDRESS 0
#define BOB_COMMAND_ACCESS_WRITE 5
#define ACCESS_CONTROL_ADDRESS 			47
#define DUAL_AUTHENTICATION_ADDRESS		48
#define DUAL_MATCHING_ADDRESS			49
#define DUAL_ACCESS_DATA1_ADDRESS 		50
#define DUAL_CARD_DATA_READY			73
#define LED_IN_READY					74
#define SOUND_IN_READY					75
#define BOB_COMMAND_PAC_WRITE			10
#define BOB_COMMAND_WIGHID_WRITE		12


class I2CBusNano : public I2CBus
{
public:
	I2CBusNano();
	~I2CBusNano();
	pthread_mutex_t &GetLock() { return m_Lock; }
	pthread_mutex_t m_Lock;
};

typedef SingletonAPI<I2CBusNano> I2CBusNanoAPI;


#endif /* I2CBUS_H_ */
