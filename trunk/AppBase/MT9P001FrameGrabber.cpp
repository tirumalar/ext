/*
 * MT9P001FrameGrabber.cpp
 *
 *  Created on: Jan 8, 2011
 *      Author: developer1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "I2CBus.h"
#include "Parsing.h"
#include "MT9P001FrameGrabber.h"
#include "Configuration.h"
#include "CamConfig.h"
#include "ppifcd_i2c.h"
#include "Image16To8Conv.h"
#include "logging.h"
#include "UtilityFunctions.h"

const char logger[30] = "MT9P001FrameGrabber";

using namespace std;

#define MAX_IMAGE_WIDTH 	2592
#define MAX_IMAGE_HEIGHT	1944

//int noprintf( char *fmt, ... ) {}
//#define printf noprintf

#include "mt9p001-dev.h"
#include "mt9p001.h"
extern "C"{
#include "file_manip.h"
}

#if 0
#undef XTIME_OP
#define XTIME_OP(m, op) { \
        DEF_TIMER(m); \
        START_TIMER; \
        op;     \
        END_TIMER; \
        }
#endif

#include "socket.h"
#include "MessageExt.h"
#include "Safe.h"
#include <iostream>
#include "HThread.h"

/////// I2C stuff

#define NANO_LED_I2C_ADDR0 0x2e // 46
#define NANO_LED_I2C_ADDR1 0x2c // 44

//////////// APR STUFF

#include <stdio.h>
#include <assert.h>

#include <apr_general.h>
#include <apr_file_io.h>
#include <apr_strings.h>
#include <apr_network_io.h>

/* default connect hostname */
#define DEF_REMOTE_HOST		"localhost"

/* default connect port number */
#define DEF_REMOTE_PORT		8081

/* default socket timeout */
#define DEF_SOCK_TIMEOUT	(APR_USEC_PER_SEC * 30)

/* default buffer size */
#define BUFSIZE			4096

/* useful macro */
#define CRLF_STR		"\r\n"


#define APR_BLOCK_VALUE 0
#define APR_BLOCK_TIMEOUT -1

#if TCP_SYNCHRONIZE
// Simple class to facilitate sharing apr initialization and memory pool
class APRPool
{
public:

	APRPool()
	{
		apr_initialize();
		apr_pool_create(&mp, NULL);
	}
	~APRPool()
	{
		apr_pool_destroy(mp);
	    apr_terminate();
	}

	apr_pool_t * Get() { return mp; }

protected:

	apr_pool_t *mp;
};

class HokeySocketClient
{
public:

	apr_pool_t *m_mp, *m_cp;
    apr_socket_t *m_s;

	HokeySocketClient(APRPool *pool) : m_mp(pool->Get()), m_s(0)
    {
		apr_pool_create(&m_cp, m_mp);	// create our own child pool
    }

	~HokeySocketClient()
	{
		if(m_s)
		{
			apr_socket_close(m_s);
		}

		apr_pool_destroy(m_cp);
	}

	bool Connect(const char *addr, int port)
	{
		return (do_connect(&m_s, m_cp, addr, port) == APR_SUCCESS ? true : false);
	}

	int Wait()
	{
		apr_status_t rv;

		char buf[BUFSIZE];
		apr_size_t len = 1;
		rv = apr_socket_recv(m_s, buf, &len);
		//printf("recv returned %d w/ %d bytes\n", rv, len); fflush(stdout);

		return (rv == APR_SUCCESS);
	}

protected:

	/**
	 * Connect to the remote host
	 */
	apr_status_t do_connect(apr_socket_t **sock, apr_pool_t *mp, const char *addr, int port)
	{
	    apr_sockaddr_t *sa;
	    apr_socket_t *s;
	    apr_status_t rv;

	    rv = apr_sockaddr_info_get(&sa, addr, APR_INET, port, 0, mp);
	    if (rv != APR_SUCCESS) {
	    	EyelockLog(logger, WARN, "Couldn't get info (%d) ", rv); fflush(stdout);
	    	return rv;
	    }

	    rv = apr_socket_create(&s, sa->family, SOCK_STREAM, APR_PROTO_TCP, mp);
	    if (rv != APR_SUCCESS) {
	    	EyelockLog(logger, DEBUG, "Couldn't create socket (%d)", rv);
	    	return rv;
	    }

	    apr_socket_opt_set(s, APR_SO_NONBLOCK, APR_BLOCK_VALUE);
	    apr_socket_timeout_set(s, APR_BLOCK_TIMEOUT); // blocking forever
	    rv = apr_socket_connect(s, sa);
	    if (rv != APR_SUCCESS) {
	    	EyelockLog(logger, ERROR, ("Couldn't connect (%d) ", rv); fflush(stdout);
	    	return rv;
	    }

	    /* see the tutorial about the reason why we have to specify options again */
	    apr_socket_opt_set(s, APR_SO_NONBLOCK, APR_BLOCK_VALUE);
	    apr_socket_timeout_set(s, APR_BLOCK_TIMEOUT);

	    *sock = s;
	    return APR_SUCCESS;
	}

	/**
	 * Send a request as a simple HTTP request protocol.
	 * Write the received response to the standard output until the EOF.
	 */
	apr_status_t do_read_block(apr_socket_t *sock, apr_pool_t *mp, char *buffer, int *bytes)
	{
		apr_status_t rv;

		char buf[BUFSIZE];
		apr_size_t len;

		len = 1;
		rv = apr_socket_recv(sock, buf, &len);
		//printf("recv returned %d w/ %d bytes\n", rv, len); fflush(stdout);

		return rv;
	}
};

#define DEF_SOCKET_BACKLOG	SOMAXCONN

//////

/* synchronization server */
class SynchronizationServer : public HThread
{

	// Handler helper thread, in case we need it...
	class SynchronizationHandler : public HThread
	{
	public:
		SynchronizationHandler() {}
		virtual unsigned int MainLoop()
		{

		}
	};

public:

	// APR primitives
	apr_pool_t *m_mp,*m_cp;
	apr_socket_t *m_s;/* listening socket */

	std::string m_Address;
	int m_Port;

	std::map<std::string, apr_socket_t *> m_Sockets;

	SynchronizationServer(APRPool *pool, const char *addr, int port) : m_mp(pool->Get()), m_Port(port)
	{
		m_Address += addr;
		apr_pool_create(&m_cp, m_mp);	// create our own child pool
	}

	~SynchronizationServer()
	{
		apr_pool_destroy(m_cp);
	}

	virtual const char* getName() { return "SynchronizationServer"; }

	apr_status_t do_listen(apr_socket_t **sock, apr_pool_t *mp, const char *addr, int listenPort)
	{
	  apr_status_t rv;
	  apr_socket_t *s;
	  apr_sockaddr_t *sa;

	  rv = apr_sockaddr_info_get(&sa, NULL, APR_INET, listenPort, 0, mp);
	  if (rv != APR_SUCCESS) {
	    return rv;
	  }

	  rv = apr_socket_create(&s, sa->family, SOCK_STREAM, APR_PROTO_TCP, mp);
	  if (rv != APR_SUCCESS) {
	    return rv;
	  }

	  /* it is a good idea to specify socket options explicitly.
	   * in this case, we make a blocking socket as the listening socket */
	  apr_socket_opt_set(s, APR_SO_NONBLOCK, APR_BLOCK_VALUE);
	  apr_socket_timeout_set(s, APR_BLOCK_TIMEOUT);
	  apr_socket_opt_set(s, APR_SO_REUSEADDR, 1);/* this is useful for a server(socket listening) process */

	  rv = apr_socket_bind(s, sa);
	  if (rv != APR_SUCCESS) {
	    return rv;
	  }
	  rv = apr_socket_listen(s, DEF_SOCKET_BACKLOG);
	  if (rv != APR_SUCCESS) {
	    return rv;
	  }
	  char *ipAddress;
	  apr_sockaddr_ip_get(&ipAddress,sa);
	  PING
	  *sock = s;
	  return rv;
	}

