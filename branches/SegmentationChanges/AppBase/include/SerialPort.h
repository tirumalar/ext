#pragma once
#include <string>

#define DEFAULT_LED_COM_PORT 3
#define BAUD_RATE 9600
#define DATA_BITS 8
#define ONESTOPBIT 1
#define NOPARITY 0
enum CameraResponse { IS_SUCCESS, IS_COMM_ERROR };

class SerialPort
{
public:
	SerialPort(int nComPort, int baudrate = BAUD_RATE, int parity = NOPARITY, int bytesize = DATA_BITS, int stopbits = ONESTOPBIT);
	virtual ~SerialPort(void);
	bool IsConnected() const { return m_bIsConnected; }
	void OpenPort();
	int set_interface_attribs(int baudrate);
	void set_mincount(int mcount);
private:
	int 	m_ComPortFd; 	// File descriptor for the port
	bool	m_bIsConnected;	//whether the connection to the serial COM port is valid
	
};

#if 0

bool Send(const std::string& sMessage);
	bool Send(const char *message, int length);
	CameraResponse Receive(char *message, int length, int &bytesRead);
#endif


