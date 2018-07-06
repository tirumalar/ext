/*
 * mt9p001 driver definitions shared with user space programs
 */

#ifndef MT9P001_DEV_H
#define MT9P001_DEV_H

struct mt9p001_i2c_ioc {
	unsigned short reg;
	unsigned short value;
};

struct mt9p001_snap_ioc {
	unsigned int start_offset;
	unsigned int img_width;
	unsigned int img_height;
	int irled_en[2];
	int trigger_time_us;
	int flash_time_ms;
};

/* for 32 bit alignment even though we only need 1 byte for each */
struct adp8860_ioc {
	unsigned short reg;
	unsigned short value;
};


/* ioctl commands */
#define MT9P001_IOC_MAGIC		0xA1


#define MT9P001_IOC_RD_IMG_WIDTH	_IOR(MT9P001_IOC_MAGIC, 1, int)
#define MT9P001_IOC_RD_IMG_HEIGHT	_IOR(MT9P001_IOC_MAGIC, 2, int)
#define MT9P001_IOC_RD_IMG_SIZE		_IOR(MT9P001_IOC_MAGIC, 3, int)


#define MT9P001_IOC_RD_I2C		_IOWR(MT9P001_IOC_MAGIC, 4, char[4])
#define MT9P001_IOC_WR_I2C		_IOR(MT9P001_IOC_MAGIC, 5, char[4])

#define MT9P001_IOC_WR_SNAP		_IOW(MT9P001_IOC_MAGIC, 6, int[7])

#define MT9P001_IOC_RD_IRLED_EN0	_IOR(MT9P001_IOC_MAGIC, 7, int)
#define MT9P001_IOC_WR_IRLED_EN0	_IOW(MT9P001_IOC_MAGIC, 8, int)

#define MT9P001_IOC_RD_IRLED_EN1	_IOR(MT9P001_IOC_MAGIC, 9, int)
#define MT9P001_IOC_WR_IRLED_EN1	_IOW(MT9P001_IOC_MAGIC, 10, int)

#define MT9P001_IOC_RD_TRIG_TIME_US	_IOR(MT9P001_IOC_MAGIC, 11, int)
#define MT9P001_IOC_WR_TRIG_TIME_US	_IOW(MT9P001_IOC_MAGIC, 12, int)

#define MT9P001_IOC_RD_FLASH_TIME_MS	_IOR(MT9P001_IOC_MAGIC, 13, int)
#define MT9P001_IOC_WR_FLASH_TIME_MS	_IOW(MT9P001_IOC_MAGIC, 14, int)

#define MT9P001_IOC_RD_ADP8860		_IOR(MT9P001_IOC_MAGIC, 21, char[4])
#define MT9P001_IOC_WR_ADP8860		_IOR(MT9P001_IOC_MAGIC, 22, char[4])

#define MT9P001_IOC_WR_SLAVE_TRIGGER _IOW(MT9P001_IOC_MAGIC, 23, int[7])


#endif