	virtual unsigned int MainLoop()
	{
		apr_status_t rv;

		rv = do_listen(&m_s, m_cp, m_Address.c_str(), m_Port);
		if (rv != APR_SUCCESS) {
			return 0;
		}
		PING
		apr_pool_t *tmp;
		apr_pool_create(&tmp, m_cp);

		while(!ShouldIQuit())
		{
			//printf("Waiting for connection\n"); fflush(stdout);

			apr_socket_t *ns;/* accepted socket */

			rv = apr_socket_accept(&ns, m_s, tmp);
			if (rv != APR_SUCCESS) {
				EyelockLog(logger, ERROR, "apr_socket_accept: failed"); fflush(stdout);
				usleep(1000);
				continue;
			}

			//printf("connection accepted\n"); fflush(stdout);

			/* it is a good idea to specify socket options for the newly accepted socket explicitly */
			apr_socket_opt_set(ns, APR_SO_NONBLOCK, APR_BLOCK_VALUE);
			apr_socket_timeout_set(ns, APR_BLOCK_TIMEOUT);

			if (DoTask(ns))
			{
				apr_pool_clear(tmp);
			}
		}

		apr_pool_destroy(tmp);

		return 0;
	}

	bool DoTask(apr_socket_t *ns)
	{
		apr_status_t rv;

		bool trigger = false;

		apr_sockaddr_t *addr_remote;
		char *ip_ptr=NULL;

		rv = apr_socket_addr_get ( &addr_remote, APR_REMOTE, ns );
		rv = apr_sockaddr_ip_get(&ip_ptr, addr_remote);
		//printf("Connection from: %s\n", ip_ptr);

		std::string address(ip_ptr);

		// place this socket in a queue sorted by ip address
		m_Sockets[address] = ns;

		if(m_Sockets.size() == 2) /* TODO: add config file # of slaves */
		{
			for(std::map<std::string, apr_socket_t *>::reverse_iterator iter = m_Sockets.rbegin(); iter != m_Sockets.rend(); iter++)
			{
				apr_size_t len = 1;
				EyelockLog(logger, DEBUG, "trigger: %s ", (*iter).first.c_str());
				rv = apr_socket_send((*iter).second, "X", &len); // always trigger the slave first
				apr_socket_close((*iter).second);

				usleep(0); /* should be more like 2000, just debugging now */
			}
			m_Sockets.clear(); // destroy all open sockets
			trigger = true;
		}

		return trigger;
	}
};
#endif

#define EXPOSURE_TIME 2000

/////////////////////

void read_img_vals(int fd)
{
	int val;

	if (ioctl(fd, MT9P001_IOC_RD_IMG_WIDTH, &val) < 0) {
		EyelockLog(logger, ERROR, "ioctl(IMG_WIDTH) - %s", strerror(errno));
		perror("ioctl(IMG_WIDTH)");
	}
	else
		EyelockLog(logger, DEBUG, "image width %d", val);


	if (ioctl(fd, MT9P001_IOC_RD_IMG_HEIGHT, &val) < 0) {
		EyelockLog(logger, ERROR, "ioctl(IMG_HEIGHT) - %s", strerror(errno));
		perror("ioctl(IMG_HEIGHT)");
	}
	else
		EyelockLog(logger, DEBUG, "image height %d", val);


	if (ioctl(fd, MT9P001_IOC_RD_IMG_SIZE, &val) < 0) {
		EyelockLog(logger, ERROR, "ioctl(IMG_SIZE) - %s", strerror(errno));
		perror("ioctl(IMG_SIZE)");
	}
	else
		EyelockLog(logger, DEBUG, "image size %d", val);
}

unsigned short read_reg(int fd, int reg)
{
	struct mt9p001_i2c_ioc ioc;

	ioc.reg = 0xff & reg;
	ioc.value = 0;

	EyelockLog(logger, DEBUG, "Read register 0x%02X ", ioc.reg);

	if (ioctl(fd, MT9P001_IOC_RD_I2C, &ioc) < 0) {
		EyelockLog(logger, ERROR, "ioctl(MT9P001_IOC_RD_I2C) - %s", strerror(errno));
		perror("ioctl(MT9P001_IOC_RD_I2C)");
	}
	else
		EyelockLog(logger, INFO, "register 0x%02X = 0x%04X", ioc.reg, ioc.value);

	return ioc.value;
}

void write_reg(int fd, int reg, int val)
{
	struct mt9p001_i2c_ioc ioc;

	ioc.reg = 0xff & reg;
	ioc.value = 0xffff & val;

	EyelockLog(logger, DEBUG, "Write 0x%04X to register 0x%02X", ioc.value, ioc.reg);

	if (ioctl(fd, MT9P001_IOC_WR_I2C, &ioc) < 0) {
		EyelockLog(logger, ERROR, "ioctl(MT9P001_IOC_WR_I2C)- %s", strerror(errno));
		perror("ioctl(MT9P001_IOC_WR_I2C)");
	}
	else
		EyelockLog(logger, INFO, "wrote 0x%04X to register 0x%02X ", ioc.value, ioc.reg);
}



void MT9P001FrameGrabber::SetDoAlternating(bool mode)
{
	SetSafeValue(m_DoAlternating, mode);
}

bool MT9P001FrameGrabber::GetDoAlternating()
{
	return GetSafeValue(m_DoAlternating);
}

void MT9P001FrameGrabber::SetRecipeIndex(int index)
{
	if(index < m_Recipes.size()) // m_Recipes not adjustable dynamically
	{
		SetSafeValue(m_RecipeIndex, index);
	}
}

int MT9P001FrameGrabber::update(unsigned char regis , unsigned short value){
	ScopeLock lock(I2CBusNanoAPI::instance().GetLock());
	write_reg(fd,regis,value);
	return 0;
}

void MT9P001FrameGrabber::update_all(camconfig* newconfig)
{
	//Check if everything validates...Else fail

	if(newconfig->row_start_val > 2004){EyelockLog(logger,  WARN, "Wrong row start value"); exit(1);}
	if(newconfig->column_start_val > 2750){EyelockLog(logger,  WARN, "Wrong column start value"); exit(1);}

	if(newconfig->row_size_val < 1){EyelockLog(logger,  WARN, "Wrong row size value (too low)"); exit(1);}
	if(newconfig->row_size_val > 2005){EyelockLog(logger,  WARN, "Wrong row size value (too high)"); exit(1);}

	if(newconfig->column_size_val < 1){EyelockLog(logger,  WARN, "Wrong column size value (too low)"); exit(1);}
	if(newconfig->column_size_val > 2751){EyelockLog(logger,  WARN, "Wrong column size value(too high)"); exit(1);}

	if(newconfig->horizontal_blank_val > 4095){EyelockLog(logger,  WARN, "Wrong horizontal blank value (too high)"); exit(1);}

	if(newconfig->vertical_blank_val < 8){EyelockLog(logger,  WARN, "Wrong vertical blank value (too low)"); exit(1);}
	if(newconfig->vertical_blank_val > 2047){EyelockLog(logger,  WARN, "Wrong vertical blank value (too high)"); exit(1);}


	if(newconfig->mirror_row > 1){EyelockLog(logger,  WARN, "Value of mirror_row should be either 0 or 1"); exit(1);}
	if(newconfig->mirror_column > 1){EyelockLog(logger,  WARN, "Value mirror_column should be either 0 or 1"); exit(1);}


	if((newconfig->row_bin > 3) | (newconfig->row_bin==2)){EyelockLog(logger,  WARN, "Value row_bin should be {0,1,3} "); exit(1);}
	if(newconfig->row_skip > 7){EyelockLog(logger,  WARN, "Value row_skip should be less than 8"); exit(1);}


	if((newconfig->column_bin > 3) | (newconfig->column_bin==2)){EyelockLog(logger,  WARN, "Value column_bin should be {0,1,3} "); exit(1);}
	if(newconfig->column_skip > 6){EyelockLog(logger,  WARN, "Value column_skip should be less than 7"); exit(1);}


	if(newconfig->green_digital_gain > 120){EyelockLog(logger,  WARN, "Value green_digital_gain should be less than 121"); exit(1);}
	if(newconfig->green_analog_multiplier >2){EyelockLog(logger,  WARN, "Value green_analog_multiplier should be either 0 or 1"); exit(1);}
	if(((unsigned short)newconfig->green_analog_gain < 8) | ((unsigned short )newconfig->green_analog_gain >63)){EyelockLog(logger,  WARN, "Value green_analog_gain should be in the range [8,63] "); EyelockLog(logger,  WARN, "green analog gain : %i ",newconfig->green_analog_gain); exit(1);  }


	if(newconfig->blue_digital_gain > 120){EyelockLog(logger,  WARN, "Value blue_digital_gain should be less than 121"); exit(1);}
	if(newconfig->blue_analog_multiplier >2){EyelockLog(logger,  WARN, "Value blue_analog_multiplier should be either 0 or 1"); exit(1);}
	if((newconfig->blue_analog_gain < 8) | (newconfig->blue_analog_gain >63)){EyelockLog(logger,  WARN, "Value blue_analog_gain should be in the range [8,63] "); exit(1);}


	if(newconfig->red_digital_gain > 120){EyelockLog(logger,  WARN, "Value red_digital_gain should be less than 121"); exit(1);}
	if(newconfig->red_analog_multiplier >2){EyelockLog(logger,  WARN, "Value green_analog_multiplier should be either 0 or 1"); exit(1);}
	if((newconfig->red_analog_gain < 8) | (newconfig->red_analog_gain >63)){EyelockLog(logger,  WARN, "Value red_analog_gain should be in the range [8,63] "); exit(1);}

	// ROW_START
	update(ROW_START,newconfig->row_start_val);



	// COLUMN_START

	if (newconfig->mirror_column){
		if(newconfig->column_bin==0) update(COLUMN_START,4*((newconfig->column_start_val)/4)+2);
		if(newconfig->column_bin==1) update(COLUMN_START,8*((newconfig->column_start_val)/8)+4);
		if(newconfig->column_bin==2) update(COLUMN_START,16*((newconfig->column_start_val)/16)+8);
	}
	else {
		if(newconfig->column_bin==0) update(COLUMN_START,4*((newconfig->column_start_val)/4));
		if(newconfig->column_bin==1) update(COLUMN_START,8*((newconfig->column_start_val)/8));
		if(newconfig->column_bin==2) update(COLUMN_START,16*((newconfig->column_start_val)/16));
	}

	//ROW_SIZE
	update(ROW_SIZE,newconfig->row_size_val);


	//COLUMN_SIZE

	update(COLUMN_SIZE,newconfig->column_size_val);

	//HORIZONTAL_BLANK

	update(HORIZONTAL_BLANK,newconfig->horizontal_blank_val);

	//VERTICAL_BLANK

	update(VERTICAL_BLANK,newconfig->vertical_blank_val);

	//PIX_CLK_CTRL
	update(PIX_CLK_CTRL,newconfig->pixel_clock_control);

	//SHUTTER_WIDTH_UPPER

	update(SHUTTER_WIDTH_UPPER,newconfig->shutter_width_upper_val);

	//SHUTTER_WIDTH_LOWER

	update(SHUTTER_WIDTH_LOWER,newconfig->shutter_width_lower_val);

	//SHUTTER_DELAY
	update(SHUTTER_DELAY,newconfig->shutter_delay);

	int blc=0;
	if(newconfig->enable_blc)
		blc=1;

	//READ_MODE2
	update(READ_MODE2,(newconfig->mirror_row<<15)+(newconfig->mirror_column<<14)+(blc<<6));

	//ROW_ADDRESS_MODE

	update(ROW_ADDRESS_MODE,(newconfig->row_bin<<4)+(newconfig->row_skip));

	//COLUMN_ADDRESS_MODE

	update(COLUMN_ADDRESS_MODE,(newconfig->column_bin<<4)+(newconfig->column_skip));


	// GREEN GAIN
	update(GREEN1_GAIN,(newconfig->green_digital_gain<<8)+(newconfig->green_analog_multiplier << 6) + (newconfig->green_analog_gain));

	update(GREEN2_GAIN,(newconfig->green_digital_gain<<8)+(newconfig->green_analog_multiplier << 6) + (newconfig->green_analog_gain));

	//BLUE GAIN
	update(BLUE_GAIN,(newconfig->blue_digital_gain<<8)+(newconfig->blue_analog_multiplier << 6) + (newconfig->blue_analog_gain));

	//RED GAIN
	update(RED_GAIN,(newconfig->red_digital_gain<<8)+(newconfig->red_analog_multiplier << 6) + (newconfig->red_analog_gain));


	//GLOBAL GAIN
	update(GLOBAL_GAIN,newconfig->global_gain_val);

	update(ROW_BLACK_DEF_OFFSET,newconfig->row_black_default_offset);//Row black default offset should always be zero
	update(ROW_BLACK_TARGET,newconfig->black_level_target);//Row black target
	update(BLC_CALIBRATION,blc);//blc

	// test Pattern related values
	update(TEST_PATTERN_TYPE,newconfig->test_pattern_type);

	if(newconfig->test_pattern_type){
		update(TEST_PATTERN_GREEN,newconfig->test_pattern_green);
		update(TEST_PATTERN_RED,newconfig->test_pattern_red);
		update(TEST_PATTERN_BLUE,newconfig->test_pattern_blue);
		update(TEST_PATTERN_BAR_WIDTH,newconfig->test_pattern_bar_width);
	}

	if(newconfig->pll_enable)
	{
		update(OUTPUT_CONTROL,newconfig->output_control&0xFFFB);
		update(PLL_CONTROL,newconfig->pll_control&0xFFFD);
		update(PLL_CONFIG_1,newconfig->pll_config1);
		update(PLL_CONFIG_2,newconfig->pll_config2);
		EyelockLog(logger, DEBUG, "Sleep %d usec ",newconfig->pll_delay);
		usleep(newconfig->pll_delay);
		update(PLL_CONTROL,(newconfig->pll_control&0xFFFD)|2);
		update(OUTPUT_CONTROL,(newconfig->output_control&0xFFFB)); // |0x4);
	}
}

void MT9P001FrameGrabber::map_uservals_to_registers(Configuration *pCfg,camconfig* newconfig){

	newconfig->read_mode1_val = 0x8000;
	newconfig->global_gain_val = 0x8;

	// Grab dc offset
	int ivheight,ivwidth;
	int userheight=pCfg->getValue("MT9P001.row_size_val",1943);
	int userwidth=pCfg->getValue("MT9P001.column_size_val",2591);
	newconfig->column_start_val=pCfg->getValue("MT9P001.column_start_val",newconfig->column_start_val);
	newconfig->row_start_val=pCfg->getValue("MT9P001.row_start_val",newconfig->row_start_val);

	userheight++;
	userwidth++;

	int binning = pCfg->getValue("MT9P001.binning",0);

	switch (binning) {
	case 0:
		newconfig->row_bin = 0;
		newconfig->column_bin = 0;
		newconfig->row_skip = 0;
		newconfig->column_skip = 0;
		break;
	case 1:
		newconfig->row_bin = 1;
		newconfig->column_bin = 1;
		newconfig->row_skip = 1;
		newconfig->column_skip = 1;
		ivheight =userheight / 2;
		ivwidth = userwidth / 2;
		break;
	case 2:
		newconfig->row_bin = 3;
		newconfig->column_bin = 3;
		newconfig->row_skip = 3;
		newconfig->column_skip = 3;
		ivheight = userheight / 4;
		ivwidth = userwidth / 4;
		break;
	default:
		newconfig->row_bin = 0;
		newconfig->column_bin = 0;
		newconfig->row_skip = 0;
		newconfig->column_skip = 0;
		break;
	}

	newconfig->row_bin = pCfg->getValue( "MT9P001.row_bin" , newconfig->row_bin); //Legal values 0 or 3
	newconfig->row_skip = pCfg->getValue( "MT9P001.row_bin" , newconfig->row_skip);//Legal values [0,7]
	newconfig->column_bin = pCfg->getValue( "MT9P001.column_bin" , newconfig->column_bin);//Legal values {0,1,3}
	newconfig->column_skip = pCfg->getValue( "MT9P001.column_skip" , newconfig->column_skip);// Legal values [0,6]

	if(newconfig->row_bin){
		if(newconfig->row_skip < newconfig->row_bin){
			newconfig->row_skip = newconfig->row_bin;
			}
		}

	if(newconfig->column_bin){
		if(newconfig->column_skip < newconfig->column_bin){
			newconfig->column_skip = newconfig->column_bin;
		}
	}

	int divisorh = 1;
	switch (newconfig->row_skip) {
	case 1:
		divisorh = 2;
		break;
	case 3:
		divisorh = 4;
		break;
	case 2:
	case 4:
	case 5:
	case 6:
	case 7:
		divisorh = 1<< newconfig->row_skip;
		break;
	default:
		newconfig->row_bin = 0;
		newconfig->row_skip = 0;
		break;
	}

	ivheight = userheight / divisorh;

	int divisorw = 1;
	switch (newconfig->column_skip) {
	case 1:
		divisorw = 2;
		break;
	case 3:
		divisorw = 4;
		break;
	case 2:
	case 4:
	case 5:
	case 6:
		divisorw = 1<< newconfig->column_skip;
		break;
	default:
		newconfig->column_bin = 0;
		newconfig->column_skip = 0;
		break;
	}

	ivwidth = userwidth / divisorw;

	//Lets Update the Height and Width..

	ivwidth = (ivwidth>>4)<<4;//make multiple of 16
	ivheight = (ivheight>>4)<<4;//make multiple of 16

	int roiWidth = ivwidth*divisorw;
	int roiHeight = ivheight*divisorh;

	userwidth = ivwidth;
	userheight = ivheight;

	m_Width = userwidth;
	m_Height = userheight;

	EyelockLog(logger, DEBUG, "W = %d H = %d  ",m_Width,m_Height);

	newconfig->row_size_val = roiHeight -1;
	newconfig->column_size_val = roiWidth -1;

	EyelockLog(logger, DEBUG, "(X,Y,W,H)::(%d,%d,%d,%d) ",newconfig->column_start_val,newconfig->row_start_val,newconfig->column_size_val,newconfig->row_size_val);

	EyelockLog(logger, DEBUG, "Row Bin %#x Skip %#x ",newconfig->row_bin,newconfig->row_skip);
	EyelockLog(logger, DEBUG, "Col Bin %#x Skip %#x ",newconfig->column_bin,newconfig->column_skip);
	EyelockLog(logger, DEBUG, "Width::User %d IntVec %d ",userwidth,ivwidth);
	EyelockLog(logger, DEBUG, "Height::User %d IntVec %d ",userheight,ivheight);

	if((newconfig->row_size_val+1+newconfig->row_start_val > (1944+54)) ||
    	(newconfig->column_size_val+1+newconfig->column_start_val > (2592+16))){
		EyelockLog(logger, DEBUG, "ROI in the Config is wrong... ");
	}

	m_DcOffset = (unsigned short)pCfg->getValue("GRI.dc", (int)0);
	m_ShiftRight = (unsigned short)pCfg->getValue("GRI.shiftRight", (int)0);
	m_FlashTime = (unsigned short)pCfg->getValue("GRI.flashTime", (int)EXPOSURE_TIME);
	m_FramePause = (int) pCfg->getValue("GRI.framePause", (int)0);
	m_MasterMode = (int) pCfg->getValue("GRI.masterMode", (int)0);
	SetDoAlternating((bool) pCfg->getValue("GRI.flashAlternate", (int)0)); // Use access method to set Safe<bool>
	m_RGBBrightness = (int) pCfg->getValue("GRI.LEDBrightness", 20); /* DJH NEW */
	m_TriggerTimeUs = (int) pCfg->getValue("GRI.TriggerTimeUs", 20); /* DJH NEW */
	m_Debug = (bool) pCfg->getValue("MT9P001.Debug", false); /* DJH NEW */
	m_DummySleep = pCfg->getValue("MT9P001.dummySleep", 100000); /* DJH NEW */

	m_shiftAndOffset.clear();
	int num = pCfg->getValue("MT9P001.shiftAndOffsetCount", 0); /* DJH NEW */
	m_NextShiftAndDCSec = pCfg->getValue("MT9P001.NextshiftAndOffsetSec", 0);
	char Buff[1024]={0};
	for(int j=0;j<num;j++){
		std::pair<unsigned short, unsigned short> val;
		sprintf(Buff,"MT9P001.%d.dc",j);
		val.second = (unsigned short)pCfg->getValue(Buff, (int)0);
		sprintf(Buff,"MT9P001.%d.shiftRight",j);
		val.first = (unsigned short)pCfg->getValue(Buff, (int)0);
		EyelockLog(logger, DEBUG, "Shift and DC for %d is %d %d ",j,val.first,val.second);
		m_shiftAndOffset.push_back(val);
	}

	if(num < 1){
		std::pair<unsigned short, unsigned short> val;
		val.first = m_ShiftRight;
		val.second = m_DcOffset;
		EyelockLog(logger, DEBUG, "Shift and DC for %d is %d %d ",0,val.first,val.second);
		m_shiftAndOffset.push_back(val);
	}

	if(m_Debug)
	{
		EyelockLog(logger, DEBUG, "DC = %d; shift = %d, flash = %d, alternate %d, pause = %d ", m_DcOffset, m_ShiftRight, m_FlashTime, GetDoAlternating(), m_FramePause);
	}

	char *data = (char *) pCfg->getValue("GRI.CameraRecipe", "");

	if(data != NULL)
	{
		std::string pipe = "|";
		std::string comma = ",";
		std::string colon = ":";
		std::string input(data);

		// GRI.CameraRecipe=4000,255|4000,127:1600,255|166,127
		std::vector<std::string> recipes = tokenize(input, colon);
		m_Recipes.resize(recipes.size());

		// Loop over recipes  m_Recipes[j][i]
		for(int j = 0; j < recipes.size(); j++)
		{
			std::vector<std::string> tokens = tokenize(recipes[j], pipe);
			for(int i = 0; i < tokens.size(); i++)
			{
				std::vector<std::string> split = tokenize(tokens[i], comma);
				int led = atoi(split[1].c_str());
				int exposure = atoi(split[0].c_str());
				m_Recipes[j].push_back( CameraRecipe(exposure, led) );
				if(GetDoAlternating())
				{
					m_Recipes[j].push_back( CameraRecipe(exposure, led) ); // if alternating duplicate each recipe
				}
			}
		}
	}

	if(m_Recipes.size() == 0)
	{
		m_Recipes.resize(1);
		m_Recipes[0].push_back( CameraRecipe(m_FlashTime, 255) );
	}

	m_diffIllumination = pCfg->getValue("Eyelock.EnableDiffIllumination",false);
	m_masterExposure = pCfg->getValue("Eyelock.LeftIlluminationValue",2500);
	m_slaveExposure = pCfg->getValue("Eyelock.RightIlluminationValue",2500);
	//Allow a user modification
	if(pCfg) m_camCfg->configure(pCfg, newconfig);

	// write this one as update_all does not write it. ppifcd_test.c is another user of the same function
	update(READ_MODE1, newconfig->read_mode1_val);

#if 0
	//GLOBAL GAIN
	update(GLOBAL_GAIN,newconfig->global_gain_val);

	//SHUTTER_WIDTH_UPPER
	update(SHUTTER_WIDTH_UPPER,newconfig->shutter_width_upper_val);

	//SHUTTER_WIDTH_LOWER
	update(SHUTTER_WIDTH_LOWER,newconfig->shutter_width_lower_val);

	int blc=0;
	if(newconfig->enable_blc)
		blc=1;

	//READ_MODE2
	update(READ_MODE2,(newconfig->mirror_row<<15)+(newconfig->mirror_column<<14)+(blc<<6));

	update(BLC_CALIBRATION,blc);//blc
	update(ROW_BLACK_DEF_OFFSET,newconfig->row_black_default_offset);//Row black default offset should always be zero
	update(ROW_BLACK_TARGET,newconfig->black_level_target);//Row black target
#else

	update_all(newconfig);

	update_extra_regs((char *)pCfg->getValue("MT9P001.extra_registers",""));
#endif

}

MT9P001FrameGrabber::MT9P001FrameGrabber()
{
	m_Offset =0;
	m_BufferSize =0;
	m_ImageSize=0;
	m_Map_base= 0;
	fd =0;
	m_camCfg = new CameraConfig;
	m_DcOffset = 0;
	m_ShiftRight = 0;
	m_FlashTime = 0;
	m_IrLedBankIndex = 0;
	m_FramePause = 0;
	m_DoAlternating = false;
	m_RGBReset = 0;
	m_pImageBuffer = 0;
	m_RecipeIndexIndex = 0;
	m_BufferIndex = 0;
	m_counter = 0;
	m_state = eREQUIREFRAME;
	m_SleepTime = 0;
#if TCP_SYNCHRONIZE
    m_pPool = new APRPool;
    m_pSynchServer = 0;
#endif

    SetSafeValue(m_RecipeIndex, 0);
    m_LedMask = 255;
    // m_RGB.set(RGBTriple(10, 10, 10));	// don't turn on LED until matcher ready

    m_sleeptimeInDummyRead  = 100000; //100 milliSec
	m_state= eDUMMYCAPTURE;
	m_dummyOffset = (2592*1944*2*3+4095 )&0xFFFFF000u;
	m_newDriverEnable = false;
	m_DummyOffsetFullyAllocated = false;
	m_dummyHeight = 10;
	m_dograb = true;
	m_diffIllumination = false;
	m_slaveExposure = 0;
	m_masterExposure = 0;
	m_resetusingpsoc = true;

	m_nwTriggerTime=10;
	m_nwFlashTime =2000;
	m_nwIlluminatorvalue=0;
	m_fromNetwork=false;


}

MT9P001FrameGrabber::~MT9P001FrameGrabber()
{

#if TCP_SYNCHRONIZE
	if(m_pSynchServer)
	{
		m_pSynchServer->End();
		delete m_pSynchServer; m_pSynchServer = 0;
	}

	if(m_pPool)
	{
		delete m_pPool;
		m_pPool = 0;
	}
#endif

}

void MT9P001FrameGrabber::init(Configuration *pCfg)
{
	m_pRingBuffer = new RingBufferImage(3); /* Allocate ring buffer to be frame buffer - 1 */
	m_RingBufferOffset = new RingBufferOffset(3);

	m_SleepTime = (int) pCfg->getValue("Eyelock.SleepMicronDriver",0);
	m_newDriverEnable = pCfg->getValue("Eyelock.NewCameraDriverEnable",true);

	m_resetusingpsoc = pCfg->getValue("Eyelock.ResetUsingPSOC",true);

	int numbits = pCfg->getValue("Eyelock.NumBits",8);
	m_numbits = numbits > 8?16:numbits;
	SetImageBits(m_numbits);

	m_camCfg->Init();
	camconfig *cam  = m_camCfg->Get_config();

	EyelockLog(logger, DEBUG, "OPEN DEVICE "); fflush(stdout);
	fd = open("/dev/mt9p001", O_RDWR);
	if (fd < 0) {
		EyelockLog(logger, ERROR, "Unable To EXE:: open /dev/mt9p001 - %s ", strerror(errno));
		perror("open(/dev/mt9p001)");
		exit(1);
	}

	m_BufferSize = ISP_MEM_SIZE;

	m_Map_base = (unsigned char *)mmap(0, m_BufferSize, PROT_READ, MAP_SHARED, fd, 0);

	if (m_Map_base == MAP_FAILED) {
		EyelockLog(logger, ERROR, "mmap error - %s", strerror(errno));
		perror("mmap");
		close(fd);
		exit(1);
	}

	map_uservals_to_registers(pCfg,cam);

	read_img_vals(fd);
#if 1
//	m_Width = cam->column_size_val+1;
//	m_Height = cam->row_size_val+1;
	m_currHeight = m_Height-1;

#else
	m_Height = (int) pCfg->getValue("FrameSize.height", (int)cam->row_size_val+1);
	m_Width = (int) pCfg->getValue("FrameSize.width", (int)cam->column_size_val+1);
#endif

	int offsetTable[4]={0};
	offsetTable[0] = 0;
	offsetTable[1] = (m_Width*m_Height*2+4095 )&0xFFFFF000u;
	offsetTable[2] = (m_Width*m_Height*2*2+4095 )&0xFFFFF000u;
	offsetTable[3] = (m_Width*m_Height*2*3+4095 )&0xFFFFF000u;
	m_dummyOffset = offsetTable[3];

	if(m_dummyOffset + offsetTable[1] < ISP_MEM_SIZE){
		m_DummyOffsetFullyAllocated = true;
	}

	for(int i=0;i<3;i++){
		EyelockLog(logger, DEBUG, "Offset Table %d -> %d  ",i,offsetTable[i]);
		m_RingBufferOffset->Push(offsetTable[i]);
	}

	m_ImageSize = 2 * m_Width * m_Height;

	if(m_Debug)
	{
		EyelockLog(logger, DEBUG, "W H S = %d %d %d ",m_Width,m_Height,m_ImageSize);
	}

	m_pImageBuffer = new unsigned char[m_ImageSize];

	if(m_MasterMode == 1)
	{
	//	FlashRGB( RGBTriple(m_RGBBrightness, m_RGBBrightness, m_RGBBrightness) );
	}

#if TCP_SYNCHRONIZE
	if(m_MasterMode == 1)
	{
		m_pSynchServer = new SynchronizationServer(m_pPool, "192.168.20.2", 8421);
	    m_pSynchServer->Begin();
	}
#endif

}
void MT9P001FrameGrabber::getPPIParams(int& pixels_per_line, int& lines_per_frame, int& ppiControl)const{
	pixels_per_line = m_Width;
	lines_per_frame = m_Height;
}

void MT9P001FrameGrabber::term()
{
	EyelockLog(logger, INFO, "MT9P001FrameGrabber::term "); fflush(stdout);

	if(m_RingBufferOffset){
		delete m_RingBufferOffset;
	}
	if(m_pRingBuffer)
		delete m_pRingBuffer;
	if(m_pImageBuffer)
		delete [] m_pImageBuffer;
	if(m_Map_base)
		munmap(m_Map_base, m_BufferSize);
	if(fd)
		close(fd);
}
bool MT9P001FrameGrabber::start(bool bStillFrames){
	EyelockLog(logger, DEBUG, "MT9P001FrameGrabber::start() "); fflush(stdout);
	Begin();
	return false;
}
bool MT9P001FrameGrabber::stop()
{
	EyelockLog(logger, INFO, "MT9P001FrameGrabber::stop() "); fflush(stdout);
	End();
	return false;
}
void MT9P001FrameGrabber::getDims(int& width, int& height)const
{
	width = m_Width;
	height = m_Height;
}

#include <ios>
#define TIME_ME(m, op) { \
		struct timeval currtime1, currtime2;uint64_t t1, t2;double elapsed; \
		gettimeofday(&currtime1, 0); \
		op; \
		gettimeofday(&currtime2, 0); \
		t1 = currtime1.tv_sec;\
		t1 = t1 * 1000000;\
		t1+= currtime1.tv_usec; \
		t2 = currtime2.tv_sec;\
		t2 = t2 * 1000000;\
		t2+= currtime2.tv_usec; \
		elapsed = (t2 - t1) / 1000000.0; \
		EyelockLog(logger, DEBUG, "%s: %lf ", m, elapsed); \
		}

