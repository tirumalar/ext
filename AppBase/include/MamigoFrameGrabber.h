/*
 * MamigoFrameGrabber.h
 *
 *  Created on: 22 Oct, 2008
 *      Author: akhil
 */

#ifndef MAMIGOFRAMEGRABBER_H_
#define MAMIGOFRAMEGRABBER_H_


#include "FrameGrabber.h"
//fwd
struct user_val;
struct init_camera;
struct ioctl_vals;
struct frameRead;
class ShiftAndDcOffset;
//struct flagState;

class MamigoFrameGrabber : public FrameGrabber{

protected:
	bool bStillFrames;	// no video stream mode of operation
	int m_pllEnable;
	int m_pllDelay;
//	struct flagState *flagstate;
	struct user_val *uv;
	struct init_camera *ic;
	struct ioctl_vals *iv;
	struct frameRead *req;
	short m_datatype;
	int skipBytes;	// bytes which need to be skipped from the beginning of frame
	void exec_ioctls(struct ioctl_vals *iv, int fd);
	void setDefaults(Configuration *pCfg);
	unsigned char DEVID;		// the device ID to be used in i2c calls
	bool isEvalBoard;  		// are we running on an eval board
	bool isEyelockBoard;
	char initialI2CAddress;		// we use this to initially select an I2C extension
	char initialI2CValue;
	void configure_MT9P031(Configuration *pCfg, struct camconfig *camCfg);
	void map_uservals_to_registers_MT9P031(struct user_val *uv, struct ioctl_vals *iv, Configuration *pCfg);
	virtual int update(unsigned char regis , unsigned short value);
	int reset_cam(void);
	int restart_cam(void);
	int update_all(struct camconfig newconfig);
public:
	// uses SourceType enum which is hidden
	MamigoFrameGrabber(short sType);
	virtual ~MamigoFrameGrabber();
	virtual void init(Configuration *pCfg=0);
	virtual void term();
	virtual bool start(bool bStillFrames=false);
	virtual bool stop();
	// the two sensors are producing Bayer
	virtual bool isBayer() const;
	virtual bool isITU656Source() const;
	virtual void getDims(int& width, int& height) const;
	virtual void getPPIParams(int& pixels_per_line, int& lines_per_frame, int& ppiControl) const;

	/*
	 * the returned pointer is over written frequesntly
	 * so it should not be used for a long time.
	 */
	virtual char *getLatestFrame_raw();
	void DMA(void);
	// typicall linux read e.g. http://linux.die.net/man/2/read
	virtual size_t readFrame(void *buf, size_t count);
	unsigned char *m_ImagePtr;
	unsigned short m_ShiftRight,m_DcOffset;
	int m_numbits;
	bool m_hw16to8;
	ShiftAndDcOffset *m_shiftAndDcOffset;
};

#endif /* MAMIGOFRAMEGRABBER_H_ */
