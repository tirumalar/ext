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

#define BOB_COMMAND_OFFSET				2
#define BOB_SW_VERSION_OFFSET			67	// 3 bytes
#define BOB_HW_VERSION_OFFSET			70	// 3 bytes
#define BOB_ACCESS_DATA_OFFSET 			73	// 20 bytes

#define I2C_BUS	"/dev/ttyACM0"

#define bcd2bin(x)	(((x) & 0x0f) + ((x) >> 4) * 10)
#define bin2bcd(x)	((((x) / 10) << 4) + (x) % 10)

using namespace std;

enum PIM_BoardRGBLed
{
	Black = 0,
	Blue = 1,
	Red = 2,
	Purple = 3,
	Green = 4,
	Cyan = 5,
	yellow = 6,
	white = 7,
};

int set_interface_attribs(int fd, int speed)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0)
	{
		printf("Error from tcgetattr: %s\n", strerror(errno));
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
		printf("Error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int internal_read_array(int fd, unsigned char reg, char *buf, int len)
{
	int result;
	char buff[6000];

	buff[0] = 56;
	buff[1] = 0x00;
	buff[2] = reg;
	buff[3] = len;

	printf("writing: %x.%x.%x.%x\n", buff[0], buff[1], buff[2], buff[3]);
	result = write(fd, buff, 4);
	if (result != 4)
	{
		perror("internal_read_array: cannot write\n");
		return -1;
	}

	usleep(100);
	len = len + 4;
	printf("reading...\n");
	result = read(fd, buff, len);
	if (result != len)
	{
		return -1;
	}
	printf("response: %x.%x.%x.%x\n", buff[0], buff[1], buff[2], buff[3]);
	memcpy(buf, buff + 4, len - 4);

	return result;
}

int internal_write_array(int fd, unsigned char reg, void *ptr, int len)
{
	int result;
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

	result = write(fd, buff, len + 4);

	if (result != len + 4)
	{
		perror("internal_write_array: cannot write\n");
	}
	else
	{
		result = 0;
	}

	usleep(50);
	return result;
}

int internal_write_reg(int fd, unsigned char reg, unsigned int val)
{
	int result = -1;
	int response_len = -1;
	int expected_response_len = 4;
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

	printf("writing: %x.%x.%x.%x.%x\n", buff[0], buff[1], buff[2], buff[3],
			buff[4]);
	result = write(fd, buff, 5);
	if (result != 5)
	{
		perror("internal_write_reg: cannot write\n");
	}
	else
	{
		result = 0;
	}

	usleep(50);

	response_len = read(fd, buff, 4);
	printf("response length: %d\n", expected_response_len);
	if (response_len == expected_response_len)
	{
		printf("response: %x.%x.%x.%x\n", buff[0], buff[1], buff[2], buff[3]);
	}
	return result;
}

int set_sys_time(struct tm* tm1)
{
	time_t time = mktime(tm1);
	if (time < 0)
	{
		printf("unable to convert RTC time to system time: %s\n",
				strerror(errno));
		return -1;
	}
	struct timeval tv;
	tv.tv_sec = time;
	tv.tv_usec = 0;
	printf("synchronizing system time with RTC: %ld\n", time);

	if (settimeofday(&tv, NULL) != 0)
	{
		printf("unable to set system time: %s\n", strerror(errno));
		return -2;
	}

	return 0;
}

int main()
{
	int fd = open(I2C_BUS, O_RDWR | O_NOCTTY | O_SYNC);

	set_interface_attribs(fd, B19200);
	usleep(100);

	char sw_version[3] =
	{ 0 };
	char hw_version[3] =
	{ 0 };

	// versions
	// ***********************************************************************
	sleep(1);
	printf("retrieving versions...\n");

	int bytes_read = internal_read_array(fd, BOB_SW_VERSION_OFFSET, sw_version,
			3);
	if (bytes_read < 3)
	{
		perror("cannot read software version\n");
	}
	else
	{
		printf("ICM software version: %d.%d.%d\n", sw_version[0], sw_version[1],
				sw_version[2]);
	}
	sleep(1);
	bytes_read = internal_read_array(fd, BOB_HW_VERSION_OFFSET, hw_version, 3);
	if (bytes_read < 3)
	{
		perror("cannot read hardware version\n");
	}
	else
	{
		printf("ICM hardware version: %x.%x.%x\n", hw_version[0], hw_version[1],
				hw_version[2]);
	}
	// ***********************************************************************

//	// LED
//	// ***********************************************************************
//	sleep(1);
//	printf("setting LED color...\n");
//	char color = Red;
//	internal_write_array(fd, BOB_ACCESS_DATA_OFFSET, &color, 1);
//	sleep(1);
//	internal_write_reg(fd, BOB_COMMAND_OFFSET, BOB_COMMAND_SET_LED);
//
//	// ***********************************************************************

	// time
	// ***********************************************************************
	printf("retrieving RTC time...\n");
	char regs[8];
	struct tm tm1;

	//internal_write_reg(fd, BOB_COMMAND_OFFSET, BOB_COMMAND_RTCREAD_CMD); // TODO: after this call every read operation hang
	//usleep(5000);
	bytes_read = internal_read_array(fd, BOB_ACCESS_DATA_OFFSET, regs, 8);
	if (bytes_read < 8)
	{
		printf("cannot read RTC time (%d bytes read)\n", bytes_read);
	}
	else
	{
		printf("received data: 0x%0x, 0x%0x, 0x%0x, 0x%0x\n", regs[0], regs[1],
				regs[2], regs[3]);

		tm1.tm_sec = bcd2bin(regs[1] & 0x7f);
		tm1.tm_min = bcd2bin(regs[2] & 0x7f);
		tm1.tm_hour = bcd2bin(regs[3] & 0x3f);
		tm1.tm_mday = bcd2bin(regs[4] & 0x3f);
		tm1.tm_wday = regs[5] & 0x7;
		tm1.tm_mon = bcd2bin(regs[6] & 0x1f) - 1;
		tm1.tm_year = bcd2bin(regs[7]) + 100;
		printf("time: %s\n", asctime(&tm1));
	}

	// TODO: uncomment after fixing retrieving RTC time
	/*if (set_sys_time(&tm1) != 0)
	 {
	 perror("cannot set system time");
	 }*/
	FILE *testfile = fopen("/home/time_sync.test", "w");
	if (testfile != NULL)
	{
		fprintf(testfile, "RTC time: %s\n", asctime(&tm1));
		fclose(testfile);
	}

	// ***********************************************************************
	close(fd);

	return 0;
}