static int ReallyWrite(int reg, unsigned int value)
{
	int status = 0;
	unsigned int check = 0;
	for(int i = 0; i < 4; i++)
	{
		status = I2CBusNanoAPI::instance().Write( reg, value );
		if(!status)
		{
			status = I2CBusNanoAPI::instance().Read( reg, &check );
			if(!status)
			{
				if(check == value)
					break;
			}
		}
	}
	return 0;
}
bool MT9P001FrameGrabber::SetPWM(unsigned char addr, unsigned char value){
	if(m_MasterMode == 1){
		ScopeLock lock(I2CBusNanoAPI::instance().GetLock());
		if(0 > I2CBusNanoAPI::instance().Assign(NANO_LED_I2C_ADDR0)){
			// EyelockLog(logger, ERROR, "Failed to assign LED address on I2C bus "); fflush(stdout);
			return false;
		}
		if(m_Debug)EyelockLog(logger, DEBUG, " Write to PWM Register Addr %02x -> %02x  ",addr,value);
		ReallyWrite(addr,value);
	}
	return true;
}

void MT9P001FrameGrabber::SetShiftAndOffset(unsigned short dc, int shift){
	ScopeLock lock(m_ShiftAndDCLock);
	m_ShiftRight = shift;
	m_DcOffset = dc;
	m_shiftAndOffset.clear();
	std::pair<unsigned short, unsigned short> val;
	val.first = m_ShiftRight;
	val.second = m_DcOffset;
	m_shiftAndOffset.push_back(val);

}

