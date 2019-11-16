#include "SerialPort.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

//values used to determine the timeout while waiting for a response from the controller
#define READ_TIMEOUT_MULTIPLIER 10
#define READ_TIMEOUT_CONSTANT   50

SerialPort::SerialPort(int nComPort, int baudrate, int parity, int bytesize, int stopbits) : m_ComPortFd(-1), m_bIsConnected(false)
{
	//m_pLogger = CLogger::GetLogger();



	//m_pLogger->Log("COM port opened: %d\n", nComPort);
}

SerialPort::~SerialPort(void)
{
	//Close the port
	close(m_ComPortFd);

	// m_pLogger = NULL;
}
#if 0
int SerialPort::Init()
{
	m_pSerialPort = new SerialPort(m_Port);
	if(m_pSerialPort && m_pSerialPort->IsConnected())
	{
		m_pAsynchSerialPort = new SerialPortAsynch(m_pSerialPort, &m_Lock);
		m_pAsynchSerialPort->Start();

		if(m_InputPort > 0)
		{
			m_pInputSerialPort = new SerialPort(m_InputPort);
		}

		SetDefaults();
	}
	return 0;
}

CoolRunnerController2::~CoolRunnerController2(void)
{
	if(m_pSerialPort)
	{
		if(m_pSerialPort->IsConnected())
		{
			SetPulsePeriod(0);
			SetPulseWidth(0);
			SetPulseCurrent(0);
		}

		if(m_pAsynchSerialPort)
		{
			m_pAsynchSerialPort->Stop();
			delete m_pAsynchSerialPort;
		}
		delete m_pSerialPort;
	}
}
#endif

void SerialPort::OpenPort()
{
	m_ComPortFd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY); //O_SYNC
	if (m_ComPortFd == -1)
	{
		perror("open_port: Unable to open /dev/ttyUSB0 - ");
	}
	else{
		m_bIsConnected = true;
		printf("Port opened successfully\n");
		fcntl(m_ComPortFd, F_SETFL, 0);
	}

}

int SerialPort::set_interface_attribs(int baudrate)
{
    struct termios tty;

    if (tcgetattr(m_ComPortFd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)baudrate);
    cfsetispeed(&tty, (speed_t)baudrate);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(m_ComPortFd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void SerialPort::set_mincount(int mcount)
{
    struct termios tty;

    if (tcgetattr(m_ComPortFd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(m_ComPortFd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}



#if 0
bool SerialPort::Send(const char *message, int length)
{
	return true;
}

bool SerialPort::Send(const std::string& sMessage)
{
	return 0;
}

CameraResponse SerialPort::Receive(char *message, int length, int &bytesRead)
{
	bytesRead = 0;
	return IS_SUCCESS;
}
#endif
