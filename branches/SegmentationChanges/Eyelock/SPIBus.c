/*
 * SPIBus.cpp
 *
 *  Created on: Jan 3, 2012
 *      Author: dhirvonen
 */

#include "SPIBus.h"

pthread_mutex_t spi_sem;

int spi_open()
{
	int fd;
	int spd = SPI_SPEED;

	pthread_mutex_lock(&spi_sem);

	fd = open(SPIDEV_NAME, O_RDWR);
	if (fd ==0)
	{
		printf("cant open %s\n",SPIDEV_NAME);
		return 0;
	}

	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spd) < 0) {
		printf("SPI max_speed_hz");
		return 0;}
	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &spd) < 0) {
		printf("SPI max_speed_hz");
		return 0;}
	// printf("actual speed %d\n",spd);

	return fd;
}

void spi_close(int fd)
{
	close(fd);
	pthread_mutex_unlock(&spi_sem);

}
void spi_send(int fd, char *dat, char len)
{
	while (len--)
	{
		write (fd,dat,1);
		usleep(4);
		dat++;
	}
}

void spi_disp_write_dat(int fd,char r, char c, char v) // write a value to the dipslay memory
{
	char temp[4];
	temp[0] = CMD_DISP_WRITE_DATA;
	temp[1] = r;
	temp[2] = c;
	temp[3] = v;
	spi_send(fd,temp,4);
}

void spi_disp_test()
{
	int fd;
	char c;
	char temp[100];
	int x;
	fd = spi_open();

	temp[0] =CMD_DISP_ENABLE; // send the display enable command
	temp[1] = 1; // we are enabling
	spi_send(fd,temp,2);

	temp[0] =CMD_DISP_REFRESH; // send the display the refress command
	spi_send(fd,temp,1);

	printf("the display should be obn\n");
	getc(stdin);
	for (x=0; x < 132+10; x++)
	{
		if (x<132)
			spi_disp_write_dat(fd,4,x,0xff);
		if (x>10)
			spi_disp_write_dat(fd,4,x-10,0);

		if ((x % 4)==0)
		{
			temp[0] =CMD_DISP_REFRESH; // send the display the refress command
			spi_send(fd,temp,1);
			usleep(25000);
		}
	}

	printf("continue..\n");
	getc(stdin);

	temp[0] =CMD_DISP_ENABLE; // send the display enable command
	temp[1] = 0; // we are disbling
	spi_send(fd,temp,2);

	printf("the display should be off\n");
	getc(stdin);
	close(fd);

}
void do_chargen(int en)
{
	int fd;
	char c = en ? CMD_CHARGE_EN :CMD_CHARGE_DIS ;

	printf("Charging set to  %d\n",en);
	fd = spi_open();
	write (fd,&c,1);
	close(fd);
}


void do_vibe_en(int en)
{
	int fd;
	char c = en ? CMD_VIBE_ON :CMD_VIBE_OFF ;

	printf("Vibe set to  %d\n",en);
	fd = spi_open();
	write (fd,&c,1);
	close(fd);
}


void do_wdt_en(void)
{
	int fd;
	char c = WDT_ON;

	printf("WDT ON\n");
	fd = spi_open();
	write (fd,&c,1);
	spi_close(fd);
	printf("WDT ON done\n");
}

void do_vibe_tickle(void)
{
	int fd;
	char c = WDT_TICKLE;
	printf("wdt\n");
	fd = spi_open();
	printf("wdta\n");
	write (fd,&c,1);
	spi_close(fd);
	printf("wdt ticled \n");
}


void do_fan(char f1, char f2)
{
	int fd;
	char c = FAN_ON;
	printf("fan \n");
	fd = spi_open();
	printf("fan \n");
	write (fd,&c,1);
	write (fd,&f1,1);
	write (fd,&f2,1);
	spi_close(fd);
	printf("wdt ticled \n");
}

void do_shutdown()
{
	int fd;
	char c = CMD_OFF ;

	printf("new \n");
	fd = spi_open();
	if (fd ==0)
	{
		printf("cant open %s\n",SPIDEV_NAME);
		return;
	}

	write (fd,&c,1);
	close(fd);

}

void gps_add_cs_cr_lf(char *p)
{
	char val =0;
	char temp[100];
	char *s = &p[1]; // skip the $
	while (*s)
	{
		val = val ^ *s;
		s++;
	}
	sprintf (temp,"%s*%02X\r\n",p,val&0xff);
	strcpy(p,temp);
	return;
}

int spi_gps_write(char *dat)
{
	int fd;
	char c = CMD_GPS_SEND_STRING ;
	fd = spi_open();
	if (fd ==0)
	{
		printf("cant open %s\n",SPIDEV_NAME);
		return 0;
	}
	write (fd,&c,1);
	spi_send(fd, dat, strlen(dat)+1);
	close(fd);
	return 1;

}

//$PLSC,200,2,300,1000,300000,30000*0E