void MT9P001FrameGrabber::GetShiftAndOffset(unsigned short& dc, int& shift){
	static int shdccnt = 0;
	static long int prevts=0;
	ScopeLock lock(m_ShiftAndDCLock);
	int sz = m_shiftAndOffset.size();
	if(shdccnt >= sz) shdccnt = 0;
	std::pair<unsigned short,unsigned short> shftdc;
	shftdc = m_shiftAndOffset[shdccnt];
	shift = shftdc.first;
	dc =  shftdc.second;

	struct timeval m_timer;
	gettimeofday(&m_timer, 0);
	long int currts = m_timer.tv_sec;
	if((currts - prevts) > m_NextShiftAndDCSec){
		shdccnt++;
		prevts = currts;
	}
}


void MT9P001FrameGrabber::FlashRGBSetLEDMask() // internal API
{
	if(m_MasterMode == 1)
	{
//		EyelockLog(logger, DEBUG, "MT9P001FrameGrabber: set color %d %d %d", m_RGB.get().R(), m_RGB.get().G(), m_RGB.get().B()); fflush(stdout);
		ScopeLock lock(I2CBusNanoAPI::instance().GetLock());

		if(0 > I2CBusNanoAPI::instance().Assign(NANO_LED_I2C_ADDR0))
		{
			EyelockLog(logger, ERROR, "Failed to assign LED address on I2C bus "); fflush(stdout);
			return;
		}
#if 1
		for(int i = 0; i < 4; i++)
		{
			ReallyWrite( 0, m_RGB.get().R() );
			ReallyWrite( 1, m_RGB.get().G());
			ReallyWrite( 2, m_RGB.get().B());
			ReallyWrite( 4, 1);
			ReallyWrite( 3, m_LedMask);
			ReallyWrite( 4, 2);
		}
#else
		I2CBusNanoAPI::instance().Write( 0, m_RGB.get().R() );
		I2CBusNanoAPI::instance().Write( 1, m_RGB.get().G());
		I2CBusNanoAPI::instance().Write( 2, m_RGB.get().B());
		I2CBusNanoAPI::instance().Write( 4, 1);
		I2CBusNanoAPI::instance().Write( 3, m_LedMask);
		I2CBusNanoAPI::instance().Write( 4, 2);
#endif
	}
}

