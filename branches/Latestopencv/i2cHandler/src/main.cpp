#include <iostream>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

#include <termios.h>

#define MAX_WRITE_LEN 		255

#define BOB_COMMAND_RTCREAD_CMD	 		5
#define BOB_COMMAND_RTCWRITE_CMD	 	6
#define BOB_COMMAND_SET_LED			 	7
#define BOB_COMMAND_OIM_SWITCH			8

#define BOB_COMMAND_OFFSET				2
#define BOB_SW_VERSION_OFFSET			67	// 3 bytes
#define BOB_HW_VERSION_OFFSET			70	// 3 bytes
#define BOB_ACCESS_DATA_OFFSET 			73	// 20 bytes
#define I2C_BUS	"/dev/ttyACM0"

#define bcd2bin(x)	(((x) & 0x0f) + ((x) >> 4) * 10)
#define bin2bcd(x)	((((x) / 10) << 4) + (x) % 10)

using namespace std;

int debug = 0;
int log_fd = -1;

//enum PIM_BoardRGBLed
//{
//	Black = 0,
//	Blue = 1,
//	Red = 2,
//	Purple = 3,
//	Green = 4,
//	Cyan = 5,
//	yellow = 6,
//	white = 7,
//};

int set_interface_attribs(int fd, int speed)
{
	struct termios tty;

	if (fd == 0)
	{
		fprintf(stderr, "set_interface_attribs: invalid descriptor\n");
		return -1;
	}

	if (tcgetattr(fd, &tty) < 0)
	{
		fprintf(stderr, "error from tcgetattr: %s\n", strerror(errno));
		return -1;
	}

	cfsetospeed(&tty, (speed_t) speed);
	cfsetispeed(&tty, (speed_t) speed);

	tty.c_cflag |= (CLOCAL | CREAD); /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8; /* 8-bit characters */
	tty.c_cflag &= ~PARENB; /* no parity bit */
	tty.c_cflag &= ~CSTOPB; /* only need 1 stop bit */
	tty.c_cflag &= ~CRTSCTS; /* no hardware flowcontrol */

	/* setup for non-canonical mode */
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL
			| IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

	/* fetch bytes as they become available */
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		fprintf(stderr, "error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int internal_read_array(int fd, unsigned char reg, char *buf, int len)
{
	int result = -1;
	char buff[6000];

	if (fd == 0)
	{
		return -1;
	}

	buff[0] = 56;
	buff[1] = 0x00;
	buff[2] = reg;
	buff[3] = len;

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	fd_set read_set, write_set;
	FD_ZERO(&write_set);
	FD_SET(fd, &write_set);

	int select_result = select(FD_SETSIZE, NULL, &write_set, NULL, &timeout);
	if (select_result == 1)
	{
		//printf("writing: %x.%x.%x.%x\n", buff[0], buff[1], buff[2], buff[3]);
		result = write(fd, buff, 4);
		if (result != 4)
		{
			fprintf(stderr, "internal_read_array: cannot write\n");
			return -3;
		}

		usleep(100);
		len = len + 4;
		//printf("reading...\n");
	}
	else if (select_result == 0)
	{
		fprintf(stderr, "internal_read_array: write timeout\n");
		return -2;
	}
	else
	{
		fprintf(stderr, "internal_read_array: select failed on write (%s)\n",
				strerror(errno));
		return -2;
	}

	FD_ZERO(&read_set);
	FD_SET(fd, &read_set);
	select_result = select(FD_SETSIZE, &read_set, NULL, NULL, &timeout);
	if (select_result == 1)
	{
		result = read(fd, buff, len);
		if (result != len)
		{
			fprintf(stderr, "internal_read_array: cannot read\n");
			return -2;
		}
		//printf("response: %x.%x.%x.%x\n", buff[0], buff[1], buff[2], buff[3]);
		memcpy(buf, buff + 4, len - 4);
	}
	else if (select_result == 0)
	{
		fprintf(stderr, "internal_read_array: read timeout\n");
		return -2;
	}
	else
	{
		fprintf(stderr, "internal_read_array: select failed on read (%s)\n",
				strerror(errno));
		return -2;
	}

	return 0;
}

int internal_write_array(int fd, unsigned char reg, void *ptr, int len)
{
	int result = -1;
	unsigned char buff[MAX_WRITE_LEN];

	if (fd == 0)
	{
		return -1;
	}

	buff[0] = 57;
	buff[1] = 0x00;
	buff[2] = reg;
	buff[3] = len;

	if (ptr)
	{
		memcpy(&buff[4], ptr, len);
	}
	else
	{
		memset(&buff[4], 0, len);
	}

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	fd_set write_set;
	FD_ZERO(&write_set);
	FD_SET(fd, &write_set);

	int select_result = select(FD_SETSIZE, NULL, &write_set, NULL, &timeout);
	if (select_result == 1)
	{
		result = write(fd, buff, len + 4);
		if (result != len + 4)
		{
			fprintf(stderr, "internal_write_array: cannot write\n");
			return -3;
		}

		usleep(50);

		// read the data that was echoed by PIM. TODO: select or flush the buffer to prevent reading wrong data
		read(fd, buff, 4);
	}
	else if (select_result == 0)
	{
		fprintf(stderr, "internal_write_array: write timeout\n");
		return -2;
	}
	else
	{
		fprintf(stderr, "internal_write_array: select failed on write (%s)\n",
				strerror(errno));
		return -2;
	}
	return 0;
}

int internal_write_reg(int fd, unsigned char reg, unsigned int val)
{
	int result = -1;
	int response_len = -1;
	unsigned char buff[5];

	if (fd == 0)
	{
		return -1;
	}

	buff[0] = 57;
	buff[1] = 0x00;
	buff[2] = reg;
	buff[3] = 0x01;
	buff[4] = val;

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	fd_set read_set, write_set;
	FD_ZERO(&write_set);
	FD_SET(fd, &write_set);

	int select_result = select(FD_SETSIZE, NULL, &write_set, NULL, &timeout);
	if (select_result == 1)
	{

//	printf("writing: %x.%x.%x.%x.%x\n", buff[0], buff[1], buff[2], buff[3],
//			buff[4]);
		result = write(fd, buff, 5);
		if (result != 5)
		{
			fprintf(stderr, "internal_write_reg: cannot write\n");
			return -2;
		}
	}
	else if (select_result == 0)
	{
		fprintf(stderr, "internal_write_reg: write timeout\n");
		return -2;
	}
	else
	{
		fprintf(stderr, "internal_write_reg: select failed on write (%s)\n",
				strerror(errno));
		return -2;
	}
	usleep(50);

	FD_ZERO(&read_set);
	FD_SET(fd, &read_set);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	select_result = select(FD_SETSIZE, &read_set, NULL, NULL, &timeout);
	if (select_result == 1)
	{
		response_len = read(fd, buff, 4);
		//printf("response length: %d\n", response_len);
		if (response_len != 4)
		{
			return -3;
		}
		//printf("response: %x.%x.%x.%x\n", buff[0], buff[1], buff[2], buff[3]);
	}
	else if (select_result == 0)
	{
		fprintf(stderr, "internal_write_reg: read timeout\n");
		return -2;
	}
	else
	{
		fprintf(stderr, "internal_write_reg: select failed (%s)\n",
				strerror(errno));
		return -2;
	}

	return 0;
}

int set_sys_time(struct tm* tm1)
{
	time_t time = mktime(tm1);
	if (time < 0)
	{
		fprintf(stderr, "unable to convert RTC time to system time: %s\n",
				strerror(errno));
		return -1;
	}
	struct timeval tv;
	tv.tv_sec = time;
	tv.tv_usec = 0;
	printf("synchronizing system time with RTC: %ld\n", time);

	if (settimeofday(&tv, NULL) != 0)
	{
		fprintf(stderr, "unable to set system time: %s\n", strerror(errno));
		return -2;
	}

	return 0;
}

int get_versions(int fd)
{
	int stat = 0;

	char sw_version[3] =
	{ 0 };
	char hw_version[3] =
	{ 0 };

	//printf("retrieving versions...\n");

	stat = internal_read_array(fd, BOB_SW_VERSION_OFFSET, sw_version, 3);
	if (stat)
	{
		fprintf(stderr, "cannot read software version\n");
		return stat;
	}
	else
	{
		printf("ICM software version: %d.%d.%d\n", sw_version[0], sw_version[1],
				sw_version[2]);
	}
	sleep(1);
	stat = internal_read_array(fd, BOB_HW_VERSION_OFFSET, hw_version, 3);
	if (stat)
	{
		fprintf(stderr, "cannot read hardware version\n");
		return stat;
	}
	else
	{
		printf("ICM hardware version: %x.%x.%x\n", hw_version[0], hw_version[1],
				hw_version[2]);
	}

	return stat;
}

int set_led(int fd, char color)
{
	int stat = 0;
	//printf("setting LED color...\n");
	stat = internal_write_array(fd, BOB_ACCESS_DATA_OFFSET, &color, 1);
	if (stat)
	{
		return stat;
	}
	sleep(1);
	stat = internal_write_reg(fd, BOB_COMMAND_OFFSET, BOB_COMMAND_SET_LED);
	if (stat)
	{
		return stat;
	}
	return 0;
}

int set_rtc_time(int fd)
{
	int stat = 0;
	unsigned char regs[8];
	time_t rawtime;
	struct tm* tm1;

	//printf("setting time...\n");
	time(&rawtime);
	tm1 = localtime(&rawtime);

	//printf("setting to %s\n", asctime(tm1));

	regs[0] = 0;
	/* This will purposely overwrite REG_SECONDS_OS */
	regs[1] = bin2bcd(tm1->tm_sec);
	regs[2] = bin2bcd(tm1->tm_min);
	regs[3] = bin2bcd(tm1->tm_hour);
	regs[4] = bin2bcd(tm1->tm_mday);
	regs[5] = tm1->tm_wday;
	regs[6] = bin2bcd(tm1->tm_mon + 1);
	regs[7] = bin2bcd(tm1->tm_year - 100);

	stat = internal_write_array(fd, BOB_ACCESS_DATA_OFFSET, regs, 8);
	if (stat)
	{
		return stat;
	}

//	printf("setting time to 0x%0x: 0x%0x: 0x%0x,0x%0x: 0x%0x,0x%0x: 0x%0x...\n",
//			regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7]);

	stat = internal_write_reg(fd, BOB_COMMAND_OFFSET, BOB_COMMAND_RTCWRITE_CMD);
	if (stat)
	{
		return stat;
	}

	return 0;

}

int get_rtc_time(int fd, struct tm *tm_out)
{
	//printf("retrieving RTC time...\n");
	char regs[8];
	int stat = 0;

	stat = internal_write_reg(fd, BOB_COMMAND_OFFSET, BOB_COMMAND_RTCREAD_CMD);
	if (stat)
	{
		return stat;
	}

	usleep(5000);
	stat = internal_read_array(fd, BOB_ACCESS_DATA_OFFSET, regs, 8);
	if (stat)
	{
		fprintf(stderr, "cannot read RTC time\n");
		return stat;
	}
	else
	{
		printf("received data: 0x%0x, 0x%0x, 0x%0x, 0x%0x\n", regs[0], regs[1],
				regs[2], regs[3]);

		tm_out->tm_sec = bcd2bin(regs[1] & 0x7f);
		tm_out->tm_min = bcd2bin(regs[2] & 0x7f);
		tm_out->tm_hour = bcd2bin(regs[3] & 0x3f);
		tm_out->tm_mday = bcd2bin(regs[4] & 0x3f);
		tm_out->tm_wday = regs[5] & 0x7;
		tm_out->tm_mon = bcd2bin(regs[6] & 0x1f) - 1;
		tm_out->tm_year = bcd2bin(regs[7]) + 100;
		tm_out->tm_isdst = -1;
		printf("time: %s\n", asctime(tm_out));
	}
	return 0;
}

int reboot_oim(int fd, char on)
{
	int stat = 0;

	if (on)
	{
		printf("turning OIM on...\n");
	}
	else
	{
		printf("turning OIM off...\n");
	}

	stat = internal_write_array(fd, BOB_ACCESS_DATA_OFFSET, &on, 1);
	if (stat)
	{
		return stat;
	}
	sleep(1);
	stat = internal_write_reg(fd, BOB_COMMAND_OFFSET, BOB_COMMAND_OIM_SWITCH);
	if (stat)
	{
		return stat;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int opt;
	int color = -1;

	int fd = open(I2C_BUS, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0)
	{
		fprintf(stderr, "cannot open port\n");
		exit(EXIT_FAILURE);
	}
	set_interface_attribs(fd, B19200);
	usleep(100);

	while ((opt = getopt(argc, argv, "dvtr:hl:")) != -1)
	{
		switch (opt)
		{
		case 'd':
			debug = 1;
			break;
		case 'v':
			if (get_versions(fd))
			{
				exit(EXIT_FAILURE);
			}
			break;
		case 'h':
			if (set_rtc_time(fd))
			{
				exit(EXIT_FAILURE);
			}
			break;
		case 't':
			struct tm tm_rtc;
			if (get_rtc_time(fd, &tm_rtc))
			{
				exit(EXIT_FAILURE);
			}
			if (set_sys_time(&tm_rtc))
			{
				exit(EXIT_FAILURE);
			}
			break;
		case 'r':
			if (reboot_oim(fd, (char)atoi(optarg)))
			{
				exit(EXIT_FAILURE);
			}
			break;
		case 'l':
			color = atoi(optarg);
			if (color < 0 || color > 7)
			{
				fprintf(stderr,
						"color is not supported (%d), supported colors: 0-7\n",
						color);
				exit(EXIT_FAILURE);
			}
			if (set_led(fd, (char) color))
			{
				exit(EXIT_FAILURE);
			}
			break;
		default:
			fprintf(stderr, "usage: %s vtrlh\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	// ***********************************************************************
	close(fd);

	return 0;
}