int spi_gps_test()
{
	int fd;
	char c = CMD_GET_BYTE ;
	char b;
	fd = spi_open();
	if (fd ==0)
	{
		printf("cant open %s\n",SPIDEV_NAME);
		return -1;
	}

	while (1)
	{
		c = CMD_GET_BYTE ;
		write (fd,&c,1);
		usleep(1000);
		read (fd,&b,1);
		usleep(1000);
		if (b!=0)
		{
			if (b=='$')
				printf ("\n%c",b&0xff);

			else
				printf ("%c",b&0xff);
			//	     printf ("\n%c (%d)",b&0xff,b&0xff);
		}
		else
		{
			usleep(2000);
			//printf("Z");
		}
	}
	close(fd);
	return c;
}


char spi_wait_result(int fd)
{
	char  tx[2];
	char rx[2];
	int ret;
	char done =0;
	struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			.len = 1,
			.delay_usecs = 100,
			.speed_hz = SPI_SPEED,
			.bits_per_word = 8,
	};

	tx[0] = 0;
	rx[0] = 0;
	done = 0;
	//   while (!done)
	while (0)
	{
		tx[0] = 0;
		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
		//	    printf("got %x\n",rx[0]);
		if (rx[0]==0x55)
			done = 1;
		usleep(1000);
	}
	done = 0;
	while (!done)
	{
		tx[0] = 0;
		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
		//	    printf("got %x\n",rx[0]);
		if (rx[0]!=0x55)
			done = 1;
		usleep(500);
	}
	return rx[0];
}

int spi_zigbee_open()
{
	int fd;
	int ret;
	char  tx[2];
	char rx[2];
	int done;

	struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			.len = 1,
			.delay_usecs = 100,
			.speed_hz = SPI_SPEED,
			.bits_per_word = 8,
	};

	fd = spi_open();
	if (fd ==0)
	{
		printf("cant open %s\n",SPIDEV_NAME);
		return -1;
	}

	tx[0] = CMD_ZIG_LINK;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	ret = spi_wait_result(fd);
	spi_close(fd);
	return rx[0];
}

int spi_zigbee_close()
{
	int fd;
	int ret;
	char  tx[2];
	char rx[2];
	int done;

	struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			.len = 1,
			.delay_usecs = 100,
			.speed_hz = SPI_SPEED,
			.bits_per_word = 8,
	};

	fd = spi_open();
	if (fd ==0)
	{
		printf("cant open %s\n",SPIDEV_NAME);
		return -1;
	}

	tx[0] = CMD_ZIG_UNLINK;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	ret = spi_wait_result(fd);
	spi_close(fd);
	return rx[0];
}

#define MAX_SEND 255
int spi_zigbee_read(char *msg, int *len)
{
	int fd;
	int ret;
	char  tx[MAX_SEND];
	char rx[MAX_SEND];
	int done;

	fd = spi_open();
	if (fd ==0)
	{
		printf("cant open %s\n",SPIDEV_NAME);
		return -1;
	}
	struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			.len = 1,
			.delay_usecs = 1000,
			.speed_hz = SPI_SPEED,
			.bits_per_word = 8,
	};

	tx[0] = CMD_ZIG_READ;
	tx[1] = 0;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	usleep(1000);
	//getc(stdin);
	*len = spi_wait_result(fd);

	tr.len = *len;
	if (tr.len>0)
	{
		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	}
	spi_close(fd);
	memcpy(msg,rx,*len);
	return ret;
}

int spi_zigbee_send(char *msg, int len)
{
	int fd;
	int ret;
	char  tx[MAX_SEND];
	char rx[MAX_SEND];
	int done;

	fd = spi_open();
	if (fd ==0)
	{
		printf("cant open %s\n",SPIDEV_NAME);
		return -1;
	}
	struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			.len = 2,
			.delay_usecs = 100,
			.speed_hz = SPI_SPEED,
			.bits_per_word = 8,
	};

	tx[0] = CMD_ZIG_SEND;
	tx[1] = len;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	memcpy(tx,msg,len);
	tr.len = len;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	ret = spi_wait_result(fd);
	spi_close(fd);
	return ret;
}

int spi_zigbee_read_batt()
{
	int fd;
	int ret,ret2;
	char  tx[2];
	char rx[2];
	int done;


	struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			.len = 1,
			.delay_usecs = 100,
			.speed_hz = SPI_SPEED,
			.bits_per_word = 8,
	};

	fd = spi_open();
	if (fd ==0)
	{
		printf("cant open %s\n",SPIDEV_NAME);
		return -1;
	}

	tx[0] = CMD_BATT;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	ret = spi_wait_result(fd);
	ret2 = spi_wait_result(fd);
	spi_close(fd);

	return ret&0xff | (ret2<<8);
}

int spi_read_button()
{
	int fd;
	int ret;
	char  tx[2];
	char rx[2];
	struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			.len = 2,
			.delay_usecs = 100,
			.speed_hz = SPI_SPEED,
			.bits_per_word = 8,
	};

	tx[0] = CMD_READ_BUTTONS;
	fd = spi_open();
	if (fd ==0)
	{
		printf("cant open %s\n",SPIDEV_NAME);
		return -1;
	}

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		printf("can't send spi message");

	/*	for (ret = 0; ret < 2; ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);
	}
	puts("");
	 */
	close(fd);
	return rx[1];
}