void MT9P001FrameGrabber::FlashRGB() // Internal API
{
	SafeLock<RGBTriple> lock(m_RGB);
	FlashRGBSetLEDMask();
}

void MT9P001FrameGrabber::FlashRGB(const RGBTriple &rgb) // public API
{
	SafeLock<RGBTriple> lock(m_RGB);
	m_RGB.set(rgb);
	FlashRGBSetLEDMask();
}

void MT9P001FrameGrabber::SetExposureandIlluminator(bool fromnw,int flashtime, int triggertime , int illval){
	ScopeLock lock(m_nwMsgLock);
	m_nwTriggerTime = triggertime>0?triggertime:10;
	m_nwFlashTime = flashtime>0?flashtime:2000;
	m_nwIlluminatorvalue = illval&0x1;
	m_fromNetwork = fromnw;
}
bool MT9P001FrameGrabber::getFromNetwork(){
	ScopeLock lock(m_nwMsgLock);
	return m_fromNetwork;
}

ImageProp MT9P001FrameGrabber::GrabFrame(int flashUS, int triggerUS, int offset,int Height, int led)
{
	mt9p001_snap_ioc snap_ioc;
	snap_ioc.img_height = m_Height;
	snap_ioc.img_width = m_Width;
	snap_ioc.irled_en[0] = 0;
	snap_ioc.irled_en[1] = 0;
	snap_ioc.trigger_time_us = triggerUS;
	snap_ioc.flash_time_ms = flashUS;
	snap_ioc.start_offset = offset;

	m_IrLedBankIndex = (1 - m_IrLedBankIndex); // Do alternating illumination

	if(getFromNetwork()){
		snap_ioc.irled_en[0] = 1-m_nwIlluminatorvalue;
		snap_ioc.irled_en[1] = m_nwIlluminatorvalue;
		snap_ioc.trigger_time_us = m_nwTriggerTime;
		snap_ioc.flash_time_ms = m_nwFlashTime;
		printf("Snap IR [%d:%d] FlashTime %d TriggerTime %d \n",snap_ioc.irled_en[0],snap_ioc.irled_en[1],snap_ioc.flash_time_ms,snap_ioc.trigger_time_us);

	}else if(flashUS > 0){
		if(GetDoAlternating())
		{	// turn on current bank
			snap_ioc.irled_en[m_IrLedBankIndex] = 1;
			if((m_diffIllumination)&&(m_MasterMode == 1)){
				if(m_IrLedBankIndex == 0){
					snap_ioc.flash_time_ms = m_masterExposure;
				}else{
					snap_ioc.flash_time_ms = m_slaveExposure;
				}
			}
			snap_ioc.trigger_time_us = 10 + (triggerUS * snap_ioc.irled_en[0]); // Pulse length encodes LED bank

			if(m_Debug && m_MasterMode == 1)
			{
				EyelockLog(logger, DEBUG, "[ %d ] Exposure %d trigger = %d ",m_IrLedBankIndex,snap_ioc.flash_time_ms, snap_ioc.trigger_time_us); fflush(stdout);
			}
		}
		else
		{   // Turn on both banks
			snap_ioc.irled_en[0] = 1;
			snap_ioc.irled_en[1] = 1;
		}
	}

	// DJH: Uncomment this line to turn off the second illuminator.
	// This makes debugging the slave illuminator state via pulse width code easier!!!!
	//snap_ioc.irled_en[1] = 0;

#if TCP_SYNCHRONIZE
	// Both slave and client mode connect to synch server
	if(m_MasterMode > 0)
	{
		bool flag = true;

		//printf("Attempt to connect to synchronization server\n"); fflush(stdout);
		HokeySocketClient client(m_pPool);

	EyelockLog(logger, DEBUG, "DJH: connect to server for synch "); fflush(stdout);
		while(!client.Connect("192.168.20.2", 8421))
		{
			EyelockLog(logger, ERROR, "Client failed to connect "); fflush(stdout);
			usleep(10000);
		}
		struct timeval m_timer;
		gettimeofday(&m_timer, 0);
		TV_AS_USEC(m_timer,starttimestamp);
		// Only the master camera has to wait for the signal to take a picture
		// Alll of the slaves can block on the IRQ
		if(m_MasterMode == 1)
		{
			while(!client.Wait())
			{
				EyelockLog(logger, ERROR, "Master failed to get green flag "); fflush(stdout);
			}
		}

	EyelockLog(logger, DEBUG, "DJH: got synch "); fflush(stdout);

	}
#endif

	struct timeval m_timer;
	gettimeofday(&m_timer, 0);
	TV_AS_USEC(m_timer,starttimestamp);
	ImageProp val ;
//	EyelockLog(logger, DEBUG, "MT9P001FrameGrabber:: Writing on to offset %#08x for %d  ", offset,m_counter+1);
	int status;
//	if(m_dummyOffset == offset){
//		if(m_Debug)
//			printf("Dummy Capture for %d\n ",m_counter+1);
//		XTIME_OP("IOCTLDUMMYSNAP",
//		status = ioctl(fd, MT9P001_IOC_WR_SLAVE_TRIGGER, &snap_ioc)
//		);
//	}else{
		XTIME_OP("IOCTLSNAP",
		status = ioctl(fd, MT9P001_IOC_WR_SNAP, &snap_ioc)
		);
//	}
	m_counter++;

//	printf("MT9P001FrameGrabber:: Done Writing on to offset %#08x %d\n",offset,m_counter);

	if (status < 0)
	{
		EyelockLog(logger, ERROR, "Unable To EXE::ioctl(MT9P001_IOC_WR_SNAP) - %s, status %d ", strerror(errno), status);
		perror("ioctl(MT9P001_IOC_WR_SNAP)");
		gettimeofday(&m_timer, 0);
		TV_AS_USEC(m_timer,endtimestamp);

		// DJH: What was this added for?
		//while(!ShouldIQuit() && !m_pRingBuffer->TryPush(val))
		//	usleep(250);

		val.m_ptr = NULL;
		val.m_ill0 = snap_ioc.irled_en[0];
		val.m_frameIndex = m_counter;
		val.m_startTime = starttimestamp;
		val.m_endTime = endtimestamp;
		return val;
	}

	gettimeofday(&m_timer, 0);
	TV_AS_USEC(m_timer,endtimestamp);

#if 1
	/*
	 *  The master sets LED illuminator state and can retrieve this from snap_ioc.
	 *  The slave measures the first illuminator panel state in the driver via the
	 *  input/slave pulse width, and reports it as a return code to this class.
	 */
//	if(m_Debug)
//	{
//		printf("FIRST LED: %d\n", (m_MasterMode==1) ? snap_ioc.irled_en[0] : status); fflush(stdout);
//	}
#endif

	if(led != m_LedMask)
	{
		m_LedMask = led;
		FlashRGB();
	}

	char* ret = (char*)(m_Map_base + offset);

	if(m_FramePause > 0)
	{
		usleep(m_FramePause);
	}

	// Fill in ImageProp with illuminator state for the master camera driver
	val.m_ptr = m_Map_base + offset;
	val.m_ill0 = snap_ioc.irled_en[0];
	val.m_frameIndex = m_counter;
	val.m_startTime = starttimestamp;
	val.m_endTime = endtimestamp;

	/*
	 *  For the slave camera we fill in the illuminator state based on the ioctl return code.
	 *  This is only needed for cases where GetDoalternating()==true.  The user must take care
	 *  to ensure that the corresponding config file entry (GRI.flashAlternate) is consistent
	 *  for both master and slave processes.
	 */
	if((m_MasterMode == 2) && GetDoAlternating())
	{
		val.m_ill0 = ((status&0xFFFFu) > 0) ? 1 : 0;
		//val.m_ill1 = 1 - val.m_ill0;
	}
//	printf("GF Frame %d ill %d %#08x \n",val.m_frameIndex,val.m_ill0,status);
	CURR_TV_AS_USEC(k)
	if(m_Debug)	EyelockLog(logger, DEBUG, "MT9P001FrameGrabber::%llu::GF %d Getting offset %d %d %d ",k,val.m_frameIndex,m_RingBufferOffset->Size(),m_pRingBuffer->Size(),m_state);

	return val;
}

static int gIndex = 0;

#define DO_THREAD 1

void MT9P001FrameGrabber::FlushAllInput(){
	ImageProp val;
	int cnt =0;
	for(int i=0;i<3;i++){
		bool status=m_pRingBuffer->TryPop(val);
		if(status){
			if(val.m_ptr >= m_Map_base){
				int val1 = val.m_ptr-m_Map_base;
				m_RingBufferOffset->Push(val1);
				cnt++;
			}
		}
	}
	if(m_Debug)EyelockLog(logger, DEBUG, "Flushed %d in input buffer  ",cnt);
}


char *MT9P001FrameGrabber::getLatestFrame_raw()
{
	int length = m_Width * m_Height ;
	unsigned char *ptr, *pOut = m_pImageBuffer ;

	SetState(eREQUIREFRAME);
	ImageProp val;
	int counter=0;
	int ctr1 = 0;
	while(!ShouldIQuit()&&!m_pRingBuffer->TryPop(val))
	{
		usleep(1000);
		counter++;
		if(counter > 3000){
			EyelockLog(logger, ERROR, "Waiting For Try pop in MT9P001FrameGrabber::getLatestFrame_raw  %d %d %d ",m_RingBufferOffset->Size(),m_pRingBuffer->Size(),m_state);
			counter = 0;
			ctr1++;
		}
#ifdef __ARM__
		if(1)
		{
			if((ctr1>2)&&(m_resetusingpsoc) && !isRunSystemCmd())
			{
				EyelockLog(logger, ERROR, "Unable to grab frames for 9 sec,will reset the mt9 driver and restart Eyelock app"); fflush(stdout);
				system("/home/root/recover_MT9_Driver.sh &");
				EyelockLog(logger, ERROR, "Recover driver script comepleted"); fflush(stdout);
				ctr1 = 0; //reset it does'nt call the recover script over and over
				return 0;
			}
		}
#endif
		if(ShouldIQuit())
			return 0;
	}

	ptr = val.m_ptr;
	m_ill0 = val.m_ill0;
	m_frameIndex = val.m_frameIndex;
	m_ts = val.m_endTime;

	int shift = 0;
	unsigned short dcoffset = 0;
	GetShiftAndOffset(dcoffset,shift);
	if(m_Debug)EyelockLog(logger, DEBUG, "Shift %d Offset %d  ",shift,dcoffset);
	if (ptr != NULL){
		if(m_numbits !=8){
			memcpy(pOut,ptr,length*2);
		}else{
			XTIME_OP("convert_real", ConvertU16ToU8((unsigned short *)ptr, pOut, length, (const int) shift, (const unsigned short) dcoffset) );
		}
	}else{
		EyelockLog(logger, DEBUG, "setting zero image  ");
		memset(pOut,0,length);
	}
	if(ptr >= m_Map_base){
		int val1 = ptr-m_Map_base;
		if(m_Debug)EyelockLog(logger, DEBUG, "IP %d Getting offset %d %d %d ",val.m_frameIndex,m_RingBufferOffset->Size(),m_pRingBuffer->Size(),m_state);
		m_RingBufferOffset->Push(val1);
	}
	return (char *)pOut;
}

void MT9P001FrameGrabber::PrintState(char *str){
	ScopeLock lock(m_stateLock);
	EyelockLog(logger, DEBUG, "%s Getting offset %d %d %d ",str,m_RingBufferOffset->Size(),m_pRingBuffer->Size(),m_state);
}


#define PAGE_SIZE 1024

/*
 * This frame grabber class relies on the thread safe ring buffer (size 2) for synchronization with
 * the processing thread.  We want to be doing the frame read + transfer for the next frame while we
 * are processing the last frame.  The logic below flip-flops between two memory locations and will
 * only work with a bounded buffer of size 2.  There isn't really any need to generalize this at this point.
 */

#define ROTATE_LED_BANK 0

void MT9P001FrameGrabber::SetState(FGSTATE state){
	ScopeLock lock(m_stateLock);
	m_state = state;
}

FGSTATE MT9P001FrameGrabber::GetState(){
	ScopeLock lock(m_stateLock);
	return m_state;
}

void  MT9P001FrameGrabber::Epilog(){
	camconfig *cam  = m_camCfg->Get_config();
	int led_mask = 255, exposure = m_FlashTime;
	EyelockLog(logger, DEBUG, "Dummy Grab Getting offset %d %d  ",m_RingBufferOffset->Size(),m_pRingBuffer->Size());
	GrabFrame(1, 20, 0,m_Height,led_mask); // run first exposure @ 1 microsecond per Ilya for possible camera driver bug fix
	EyelockLog(logger, DEBUG, "Dummy Grab Getting offset %d %d  ",m_RingBufferOffset->Size(),m_pRingBuffer->Size());
}

unsigned int MT9P001FrameGrabber::process(int& index, int& i,int& recipeIndex ){
	int led_mask = 255, exposure = m_FlashTime;
	recipeIndex = GetSafeValue(m_RecipeIndex);

	if(i >= m_Recipes[recipeIndex].size()){
		i = 0;
	}
	if(m_Recipes[recipeIndex].size()){
		exposure = m_Recipes[recipeIndex][i].GetExposure();
		led_mask = m_Recipes[recipeIndex][i].GetLEDMask();
	}
	int offset1;
	FGSTATE state = GetState();

	if((m_RingBufferOffset->Empty()||(state ==  eDUMMYCAPTURE))&&((m_MasterMode != 2)&&m_newDriverEnable && m_DummyOffsetFullyAllocated)){
		SetState(eDUMMYCAPTURE);
		state = eDUMMYCAPTURE;
		offset1 = m_dummyOffset;
	}else{
		int counter = 0;
		while(!ShouldIQuit() && !m_RingBufferOffset->TryPop(offset1)){
			usleep(1000);
			counter++;
			if(counter > 3000){
				EyelockLog(logger, DEBUG, " Waiting For Try pop in MT9P001FrameGrabber::MainLoop %d %d %d ",m_RingBufferOffset->Size(),m_pRingBuffer->Size(),m_state);
				counter=0;
			}
			if(ShouldIQuit())
				return 0;
		}
	}

	ImageProp val;
	XTIME_OP("GrabFrame()", val = GrabFrame(exposure, m_TriggerTimeUs, offset1,m_currHeight+1,led_mask));
	if(offset1 != m_dummyOffset){
		m_sleeptimeInDummyRead = val.m_endTime - val.m_startTime;
	}
	if(val.m_ptr == NULL){
		EyelockLog(logger, ERROR, "MT9P001FrameGrabber::MainLoop() missed a frame "); fflush(stdout);
		if(offset1 != m_dummyOffset)m_RingBufferOffset->Push(offset1);
		return 1;
	}
	if(offset1 == m_dummyOffset){
		int sleeptime = MIN(MAX(m_sleeptimeInDummyRead,m_DummySleep),m_DummySleep);
		return 1;
	}
	m_pRingBuffer->Push(val);
	if(index > 2) index = 0;
	/* Increment the recipe counter modulo the # of recipes (see top of loop) */
	i++;
	return 1;
}


bool MT9P001FrameGrabber::shouldWait(){
	ScopeLock lock(m_grabLock);
//	EyelockLog(logger, DEBUG, "MT9P001FrameGrabber::shouldWait %s  ",!m_dograb? "TRUE":"FALSE");
	return !m_dograb;
}

void MT9P001FrameGrabber::setShouldGrab(bool val){
	ScopeLock lock(m_grabLock);
//	EyelockLog(logger, DEBUG, "MT9P001FrameGrabber::setShouldGrab %s  ",val? "TRUE":"FALSE");
	if(!m_dograb)dataAvailable();
	m_dograb = val;
}


unsigned int MT9P001FrameGrabber::MainLoop() {
	std::string name = "MT9P001FrameGrabber::";
	try {
		Epilog();
		int index = 0, i = 0, recipeIndex = 0; /* i is index for recipe */
		while(!ShouldIQuit()) {
			waitForData();
			if(!process(index,i,recipeIndex))
				return 0;
			if(m_SleepTime)
				usleep(m_SleepTime);
			Frequency();
		}
	} catch (std::exception& ex) {
		cout << name << ex.what() << endl;
		cout << name << "exiting thread" << endl;
	} catch (Exception& ex1) {
		ex1.PrintException();
		cout << name << "exiting thread" << endl;
	} catch (const char *msg) {
		cout << name << msg << endl;
		cout << name << "exiting thread" << endl;
	} catch (...) {
		cout << name << "Unknown exception! exiting thread" << endl;
	}

	return 0;
}

size_t MT9P001FrameGrabber::readFrame(void *buf, size_t count){

	return read(fd,buf, count);
}


